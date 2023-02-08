#include "StateTest.h"

using namespace vlx;

StateTest::StateTest(const StateID state_id, StateStack& state_stack, World& world)
	: State(state_id, state_stack, world), sys(world.GetEntityAdmin(), 100)
{

}

void StateTest::OnCreated()
{
    m_entity_admin = &GetWorld().GetEntityAdmin();

	ObjectSystem& object_system = GetWorld().GetSystem<ObjectSystem>();

	e0 = object_system.CreateObject();
	e0.AddComponents(ObjectType{});

	e0.GetComponent<Sprite>().SetTexture(GetWorld().GetTextureHolder().Get(Texture::ID::IdleCursor));
	e0.GetComponent<Sprite>().SetOpacity(1.0f);

	et0 = e0.GetComponentRef<LocalTransform>();
	et0->SetPosition({ 0.0f, 0.0f });

	e1 = e0.Duplicate();
	et1 = e1.GetComponentRef<LocalTransform>();

	GetWorld().GetSystem<RelationSystem>().AttachInstant(e0, e1);

	Entity e4 = e1.Duplicate();

	e2 = e1.Duplicate();
	e2.AddComponent<gui::Label>(105, 105);

	gui::Label& label = e2.GetComponent<gui::Label>();
	label.setString("potato");

	m_entities.reserve(100000);

	m_entity_admin->Reserve<Object, LocalTransform, Transform, Relation, Sprite, Velocity>(m_entities.capacity());
	for (int i = 0; i < m_entities.capacity(); ++i)
	{
		Entity& added = m_entities.emplace_back(e1.Duplicate());

		added.AddComponent<Velocity>(
			rnd::random(-50.0f, 50.0f),
			rnd::random(-50.0f, 50.0f));

		added.GetComponent<LocalTransform>().SetPosition({ rnd::random() * 100, rnd::random() * 100 });
		//GetWorld().GetSystem<RelationSystem>().AttachInstant(m_entities.at(rnd::random<int>(0, m_entities.size() - 1)), added);
	}
	
	m_entity_admin->SortComponents<Relation>(m_entities.front(),
		[](const Relation& lhs, const Relation& rhs)
		{
			if (lhs.HasParent() && rhs.HasParent())
				return lhs.GetParent().Get() < rhs.GetParent().Get();

			return false;
		});

	int a = sizeof(Sprite);
	int c = sizeof(Transform);
	int d = sizeof(LocalTransform);
	int e = sizeof(Relation);

	b0 = object_system.CreateObject();
	b0.AddComponents(gui::ButtonType{});

	b0.GetComponent<Object>().IsGUI = true;
	b0.GetComponent<gui::Button>().SetSize({ 128, 128 });
	b0.GetComponent<Sprite>().SetSize({ 128, 128 });
	b0.GetComponent<LocalTransform>().SetPosition({ 100, 100 });
	b0.GetComponent<Sprite>().SetTexture(GetWorld().GetTextureHolder().Get(Texture::ID::Square));
	b0.GetComponent<gui::Button>().Pressed += []() { std::puts("Press"); };
	b0.GetComponent<gui::Button>().Released += []() { std::puts("Relased"); };
	b0.GetComponent<gui::Button>().Entered += []() { std::puts("Entered"); };
	b0.GetComponent<gui::Button>().Exited += []() { std::puts("Exited"); };

	Time& time = GetWorld().GetTime();
	sys.Each([&time](const EntityID entity, Velocity& velocity, LocalTransform& local_transform)
		{
			local_transform.Move(velocity.velocity * time.GetDT());
		});

}

bool StateTest::HandleEvent(const sf::Event& event)
{
    return true;
}

bool StateTest::Update(Time& time)
{
	if (m_entity_admin->IsEntityRegistered(e0))
		et0->Move({ 5.0f * time.GetDT(), 0.0f });

	et1->SetPosition({ 0.0f, 5.0f * time.GetDT() });

	if (GetWorld().GetControls().Get<KeyboardInput>().Pressed(sf::Keyboard::Space))
	{
		GetWorld().GetSystem<ObjectSystem>().DeleteObjectInstant(e0); // TODO: tell children transforms that parent was removed
		for (const Entity& entity : m_entities)
		{
			m_entity_admin->SetComponent(entity, Velocity({ rnd::random(-100.0f, 100.0f), rnd::random(-100.0f, 100.0f) }));
		}
	}

	GetWorld().GetWindow().setTitle(std::to_string(GetWorld().GetTime().GetFramerate()));

	m_entity_admin->RunSystems(100);

    return true;
}

void StateTest::Draw()
{

}
