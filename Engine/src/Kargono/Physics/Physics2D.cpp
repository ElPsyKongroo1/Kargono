#include "kgpch.h"

#include "Kargono/Physics/Physics2D.h"

#include "Kargono/Core/Core.h"
#include "Kargono/Scene/Scene.h"
#include "Kargono/Scene/Entity.h"


#include "box2d/b2_contact.h"
#include "box2d/Box2d.h"
#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"

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
		Events::PhysicsCollisionEvent event = Events::PhysicsCollisionEvent(entityOne, entityTwo);
		m_CallbackFunc(event);
	}

	void ContactListener::EndContact(b2Contact* contact)
	{
		KG_ASSERT(m_CallbackFunc, "Missing Callback Function to link to Event Dispatch!");
		UUID entityOne = contact->GetFixtureA()->GetBody()->GetUserData().UUID;
		UUID entityTwo = contact->GetFixtureB()->GetBody()->GetUserData().UUID;
		Events::PhysicsCollisionEnd event = Events::PhysicsCollisionEnd(entityOne, entityTwo);
		m_CallbackFunc(event);
	}

	
	Physics2DWorld::Physics2DWorld(Scene* scene, const Math::vec2& gravity)
	{
		// Initialize Physics2DWorld with selected settings.
		m_Scene = scene;
		m_PhysicsWorld = CreateScope<b2World>(b2Vec2(gravity.x, gravity.y));
		m_PhysicsWorld->SetAllowSleeping(false);
		m_ContactListener = CreateScope<ContactListener>();
		Core::GetCurrentApp().RegisterCollisionEventListener(*m_ContactListener);
		m_PhysicsWorld->SetContactListener(m_ContactListener.get());

		// Register each entity into the Physics2DWorld
		auto rigidBodyView = scene->GetAllEntitiesWith<Rigidbody2DComponent>();
		for (auto e : rigidBodyView)
		{
			Entity entity = { e, scene };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = Utility::Rigidbody2DTypeToBox2DBody(rb2d.Type);
			bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
			bodyDef.angle = transform.Rotation.z;

			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			body->SetFixedRotation(rb2d.FixedRotation);
			b2BodyUserData& bodyUser = body->GetUserData();
			bodyUser.UUID = entity.GetUUID();
			rb2d.RuntimeBody = body;

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
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

			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				auto& circleColliderComponent = entity.GetComponent<CircleCollider2DComponent>();

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

	void Physics2DWorld::OnUpdate(Timestep ts)
	{
		
		const int32_t velocityIterations = 6;
		const int32_t positionIterations = 2;
		m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

		// Retrieve transform from Box2D
		auto view = m_Scene->GetAllEntitiesWith<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, m_Scene };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			const auto& position = body->GetPosition();
			transform.Translation.x = position.x;
			transform.Translation.y = position.y;
			transform.Rotation.z = body->GetAngle();
			// TODO FOR DEBUGGING
			KG_ASSERT(!std::isnan(position.x) && !std::isnan(position.y) && !std::isnan(body->GetAngle()));
		}
		
	}

	void Physics2DWorld::SetGravity(const Math::vec2& gravity)
	{
		m_PhysicsWorld->SetGravity(b2Vec2(gravity.x, gravity.y));
	}

}
