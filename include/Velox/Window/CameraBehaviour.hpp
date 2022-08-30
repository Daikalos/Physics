#pragma once

#include <SFML/Graphics.hpp>

#include <Velox/Utilities.hpp>
#include <Velox/Input.hpp>

#include <functional>

#include "Window.h"
#include "Cameras.h"

namespace vlx
{
	class Camera;

	////////////////////////////////////////////////////////////
	// Behaviour for the camera, e.g., attach, dragging, 
	// lerp, shake, letterboxview, etc. The idea for Camera
	// Behaviour is to allow for multiple types of behaviours
	// or effects for the camera that can be easily added or 
	// removed
	////////////////////////////////////////////////////////////
	class CameraBehaviour : private NonCopyable
	{
	public:
		using Ptr = typename std::unique_ptr<CameraBehaviour>;
		using Func = typename std::function<Ptr()>;

		struct Context // holds vital objects
		{
			Context(const Window& window, const ControlMap& controls)
				: window(&window), controls(&controls) { }

			const Window*		window;
			const ControlMap*	controls;
		};

		struct OnCreateArgs { }; // TODO: maybe pass structs as args instead??

	protected:
		Camera& GetCamera() const { return *m_camera; }
		const Context& GetContext() const { return m_context; }

	public:
		CameraBehaviour(camera::ID id, Camera& camera, Context context) :
			m_id(id), m_camera(&camera), m_context(context) { }
		virtual ~CameraBehaviour() = default;

		camera::ID GetId() const noexcept { return m_id; }

		////////////////////////////////////////////////////////////
		// OnActivate is called whenever the behaviour is put as 
		// last in the stack
		////////////////////////////////////////////////////////////
		virtual void OnActivate() {}

		////////////////////////////////////////////////////////////
		// OnDestroy is called when the behaviour is removed from
		// the stack
		////////////////////////////////////////////////////////////
		virtual void OnDestroy() {}

		virtual bool HandleEvent(const sf::Event& event) = 0;

		virtual bool PreUpdate(const Time& time)				{ return true; }
		virtual bool Update(const Time& time) = 0;
		virtual bool FixedUpdate(const Time& time)				{ return true; }
		virtual bool PostUpdate(const Time& time, float interp) { return true; }

	private:
		camera::ID		m_id;
		Camera* const	m_camera;
		Context			m_context;
	};
}