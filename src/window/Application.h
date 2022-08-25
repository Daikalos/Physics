#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string_view>

#include "../Utilities.hpp"
#include "../Input.hpp"

//#include "../ecs/ECS.hpp"
#include "../scene/StateStack.h"
#include "../graphics/ResourceHolder.hpp"
#include "../utilities/Time.hpp"

#include "Window.h"
#include "Camera.h"
#include "Binds.h"

#include "../test/StateTest.h"

static const std::string DATA_FOLDER = "../data/";
static const std::string AUDIO_FOLDER = DATA_FOLDER + "audio/";
static const std::string TEXTURE_FOLDER = DATA_FOLDER + "textures/";

namespace vlx
{
	class Application final
	{
	private:
		using DefKeyboard = typename KeyboardInputBindable<bn::Key>;
		using DefMouse = typename MouseInputBindable<bn::Button>;

		using XboxHandler = typename JoystickInputBindable<bn::XboxButton, bn::XboxAxis>;
		using PsHandler = typename JoystickInputBindable<bn::PlaystationButton, bn::PlaystationAxis>;

	public:
		Application(std::string_view name);
		~Application();

		void Run();

	private:
		void ProcessEvents();

		void PreUpdate();
		void Update();
		void FixedUpdate();
		void PostUpdate(float interp);

		void Draw();
		
		void RegisterStates();
		void RegisterControls();
		void LoadMainTextures();

	private:
		Window			m_window;
		Camera			m_camera;
		StateStack		m_state_stack;
		TextureHolder	m_texture_holder;
		FontHolder		m_font_holder;
		Time			m_time;
		Controls		m_controls;
	};
}

