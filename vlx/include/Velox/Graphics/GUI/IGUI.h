#pragma once

#include <SFML/Graphics.hpp>

#include <Velox/Utilities.hpp>
#include <Velox/ECS/IComponent.h>

namespace vlx::gui
{
	class IGUI : private NonCopyable
	{
	public:
		using Ptr = typename std::shared_ptr<IGUI>;

	public:
		IGUI() = default;
		virtual ~IGUI() = default;

		constexpr bool IsSelected() const noexcept;

		virtual bool IsSelectable() const = 0;

		virtual void Select();
		virtual void Deselect();

		virtual constexpr bool IsActive() const;
		virtual void Activate();
		virtual void Deactivate();

		virtual bool Update(float dt);
		virtual void HandleEvent(const sf::Event& event) = 0;
		virtual bool Draw();

		virtual sf::Vector2f GetSize() const;

	private:
		bool m_selected	{false};
		bool m_active	{false};
	};
}
