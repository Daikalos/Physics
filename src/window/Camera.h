#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <unordered_map>
#include <memory>
#include <type_traits>

#include "../utilities/VectorUtilities.h"
#include "../utilities/NonCopyable.h"
#include "CameraBehaviour.hpp"
#include "Cameras.h"

namespace fge
{
	class Camera final : public sf::View, NonCopyable
	{
	private:
		using Stack = typename std::vector<CameraBehaviour::Ptr>;
		using Factory = typename std::unordered_map<Cameras::ID, CameraBehaviour::Func>;

		enum class Action
		{
			Push,
			Pop,
			Erase,
			Clear
		};

		template<typename...  Args>
		struct PendingChange
		{
			using Ptr = typename std::unique_ptr<PendingChange>;

			explicit PendingChange(const Action& action, const Cameras::ID& camera_id = Cameras::ID::None, Args&&... args)
				: action(action), camera_id(camera_id), args(std::forward<Args>(args)...) { }

			const Action				action;
			const Cameras::ID			camera_id;
			const std::tuple<Args...>	args;
		};

	public:
		explicit Camera(CameraBehaviour::Context context);

		sf::Transform GetViewMatrix() const;
		sf::Vector2f ViewToWorld(const sf::Vector2f& position) const;
		sf::Vector2f GetMouseWorldPosition(const sf::RenderWindow& window) const;

		sf::Vector2f GetPosition() const noexcept;
		sf::Vector2f GetScale() const noexcept;
		sf::Vector2f GetSize() const noexcept;

		void SetPosition(const sf::Vector2f& position);
		void SetScale(const sf::Vector2f& scale);
		void SetSize(const sf::Vector2f& size);

	public:
		void HandleEvent(const sf::Event& event);

		void PreUpdate(const Time& time);
		void Update(const Time& time);
		void FixedUpdate(const Time& time);
		void PostUpdate(const Time& time, float interp);

		template<typename... Args>
		void Push(const Cameras::ID& camera_id, Args&&... args);
		void Erase(const Cameras::ID& camera_id);
		void Pop();
		void Clear();

		template<class T, typename... Args, typename std::enable_if_t<std::is_base_of_v<CameraBehaviour, T>, bool> = true>
		void RegisterBehaviour(const Cameras::ID& camera_id, Args&&... args);
		CameraBehaviour* GetBehaviour(const Cameras::ID& camera_id);

	private:
		CameraBehaviour::Ptr CreateBehaviour(const Cameras::ID& camera_id);
		void ApplyPendingChanges();

	private:
		CameraBehaviour::Context	m_context;

		sf::Vector2f				m_position;
		sf::Vector2f				m_scale;	 
		sf::Vector2f				m_size;

		Stack						m_stack;		// stack of camera behaviours
		Factory						m_factory;		// stores funcs for creating camera behaviours
		std::vector<PendingChange>	m_pending_list; // TODO: ALLOW FOR STORING VARIADIC PARAMETERS AND PASS THEM TO ONCREATE FOR BEHAVIOUR CHILDREN, dunno if possible even, should reconsider structure if not
	};

	template<typename ...Args>
	inline void Camera::Push(const Cameras::ID& camera_id, Args&&... args)
	{
		m_pending_list.push_back(PendingChange(Action::Push, camera_id, std::forward<Args>(args)...));
	}

	template<class T, typename... Args, typename std::enable_if_t<std::is_base_of_v<CameraBehaviour, T>, bool>>
	inline void Camera::RegisterBehaviour(const Cameras::ID& camera_id, Args&&... args)
	{
		m_factory[camera_id] = [this, &args...]()
		{
			return CameraBehaviour::Ptr(new T(camera_id, *this, m_context, std::forward<Args>(args)...));
		};
	}
}

