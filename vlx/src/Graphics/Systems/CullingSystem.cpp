#include <Velox/Graphics/Systems/CullingSystem.h>

using namespace vlx;

CullingSystem::CullingSystem(EntityAdmin& entity_admin, const LayerType id, const Camera& camera)
	: SystemAction(entity_admin, id), m_system(entity_admin, id), m_camera(&camera)
{
	m_system.All([this, &camera](std::span<const EntityID> entities, Renderable* renderables, GlobalTransform* global_transforms, Sprite* sprites)
		{
			const Vector2f camera_size = camera.GetSize() / camera.GetScale();
			const Vector2f camera_pos = camera.GetPosition() - camera_size / 2.0f;

			const RectFloat camera_rect = 
			{ 
				camera_pos.x - LENIENCY,
				camera_pos.y - LENIENCY,
				camera_size.x + LENIENCY,
				camera_size.y + LENIENCY
			};

			const RectFloat gui_camera_rect =
			{
				-LENIENCY,
				-LENIENCY,
				camera.GetSize().x + LENIENCY,
				camera.GetSize().y + LENIENCY
			};

			for (std::size_t i = 0; i < entities.size(); ++i)
			{
				Renderable&	renderable		= renderables[i];
				GlobalTransform& transform	= global_transforms[i];
				Sprite&	sprite				= sprites[i];

				const RectFloat rect = transform.GetTransform().TransformRect(RectFloat({ 0, 0 }, sprite.GetSize()));
				renderable.IsVisible = renderable.IsGUI ? gui_camera_rect.Overlaps(rect) : camera_rect.Overlaps(rect);
			}
		});
}

bool CullingSystem::IsRequired() const noexcept
{
	return false;
}

void CullingSystem::Start()
{

}

void CullingSystem::PreUpdate()
{

}

void CullingSystem::Update()
{
	Execute();
}

void CullingSystem::FixedUpdate()
{

}

void CullingSystem::PostUpdate()
{

}
