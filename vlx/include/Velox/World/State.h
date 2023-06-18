#pragma once

#include <memory>
#include <functional>

#include <SFML/Graphics.hpp>

#include <Velox/Config.hpp>
#include <Velox/System/Time.h>

#include "Identifiers.h"

namespace vlx
{
	class StateStack;
	class World;

	class VELOX_API State
	{
	public:
		using Ptr = std::unique_ptr<State>;
		using Func = std::function<Ptr()>;

	public:
		State(StateStack& state_stack, World& world, StateID id);
		virtual ~State() = default;

	public:
		NODISC StateID GetID() const noexcept;

	protected:
		NODISC const StateStack& GetStack() const;
		NODISC StateStack& GetStack();

		NODISC const World& GetWorld() const;
		NODISC World& GetWorld();

	public:
		///	OnCreated is called when first constructed
		/// 
		virtual void OnCreated() {}

		///	OnActivate is called whenever the state is put as last in the stack
		/// 
		virtual void OnActivate() {}

		///	OnDestroy is called when the state is removed from the stack
		/// 
		virtual void OnDestroy() {}

		virtual bool HandleEvent(const sf::Event& event) = 0;

		virtual bool Start(Time& time)			{ return true; }
		virtual bool PreUpdate(Time& time)		{ return true; }
		virtual bool Update(Time& time) = 0;
		virtual bool FixedUpdate(Time& time)	{ return true; }
		virtual bool PostUpdate(Time& time)		{ return true; }

		virtual void Draw() = 0;

	private:
		StateStack* m_state_stack	{nullptr};
		World*		m_world			{nullptr};
		StateID		m_id			{-1};
	};
}