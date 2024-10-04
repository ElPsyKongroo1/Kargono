#include "kgpch.h"

#include "Kargono/Physics/Physics2D.h"

#include "Kargono/Core/Engine.h"
#include "Kargono/Scenes/Scene.h"
#include "Kargono/ECS/Entity.h"

#include "API/Physics/Box2DBackend.h"

namespace Kargono::Physics
{
	ContactListener::ContactListener()
		: b2ContactListener()
	{
	}
	void ContactListener::BeginContact(b2Contact* contact)
	{
		KG_ASSERT(m_CallbackFunc, "Missing Callback Function to link to Event Dispatch!");
		UUID entityOne = contact->GetFixtureA()->GetBody()->GetUserData().UUID;
		UUID entityTwo = contact->GetFixtureB()->GetBody()->GetUserData().UUID;
		Events::PhysicsCollisionStart event = Events::PhysicsCollisionStart(entityOne, entityTwo);
		m_CallbackFunc(&event);
	}

	void ContactListener::EndContact(b2Contact* contact)
	{
		KG_ASSERT(m_CallbackFunc, "Missing Callback Function to link to Event Dispatch!");
		UUID entityOne = contact->GetFixtureA()->GetBody()->GetUserData().UUID;
		UUID entityTwo = contact->GetFixtureB()->GetBody()->GetUserData().UUID;
		Events::PhysicsCollisionEnd event = Events::PhysicsCollisionEnd(entityOne, entityTwo);
		m_CallbackFunc(&event);
	}

	
	Physics2DWorld::Physics2DWorld(Scenes::Scene* scene, const Math::vec2& gravity)
	{
		// Initialize Physics2DWorld with selected settings.
		m_Scene = scene;
		m_PhysicsWorld = CreateScope<b2World>(b2Vec2(gravity.x, gravity.y));
		m_PhysicsWorld->SetAllowSleeping(false);
		m_ContactListener = CreateScope<ContactListener>();
		EngineService::RegisterCollisionEventListener(*m_ContactListener);
		m_PhysicsWorld->SetContactListener(m_ContactListener.get());

		// Register each entity into the Physics2DWorld
		auto rigidBodyView = scene->GetAllEntitiesWith<ECS::Rigidbody2DComponent>();
		for (auto e : rigidBodyView)
		{
			ECS::Entity entity = { e, &scene->m_Registry };
			auto& transform = entity.GetComponent<ECS::TransformComponent>();
			auto& rb2d = entity.GetComponent<ECS::Rigidbody2DComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = Utility::Rigidbody2DTypeToBox2DBody(rb2d.Type);
			bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
			bodyDef.angle = transform.Rotation.z;

			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			body->SetFixedRotation(rb2d.FixedRotation);
			b2BodyUserData& bodyUser = body->GetUserData();
			bodyUser.UUID = entity.GetUUID();
			rb2d.RuntimeBody = body;

			if (entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				auto& bc2d = entity.GetComponent<ECS::BoxCollider2DComponent>();
				b2Vec2 offsets{ bc2d.Offset.y, -bc2d.Offset.x };
				b2PolygonShape boxShape;
				boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y,
									offsets, 0);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = bc2d.Density;
				fixtureDef.friction = bc2d.Friction;
				fixtureDef.restitution = bc2d.Restitution;
				fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}

			if (entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				auto& circleColliderComponent = entity.GetComponent<ECS::CircleCollider2DComponent>();

				b2CircleShape circleShape;
				circleShape.m_p.Set(circleColliderComponent.Offset.x, circleColliderComponent.Offset.y);
				circleShape.m_radius = transform.Scale.x * circleColliderComponent.Radius;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circleShape;
				fixtureDef.density = circleColliderComponent.Density;
				fixtureDef.friction = circleColliderComponent.Friction;
				fixtureDef.restitution = circleColliderComponent.Restitution;
				fixtureDef.restitutionThreshold = circleColliderComponent.RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}
		}
	}

	Physics2DWorld::~Physics2DWorld()
	{
		m_ContactListener = {};
		m_PhysicsWorld.reset();
		m_PhysicsWorld = nullptr;
	}

	void Physics2DService::Init(Scenes::Scene* scene, PhysicsSpecification& physicsSpec)
	{
		KG_ASSERT(!s_ActivePhysicsWorld, "Attempt to initialize the physics 2D service, however, the service is already active.");
		s_ActivePhysicsWorld = CreateRef<Physics2DWorld>(scene, physicsSpec.Gravity);
	}

	void Physics2DService::Terminate()
	{
		KG_ASSERT(s_ActivePhysicsWorld, "Attempt to terminate the active physics 2D service, however, the service is not currently active.");
		s_ActivePhysicsWorld.reset();
		s_ActivePhysicsWorld = nullptr;
	}

	void Physics2DService::OnUpdate(Timestep ts)
	{
		const int32_t velocityIterations = 6;
		const int32_t positionIterations = 2;
		s_ActivePhysicsWorld->m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

		// Retrieve transform from Box2D
		auto view = s_ActivePhysicsWorld->m_Scene->GetAllEntitiesWith<ECS::Rigidbody2DComponent>();
		for (auto e : view)
		{
			ECS::Entity entity = { e, &s_ActivePhysicsWorld->m_Scene->m_Registry };
			auto& transform = entity.GetComponent<ECS::TransformComponent>();
			auto& rb2d = entity.GetComponent<ECS::Rigidbody2DComponent>();

			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			const auto& position = body->GetPosition();
			transform.Translation.x = position.x;
			transform.Translation.y = position.y;
			transform.Rotation.z = body->GetAngle();
			// TODO FOR DEBUGGING
			KG_ASSERT(!std::isnan(position.x) && !std::isnan(position.y) && !std::isnan(body->GetAngle()));
		}
	}

	void Physics2DService::SetActiveGravity(const Math::vec2& gravity)
	{
		s_ActivePhysicsWorld->m_PhysicsWorld->SetGravity(b2Vec2(gravity.x, gravity.y));
	}

}
