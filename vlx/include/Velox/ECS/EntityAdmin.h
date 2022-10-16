#pragma once

#include <unordered_map>
#include <memory>
#include <vector>
#include <queue>

#include <Velox/Utilities.hpp>
#include <Velox/Config.hpp>

#include "Identifiers.hpp"
#include "Archetype.hpp"
#include "ComponentAlloc.hpp"
#include "System.hpp"

namespace vlx
{
	class Entity;

	////////////////////////////////////////////////////////////
	// 
	// Based on article by Deckhead:
	// https://indiegamedev.net/2020/05/19/an-entity-component-system-with-data-locality-in-cpp/ 
	// 
	////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////
	// 
	// Data-oriented ECS design. Be warned that adding and removing components
	// is an expensive operation. Try to avoid performing operations on runtime as much as possible
	// and look at using pooling instead to prevent removing and adding entities often.
	// 
	////////////////////////////////////////////////////////////
	class EntityAdmin final : private NonCopyable
	{
	private:
		struct Record
		{
			Archetype*	archetype{nullptr};
			std::size_t index{0}; // where in the archetype entity array is the entity located at
		};

		struct ArchetypeRecord
		{
			std::size_t column{0}; // where in the archetype's data is the component's data located at
		};

		using ComponentPtr = typename std::unique_ptr<IComponentAlloc>;
		using ArchetypePtr = typename std::unique_ptr<Archetype>;

		using SystemsArrayMap			= typename std::unordered_map<LayerType, std::vector<ISystem*>>;
		using ArchetypesArray			= typename std::vector<ArchetypePtr>;
		using ArchetypeMap				= typename std::unordered_map<ArchetypeID, std::vector<Archetype*>>;
		using EntityArchetypeMap		= typename std::unordered_map<EntityID, Record>;
		using ComponentTypeIDBaseMap	= typename std::unordered_map<ComponentTypeID, ComponentPtr>;
		using ComponentArchetypesMap	= typename std::unordered_map<ComponentTypeID, std::unordered_map<ArchetypeID, ArchetypeRecord>>;

	public:
		VELOX_API EntityAdmin();
		VELOX_API ~EntityAdmin();

	public:
		template<IsComponent C>
		C& GetComponent(const EntityID entity_id) const;

		template<IsComponent C>
		C* TryGetComponent(const EntityID entity_id) const;

		template<IsComponent C>
		bool HasComponent(const EntityID entity_id) const;

		template<IsComponent C>
		bool IsComponentRegistered() const;

		/// <summary>
		///		Get all entities that contain the provided components
		/// </summary>
		/// <param name="restricted:">
		///		Get all entities that strictly only have the provided components
		/// </param>
		/// <returns></returns>
		template<IsComponents... Cs>
		std::vector<EntityID> GetEntitiesWith(bool restricted = false) const;

	public:
		template<IsComponent C, typename... Args> requires std::constructible_from<C, Args...>
		C* AddComponent(const EntityID entity_id, Args&&... args);

		template<IsComponent C>
		void RemoveComponent(const EntityID entity_id);

		template<IsComponent C>
		void RegisterComponent();

		/// <summary>
		///		Increase the capacity in each matching archetype to reduce reallocations. Do note that it
		///		will create new archetypes to reserve if they do not exist.
		/// </summary>
		/// <param name="component_count:">
		///		Number of components to reserve for in the archetypes
		/// </param>
		template<IsComponents... Cs>
		void Reserve(const std::size_t component_count);

		template<IsComponents... Cs, class Comp>
		void SortComponents(Comp&& comparison) requires SameTypeParameter<Comp, std::tuple_element_t<0, std::tuple<Cs...>>, 0, 1>;

	public:		
		VELOX_API EntityID GetNewEntityID();

		VELOX_API const std::size_t& GetComponentIndex(const EntityID entity_id) const;
		VELOX_API bool InArchetype(const ArchetypeID archetype, const EntityID entity_id) const;

		VELOX_API void RunSystems(const LayerType layer) const;
		VELOX_API void SortSystems(const LayerType layer);

		VELOX_API void RegisterSystem(const LayerType layer, ISystem* system);
		VELOX_API void RegisterEntity(const EntityID entity_id);

		VELOX_API void RemoveSystem(const LayerType layer, ISystem* system);
		VELOX_API void RemoveEntity(const EntityID entity_id);

		VELOX_API void AddComponent(const EntityID entity_id, const ComponentTypeID add_component_id);
		VELOX_API void RemoveComponent(const EntityID entity_id, const ComponentTypeID rmv_component_id);

		/// <summary>
		///		Shrinks the ECS by removing all the empty archetypes
		/// </summary>
		/// <param name="extensive:"> 
		///		perform a complete shrink of the ECS by removing all the extra data space, will most likely 
		///		invalidate all the existing pointers from e.g., GetComponent()
		/// </param>
		VELOX_API void Shrink(bool extensive = false);

	private:
		VELOX_API Archetype* GetArchetype(const ComponentIDs& component_ids);
		VELOX_API Archetype* CreateArchetype(const ComponentIDs& component_ids, const ArchetypeID id);

		VELOX_API void MakeRoom(
			Archetype* archetype,
			const IComponentAlloc* component,
			const std::size_t data_size, 
			const std::size_t i);

	private:
		EntityID				m_entity_id_counter;	// current id counter for entities
		std::queue<EntityID>	m_reusable_entity_ids;	// reusable ids of entities that have been destroyed

		SystemsArrayMap			m_systems;					// map layer to array of systems (layer allows for controlling the order of calls)
		ArchetypesArray			m_archetypes;				// find matching archetype to update matching entities
		ArchetypeMap			m_archetype_map;			// map set of components to matching archetypes that contains such components
		EntityArchetypeMap		m_entity_archetype_map;		// map entity to where its data is located at in the archetype
		ComponentArchetypesMap	m_component_archetypes_map; // map component to the archetypes it exists in and where all of the components data in the archetype is located at
		ComponentTypeIDBaseMap	m_component_map;			// access to helper functions for modifying each unique component
	};

	template<IsComponent C, typename... Args> requires std::constructible_from<C, Args...>
	inline C* EntityAdmin::AddComponent(const EntityID entity_id, Args&&... args)
	{
		assert(IsComponentRegistered<C>()); // component should be registered

		const auto eit = m_entity_archetype_map.find(entity_id);
		if (eit == m_entity_archetype_map.end())
			return nullptr;

		Record& record = eit->second;
		Archetype* old_archetype = record.archetype;

		C* add_component = nullptr;
		Archetype* new_archetype = nullptr;

		const ComponentTypeID add_component_id = ComponentAlloc<C>::GetTypeID();

		if (old_archetype) // already has an attached archetype, define a new archetype
		{
			ComponentIDs new_archetype_id = old_archetype->type; // create copy
			if (!cu::InsertUniqueSorted(new_archetype_id, add_component_id)) // insert while keeping the vector sorted (this should ensure that the archetype is always sorted)
				return nullptr;

			new_archetype = GetArchetype(new_archetype_id);
			assert(new_archetype_id == new_archetype->type);

			const EntityID last_entity_id = old_archetype->entities.back();
			Record& last_record = m_entity_archetype_map[last_entity_id];

			const bool same_entity = (last_entity_id == entity_id);

			for (std::size_t i = 0, j = 0; i < new_archetype_id.size(); ++i) // move all the data from old to new and perform swaps at the same time
			{
				const ComponentTypeID component_id	= new_archetype_id[i];
				const IComponentAlloc* component	= m_component_map[component_id].get();
				const std::size_t& component_size	= component->GetSize();

				const std::size_t current_size		= new_archetype->entities.size() * component_size;
				const std::size_t new_size			= current_size + component_size;

				if (new_size > new_archetype->component_data_size[i])
					MakeRoom(new_archetype, component, component_size, i);

				if (component_id == add_component_id)
				{
					assert(add_component == nullptr); // should never happen twice

					add_component = new(&new_archetype->component_data[i][current_size])
						C(std::forward<Args>(args)...);
				}
				else
				{
					component->MoveDestroyData(*this, entity_id,
						&old_archetype->component_data[j][record.index * component_size],
						&new_archetype->component_data[i][current_size]);

					if (!same_entity)
					{
						component->MoveDestroyData(*this, entity_id,
							&old_archetype->component_data[j][last_record.index * component_size],
							&old_archetype->component_data[j][record.index * component_size]); // move data to last
					}

					++j;
				}

				m_component_archetypes_map[component_id][new_archetype->id].column = i;
			}

			assert(add_component != nullptr); // a new component should have been added

			if (!same_entity) // move back to current
			{
				old_archetype->entities[record.index] = old_archetype->entities.back();
				last_record.index = record.index;
			}

			old_archetype->entities.pop_back(); // by only removing the last entity, it means that when the next component is added, it will overwrite the previous
		}
		else // if the entity has no archetype, first component
		{
			ComponentIDs new_archetype_id(1, add_component_id);	// construct archetype with the component id
			new_archetype = GetArchetype(new_archetype_id);		// construct or get archetype using the id

			const IComponentAlloc* component			= m_component_map[add_component_id].get();
			const std::size_t& component_size	= component->GetSize();

			const std::size_t current_size	= new_archetype->entities.size() * component_size;
			const std::size_t new_size		= current_size + component_size;

			if (new_size > new_archetype->component_data_size[0])
				MakeRoom(new_archetype, component, component_size, 0); // make room and move over existing data

			add_component = new(&new_archetype->component_data[0][current_size])
				C(std::forward<Args>(args)...);

			m_component_archetypes_map[add_component_id][new_archetype->id].column = 0;
		}

		add_component->Created(*this, entity_id);

		new_archetype->entities.push_back(entity_id);
		record.index = new_archetype->entities.size() - 1;
		record.archetype = new_archetype;

		return add_component;
	}

	template<IsComponent C>
	inline void EntityAdmin::RemoveComponent(const EntityID entity_id)
	{
		RemoveComponent(entity_id, ComponentAlloc<C>::GetTypeID());
	}

	template<IsComponent C>
	inline void EntityAdmin::RegisterComponent()
	{
		const ComponentTypeID component_id = ComponentAlloc<C>::GetTypeID();

		if (!m_component_map.contains(component_id))
			m_component_map.emplace(component_id, std::make_unique<ComponentAlloc<C>>());
	}

	template<IsComponent C>
	inline bool EntityAdmin::IsComponentRegistered() const
	{
		return m_component_map.contains(ComponentAlloc<C>::GetTypeID());
	}

	template<IsComponent C>
	inline bool EntityAdmin::HasComponent(const EntityID entity_id) const
	{
		assert(IsComponentRegistered<C>()); // component should be registered

		const auto eit = m_entity_archetype_map.find(entity_id);
		if (eit == m_entity_archetype_map.end())
			return false;

		const Archetype* archetype = eit->second.archetype;

		if (!archetype)
			return false;

		const ComponentTypeID& component_id = ComponentAlloc<C>::GetTypeID();

		const auto cit = m_component_archetypes_map.find(component_id);
		if (cit == m_component_archetypes_map.end())
			return false;

		return cit->second.contains(archetype->id);
	}

	template<IsComponent C>
	inline C& EntityAdmin::GetComponent(const EntityID entity_id) const
	{
		assert(IsComponentRegistered<C>()); // component should be registered

		const auto eit = m_entity_archetype_map.find(entity_id);

		const Record& record = eit->second;
		const Archetype* archetype = record.archetype;

		const ComponentTypeID& component_id = ComponentAlloc<C>::GetTypeID();

		const auto cit = m_component_archetypes_map.find(component_id);
		const auto ait = cit->second.find(archetype->id);

		C* components = reinterpret_cast<C*>(&archetype->component_data[ait->second.column][0]);
		return components[record.index];
	}

	template<IsComponent C>
	inline C* EntityAdmin::TryGetComponent(const EntityID entity_id) const
	{
		const auto eit = m_entity_archetype_map.find(entity_id);
		if (eit == m_entity_archetype_map.end())
			return nullptr;

		const Record& record = eit->second;
		const Archetype* archetype = record.archetype;

		if (!archetype)
			return nullptr;

		const ComponentTypeID& component_id = Component<C>::GetTypeID();

		const auto cit = m_component_archetypes_map.find(component_id);
		if (cit == m_component_archetypes_map.end())
			return nullptr;

		const auto ait = cit->second.find(archetype->id);
		if (ait == cit->second.end())
			return nullptr;

		const ArchetypeRecord& a_record = ait->second;

		C* components = reinterpret_cast<C*>(&archetype->component_data[a_record.column][0]);
		return &components[record.index];
	}

	template<IsComponents... Cs>
	inline std::vector<EntityID> EntityAdmin::GetEntitiesWith(bool restricted) const
	{
		std::vector<EntityID> entities;

		const ComponentIDs component_ids = SortKeys({ { Component<Cs>::GetTypeID()... } });
		const ArchetypeID archetype_id = cu::VectorHash<ComponentIDs>()(component_ids); // see system.hpp

		const auto it = m_archetype_map.find(archetype_id);
		if (it == m_archetype_map.end())
			return entities;

		const std::vector<Archetype*>& archetypes = it->second;

		if (!restricted)
		{
			std::size_t total_size = 0; // used to prevent numerous reallocations

			for (const Archetype* archetype : archetypes)
				total_size += archetype->entities.size();

			entities.reserve(total_size);
			for (const Archetype* archetype : archetypes)
			{
				entities.insert(entities.end(), 
					archetype->entities.begin(), 
					archetype->entities.end());
			}
		}
		else
		{
			for (const Archetype* archetype : archetypes)
			{
				if (archetype->type != component_ids)
					continue;

				entities.insert(entities.end(),
					archetype->entities.begin(),
					archetype->entities.end());

				break;
			}
		}

		return entities;
	}

	template<IsComponents... Cs>
	inline void EntityAdmin::Reserve(const std::size_t component_count)
	{
		const ComponentIDs component_ids = SortKeys({ { Component<Cs>::GetTypeID()... } }); // see system.hpp
		const ArchetypeID archetype_id = cu::VectorHash<ComponentIDs>()(component_ids);

		if (!m_archetype_map.contains(archetype_id))
		{
			for (std::size_t i = 0; i < component_ids.size(); ++i) // archetypes did not exist, create them
				CreateArchetype(ComponentIDs(component_ids.begin(), component_ids.begin() + (i + 1)));
		}

		const auto& archetypes = m_archetype_map[archetype_id];
		for (Archetype* archetype : archetypes)
		{
			for (const ComponentTypeID component_id : component_ids)
			{
				const auto cit = m_component_archetypes_map.find(component_id);
				if (cit == m_component_archetypes_map.end())
					continue;

				const auto ait = cit->second.find(archetype->id);
				if (ait == cit->second.end())
					continue;

				const auto i = ait->second.column;

				const IComponentAlloc* component	= m_component_map[component_id].get();
				const std::size_t& component_size	= component->GetSize();

				const std::size_t current_size		= archetype->entities.size() * component_size;
				const std::size_t new_size			= component_count * component_size;

				if (new_size > current_size)
				{
					archetype->component_data_size[i] = new_size;
					ComponentData new_data = std::make_unique<ByteArray>(archetype->component_data_size[i]);

					for (std::size_t j = 0; j < archetype->entities.size(); ++j)
					{
						component->MoveDestroyData(
							&archetype->component_data[i][j * component_size],
							&new_data[j * component_size]);
					}

					archetype->component_data[i] = std::move(new_data);
				}
			}
		}
	}

	template<IsComponents... Cs, class Comp>
	inline void EntityAdmin::SortComponents(Comp&& comparison) requires SameTypeParameter<Comp, std::tuple_element_t<0, std::tuple<Cs...>>, 0, 1>
	{
		using C = typename std::tuple_element_t<0, std::tuple<Cs...>>; // the component that is meant to be sorted

		const ComponentIDs component_ids = SortKeys({ { ComponentAlloc<Cs>::GetTypeID()... } }); // see system.hpp
		const ArchetypeID archetype_id = cu::VectorHash<ComponentIDs>()(component_ids);

		const auto it = m_archetype_map.find(archetype_id);
		if (it == m_archetype_map.end())
			return;

		Archetype* archetype = it->second.front();

		if (archetype->id != archetype_id)
			throw std::runtime_error("the specified archetype does not exist");

		const ComponentTypeID& component_id = ComponentAlloc<C>::GetTypeID();

		const auto cit = m_component_archetypes_map.find(component_id);
		if (cit == m_component_archetypes_map.end())
			return;

		const auto ait = cit->second.find(archetype->id);
		if (ait == cit->second.end())
			return;

		const ArchetypeRecord& a_record = ait->second;

		C* components = reinterpret_cast<C*>(&archetype->component_data[a_record.column][0]);

		std::vector<std::size_t> indices(archetype->entities.size());
		std::iota(indices.begin(), indices.end(), 0);

		std::stable_sort(indices.begin(), indices.end(),
			[&comparison, &components](const std::size_t lhs, std::size_t rhs)
			{
				return std::forward<Comp>(comparison)(components[lhs], components[rhs]);
			});

		for (std::size_t i = 0; i < archetype->type.size(); ++i) // sort the components, all need to be sorted
		{
			const ComponentTypeID component_id	= archetype->type[i];
			const IComponentAlloc* component	= m_component_map[component_id].get();
			const std::size_t& component_size	= component->GetSize();

			ComponentData new_data = std::make_unique<ByteArray>(archetype->component_data_size[i]);

			for (std::size_t j = 0; j < archetype->entities.size(); ++j)
			{
				component->MoveDestroyData(*this, archetype->entities[j],
					&archetype->component_data[i][indices[j] * component_size],
					&new_data[j * component_size]);
			}

			archetype->component_data[i] = std::move(new_data);
		}

		std::vector<EntityID> new_entities;
		new_entities.reserve(archetype->entities.size());

		for (std::size_t i = 0; i < archetype->entities.size(); ++i) // now swap the entities
		{
			const std::size_t index = indices[i];
			const EntityID entity_id = archetype->entities[index];

			auto it = m_entity_archetype_map.find(entity_id);
			assert(it != m_entity_archetype_map.end()); // should never happen

			it->second.index = i;
			new_entities.push_back(entity_id);
		}

		archetype->entities = new_entities;
	}
}






// -- old add component --

//// cant add multiple of the same component to same entity
//if (std::find(old_archetype->type.begin(), old_archetype->type.end(), new_comp_type_id) != old_archetype->type.end())
//	return nullptr;

//ArchetypeID new_archetype_id = old_archetype->type;	  // create copy
//cu::InsertSorted(new_archetype_id, new_comp_type_id); // insert while keeping the vector sorted (this should ensure that the archetype is always sorted)

//new_archetype = GetArchetype(new_archetype_id); // create the new archetype

//for (std::size_t i = 0; i < new_archetype_id.size(); ++i) // for every component in the new archetype
//{
//	const ComponentTypeID& component_id = new_archetype_id[i];
//	const ComponentBase* component = m_component_map[component_id].get();
//	const std::size_t& component_size = component->GetSize();

//	const std::size_t current_size = new_archetype->entities.size() * component_size;
//	const std::size_t new_size = current_size + component_size;

//	if (new_size > new_archetype->component_data_size[i])
//		MakeRoom(new_archetype, component, component_size, i); // make room to fit old data

//	bool found = MoveComponent(old_archetype, new_archetype, component, component_id, record.index * component_size, current_size, i);

//	if (!found) // if there was no old instance we could copy our data to, construct a new component
//	{
//		new_component = new (&new_archetype->component_data[i][current_size])
//			C(std::forward<Args>(args)...);
//	}
//}

//const ArchetypeID& old_archetype_id = old_archetype->type;
//for (std::size_t i = 0; i < old_archetype_id.size(); ++i) // we'll have to remove this entity's data in the old as it has been assigned a new one
//{
//	const ComponentTypeID& component_id = old_archetype_id[i];
//	const ComponentBase* component = m_component_map[component_id].get();

//	EraseComponent(old_archetype, component, record.index, i);
//}

//auto it = std::find(old_archetype->entities.begin(), old_archetype->entities.end(), entity_id); // look at swap and pop

//std::for_each(it, old_archetype->entities.end(),
//	[this, &old_archetype](const EntityID& eid)
//	{
//		--m_entity_archetype_map[eid].index;
//	});

//old_archetype->entities.erase(it); // may result in dangling empty archetypes, but is better than having to continously reallocate new memory



// -- old remove component --

//ArchetypeID new_archetype_id = old_archetype->type; // create copy

//new_archetype_id.erase(std::remove(
//	new_archetype_id.begin(), new_archetype_id.end(), comp_type_id), new_archetype_id.end()); // remove this component

////std::sort(new_archetype_id.begin(), new_archetype_id.end()); should not be needed

//Archetype* new_archetype = GetArchetype(new_archetype_id);

//for (std::size_t i = 0; i < new_archetype_id.size(); ++i)
//{
//	const ComponentTypeID& component_id = new_archetype_id[i];
//	const ComponentBase* component = m_component_map[component_id].get();
//	const std::size_t& component_size = component->GetSize();

//	const std::size_t current_size = new_archetype->entities.size() * component_size;
//	const std::size_t new_size = current_size + component_size;

//	if (new_size > new_archetype->component_data_size[i])
//		MakeRoom(new_archetype, component, component_size, i); // make room to fit data

//	component->ConstructData(&new_archetype->component_data[i][current_size]);

//	bool found = MoveComponent(old_archetype, new_archetype, component, component_id, record.index * component_size, current_size, i);

//	assert(found);
//}

//const ArchetypeID& old_archetype_id = old_archetype->type;
//for (std::size_t i = 0; i < old_archetype_id.size(); ++i)
//{
//	const ComponentTypeID& component_id = old_archetype_id[i];
//	const ComponentBase* component = m_component_map[component_id].get();

//	if (component_id == comp_type_id)
//	{
//		const std::size_t& component_size = component->GetSize();
//		component->DestroyData(&old_archetype->component_data[i][record.index * component_size]);
//	}

//	EraseComponent(old_archetype, component, record.index, i);
//}

//auto it = std::find(
//	old_archetype->entities.begin(),
//	old_archetype->entities.end(), entity_id);

//std::for_each(it, old_archetype->entities.end(),
//	[this](const EntityID& eid)
//	{
//		--m_entity_archetype_map[eid].index;
//	});

//old_archetype->entities.erase(std::remove(
//	old_archetype->entities.begin(), old_archetype->entities.end(), entity_id),
//	old_archetype->entities.end());

//new_archetype->entities.push_back(entity_id);
//record.index = new_archetype->entities.size() - 1;
//record.archetype = new_archetype;