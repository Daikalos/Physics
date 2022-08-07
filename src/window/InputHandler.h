#pragma once

#include <SFML/Graphics.hpp>

#include <unordered_map>
#include <string>

namespace fge
{
	enum KeyName
	{
		Up,
		Left,
		Right,
		Down,

		KeyNameCount
	};

	enum ButtonName
	{
		Drag,

		ButtonNameCount
	};

	// translates basic input from keyboard and mouse for more extensible usage
	//
	class InputHandler
	{
	public:
		InputHandler();
		~InputHandler();

		// call at start of loop before poll event
		//
		bool update();

	public:
		inline bool get_button_pressed(const sf::Mouse::Button& button) const
		{
			if (button < 0 || button >= sf::Mouse::ButtonCount)
				return false;

			return _current_button_state[button] && !_previous_button_state[button];
		}
		inline bool get_button_released(const sf::Mouse::Button& button) const
		{
			if (button < 0 || button >= sf::Mouse::ButtonCount)
				return false;

			return !_current_button_state[button] && _previous_button_state[button];
		}
		inline bool get_button_held(const sf::Mouse::Button& button) const
		{
			if (button < 0 || button >= sf::Mouse::ButtonCount)
				return false;

			return _current_button_state[button] && _previous_button_state[button];
		}

		inline bool get_key_pressed(const sf::Keyboard::Key& key) const
		{
			if (key < 0 || key >= sf::Keyboard::Key::KeyCount)
				return false;

			return (_current_key_state[key] && !_previous_key_state[key]);
		}
		inline bool get_key_released(const sf::Keyboard::Key& key) const
		{
			if (key < 0 || key >= sf::Keyboard::Key::KeyCount)
				return false;

			return (!_current_key_state[key] && _previous_key_state[key]);
		}
		inline bool get_key_held(const sf::Keyboard::Key& key) const
		{
			if (key < 0 || key >= sf::Keyboard::Key::KeyCount)
				return false;

			return _current_key_state[key] && _previous_key_state[key];
		}

		inline bool get_key_pressed(const KeyName& name) const { return _key_bindings.contains(name) && get_key_pressed(_key_bindings.at(name)); }
		inline bool get_key_released(const KeyName& name) const { return _key_bindings.contains(name) && get_key_released(_key_bindings.at(name)); }
		inline bool get_key_held(const KeyName& name) const { return _key_bindings.contains(name) && get_key_held(_key_bindings.at(name)); }

		inline bool get_button_pressed(const ButtonName& name) const { return _button_bindings.contains(name) && get_button_pressed(_button_bindings.at(name)); }
		inline bool get_button_released(const ButtonName& name) const { return _button_bindings.contains(name) && get_button_released(_button_bindings.at(name)); }
		inline bool get_button_held(const ButtonName& name) const { return _button_bindings.contains(name) && get_button_held(_button_bindings.at(name)); }

		inline bool get_scroll_up() const { return _scroll_delta > 0; }
		inline bool get_scroll_down() const { return _scroll_delta < 0; }

		void set_scroll_delta(float scroll_delta)
		{
			_scroll_delta = scroll_delta;
		}

		void set_key_binding(KeyName name, sf::Keyboard::Key key)
		{
			_key_bindings[name] = key;
		}
		void set_button_binding(ButtonName name, sf::Mouse::Button button)
		{
			_button_bindings[name] = button;
		}

	private:
		bool _current_button_state[sf::Mouse::ButtonCount];
		bool _previous_button_state[sf::Mouse::ButtonCount];

		bool _current_key_state[sf::Keyboard::KeyCount];
		bool _previous_key_state[sf::Keyboard::KeyCount];

		float _scroll_delta;

		std::unordered_map<KeyName, sf::Keyboard::Key> _key_bindings;
		std::unordered_map<ButtonName, sf::Mouse::Button> _button_bindings;

		int _joystick_count;
		bool _available_joysticks[sf::Joystick::Count];
	};
}
