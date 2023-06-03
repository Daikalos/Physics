#pragma once

#include <SFML/Graphics.hpp>

#include <Velox/ECS/Identifiers.hpp>

#include <Velox/Graphics.hpp>
#include <Velox/Input.hpp>
#include <Velox/Window.hpp>
#include <Velox/Config.hpp>

#include <Velox/Graphics/Components/Renderable.h>
#include <Velox/Graphics/Components/GlobalTransform.h>

#include <Velox/ECS/SystemAction.h>

#include <Velox/World/EngineBinds.h>

namespace vlx::gui
{
	class VELOX_API GUISystem final : public SystemAction
	{
	public:
		GUISystem(EntityAdmin& entity_admin, const LayerType id, const Camera& camera, const ControlMap& controls);

	public:
		bool IsRequired() const noexcept override;

	public:
		void Start() override;
		void PreUpdate() override;
		void Update() override;
		void FixedUpdate() override;
		void PostUpdate() override;

	private:
		System<Renderable, GlobalTransform, Button>	m_button_system;
		System<Renderable, GlobalTransform, Label>	m_label_system;
	};
}