#pragma once

#include <string_view>

#include <SFML/Graphics.hpp>

#include <Velox/System/Vector2.hpp>
#include <Velox/Utility/NonCopyable.h>
#include <Velox/Config.hpp>

namespace vlx
{
	///	Settings for type of borders
	///
	enum class WindowBorder
	{
		Windowed,
		Fullscreen,
		BorderlessWindowed // TODO: FIX BORDERLESS WINDOWED ON LOWER RESOLUTION, ALSO LOOK FOR SOLUTION ON FULLSCREEN AT MAX RESOLUTION
	};

	///	Expanded to allow for toggle fullscreen, change resolution, and other settings
	/// 
	class VELOX_API Window final : public sf::RenderWindow, NonCopyable
	{
	public:
		Window(
			std::string name,
			const sf::VideoMode& mode, 
			const WindowBorder& window_border, 
			const sf::ContextSettings& settings, 
			bool vertical_sync, int frame_rate);

		Window(
			std::string name);

		Window(
			std::string name, 
			const sf::VideoMode& mode);

		Window(
			std::string name, 
			const sf::VideoMode& mode,
			const WindowBorder& window_border);

	public:
		
		///	Only gets modes that match the aspect ratio of the desktop
		///
		NODISC const std::vector<sf::VideoMode>& GetValidModes(bool update = false) const;

		NODISC Vector2i GetOrigin() const noexcept;
		NODISC const Vector2f& GetRatioCmp() const noexcept;

	public:
		void onCreate() override;

		void Initialize();
		void HandleEvent(const sf::Event& event);

		void SetFramerate(int frame_rate);
		void SetVerticalSync(bool flag);

		void SetResolution(int index);

		void Build(WindowBorder window_border, sf::VideoMode mode, sf::ContextSettings settings);

		void SetBorder(WindowBorder border);
		void SetMode(sf::VideoMode mode);
		void SetSettings(sf::ContextSettings settings);

	private:
		std::string			m_name;										// name of window
		sf::VideoMode		m_mode;										// current mode set
		WindowBorder		m_border		{WindowBorder::Windowed};	// current border type
		sf::ContextSettings m_settings;									// settings
		Vector2f			m_ratio_cmp;								// current ratio in percentage when compared to default desktop mode
		bool				m_vertical_sync	{false};					// vertical sync
		int					m_frame_rate	{300};						// maximum frame rate
	};
}