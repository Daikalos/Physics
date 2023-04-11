#pragma once

#include <SFML/Graphics/Transformable.hpp>

#include <Velox/ECS/IComponent.h>
#include <Velox/Graphics/Components/Transform.h>
#include <Velox/System/Rectangle.hpp>
#include <Velox/System/Event.hpp>
#include <Velox/Config.hpp>

#include "../CollisionResult.h"
#include "../PhysicsBody.h"

namespace vlx
{
	class VELOX_API Shape : public IComponent
	{
	public:
		enum Type : short
		{
			None = -1,
			Circle,
			Box,
			Point,
			Convex,

			// Not sure if the engine will support concave shapes, seems very difficult to implement and may cost a lot of performance and memory,
			// http://wscg.zcu.cz/wscg2004/Papers_2004_Full/B83.pdf is the best i've found that presents a viable solution

			Count // always keep last
		};

	public:
		virtual ~Shape() = default;

	public:
		const sf::Transform& GetTransform() const;
		const sf::Transform& GetInverseTransform() const;
		const RectFloat& GetAABB() const;
		Vector2f GetCenter() const;

	protected:
		sf::Transform			m_transform; // transform is needed since physics objects will need to collide around center
		mutable sf::Transform	m_inverse_transform;
		RectFloat				m_aabb;
		mutable bool			m_update_inverse {true};
	};

	template<class S>
	class ShapeCRTP : public Shape
	{
		constexpr auto GetType() const noexcept -> Shape::Type;
		void Initialize(PhysicsBody& body) const;
		void UpdateAABB(const Transform& transform);
		void UpdateTransform(const Transform& transform);

		friend class PhysicsDirtySystem;
	};

	template<class S>
	inline constexpr auto ShapeCRTP<S>::GetType() const noexcept -> Type
	{
		return static_cast<const S*>(this)->GetType();
	}

	template<class S>
	inline void ShapeCRTP<S>::Initialize(PhysicsBody& body) const
	{
		static_cast<const S*>(this)->InitializeImpl(body);
	}

	template<class S>
	inline void ShapeCRTP<S>::UpdateAABB(const Transform& transform)
	{
		static_cast<S*>(this)->UpdateAABBImpl(transform);
	}

	template<class S>
	inline void ShapeCRTP<S>::UpdateTransform(const Transform& transform)
	{
		static_cast<S*>(this)->UpdateTransformImpl(transform);
	}
}