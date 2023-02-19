#pragma once

#include <SFML/Graphics.hpp>

#include <Velox/ECS/Identifiers.hpp>

#include <Velox/Graphics.hpp>
#include <Velox/Input.hpp>
#include <Velox/Window.hpp>
#include <Velox/Config.hpp>

#include <Velox/Graphics/Components/Transform.h>
#include <Velox/Graphics/Components/Object.h>

#include <Velox/ECS/SystemObject.h>

#include <Velox/World/EngineBinds.h>

namespace vlx::gui
{
	class VELOX_API GUISystem : public SystemObject
	{
	public:
		GUISystem(EntityAdmin& entity_admin, const LayerType id, const Camera& camera, const ControlMap& controls);

	public:
		void Update() override;

	private:
		System<Object, Transform, Button>	m_button_system;
		System<Object, Transform, Label>	m_label_system;
	};
}