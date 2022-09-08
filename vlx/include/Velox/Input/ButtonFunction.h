#pragma once

#include <Velox/Utilities.hpp>

#include "InputHandler.h"

namespace vlx
{
	enum ButtonEvent : std::uint16_t
	{
		BE_Begin = 0,

		BE_Pressed = 0,
		BE_Released = 1,
		BE_Held = 2,

		BE_End = 3
	};

	inline ButtonEvent& operator++(ButtonEvent& button_event)
	{
		button_event = static_cast<ButtonEvent>(static_cast<std::uint16_t>(button_event) + 1);
		return button_event;
	}

	template<std::derived_from<InputHandler> T, Enum ButtonType> requires IsButtonInput<T, ButtonType>
	class ButtonFunction final
	{
	public:
		using ButtonFunc	= typename std::array<std::function<void()>, BE_End>;
		using BoundFunc		= typename std::pair<ButtonType, ButtonFunc>;

	public:
		ButtonFunction() = default;
		ButtonFunction(const T* input);
	
		template<typename Func, class U>
		void Add(ButtonType button, ButtonEvent event, Func&& func, U* object);

		template<Enum Bind, typename Func, class U>
		void Add(Bind key, ButtonEvent event, Func&& func, U* object);

	public:
		void Execute();
		void operator()();

	private:
		const T* m_input{nullptr};
		std::vector<BoundFunc> m_funcs;
	};

	template<std::derived_from<InputHandler> T, Enum ButtonType> requires IsButtonInput<T, ButtonType>
	inline ButtonFunction<T, ButtonType>::ButtonFunction(const T* input) : m_input(input) {}

	template<std::derived_from<InputHandler> T, Enum ButtonType> requires IsButtonInput<T, ButtonType>
	template<typename Func, class U>
	inline void ButtonFunction<T, ButtonType>::Add(ButtonType key, ButtonEvent event, Func&& func, U* object)
	{
		ButtonFunc& funcs = m_funcs.emplace_back(key, ButtonFunc()).second;
		funcs[event] = std::bind(std::forward<Func>(func), object);
	}

	template<std::derived_from<InputHandler> T, Enum ButtonType> requires IsButtonInput<T, ButtonType>
	template<Enum Bind, typename Func, class U>
	inline void ButtonFunction<T, ButtonType>::Add(Bind key, ButtonEvent event, Func&& func, U* object)
	{
		const auto& binds = m_input->GetMap<Bind>();
		Add(binds.At(key), event, std::forward<Func>(func), object);
	}

	template<std::derived_from<InputHandler> T, Enum ButtonType> requires IsButtonInput<T, ButtonType>
	inline void ButtonFunction<T, ButtonType>::Execute()
	{
		for (const BoundFunc& pair : m_funcs)
		{
			const ButtonType& key = pair.first;
			auto& button_func = pair.second;

			for (ButtonEvent event = BE_Begin; event != BE_End; ++event)
			{
				bool triggered = false;

				switch (event)
				{
				case BE_Pressed:	triggered = m_input->Pressed(key);	break;
				case BE_Released:	triggered = m_input->Released(key);	break;
				case BE_Held:		triggered = m_input->Held(key);		break;
				}

				if (triggered)
				{
					const auto& func = button_func[event];
					if (func) func();
				}
			}
		}
	}

	template<std::derived_from<InputHandler> T, Enum ButtonType> requires IsButtonInput<T, ButtonType>
	inline void ButtonFunction<T, ButtonType>::operator()()
	{
		Execute();
	}
}