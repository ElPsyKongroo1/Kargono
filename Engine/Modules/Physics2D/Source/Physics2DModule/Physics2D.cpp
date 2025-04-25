#include "kgpch.h"

#include "Physics2DModule/Physics2D.h"

#include "Kargono/Core/Engine.h"
#include "Kargono/Scenes/Scene.h"
#include "ECSModule/Entity.h"

#include "Physics2DModule/ExternalAPI/Box2DBackend.h"

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
		for (auto enttID : rigidBodyView)
		{
			ECS::Entity entity = scene->GetEntityByEnttID(enttID);
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
				ECS::BoxCollider2DComponent& boxColliderComp = entity.GetComponent<ECS::BoxCollider2DComponent>();
				b2Vec2 offsets{ boxColliderComp.Offset.y, -boxColliderComp.Offset.x };
				b2PolygonShape boxShape;
				boxShape.SetAsBox(boxColliderComp.Size.x * transform.Scale.x, boxColliderComp.Size.y * transform.Scale.y,
									offsets, 0);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = boxColliderComp.Density;
				fixtureDef.friction = boxColliderComp.Friction;
				fixtureDef.restitution = boxColliderComp.Restitution;
				fixtureDef.restitutionThreshold = boxColliderComp.RestitutionThreshold;
				fixtureDef.isSensor = boxColliderComp.IsSensor;
				body->CreateFixture(&fixtureDef);
			}

			if (entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				ECS::CircleCollider2DComponent& circleColliderComponent = entity.GetComponent<ECS::CircleCollider2DComponent>();

				b2CircleShape circleShape;
				circleShape.m_p.Set(circleColliderComponent.Offset.x, circleColliderComponent.Offset.y);
				circleShape.m_radius = transform.Scale.x * circleColliderComponent.Radius;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circleShape;
				fixtureDef.density = circleColliderComponent.Density;
				fixtureDef.friction = circleColliderComponent.Friction;
				fixtureDef.restitution = circleColliderComponent.Restitution;
				fixtureDef.restitutionThreshold = circleColliderComponent.RestitutionThreshold;
				fixtureDef.isSensor = circleColliderComponent.IsSensor;
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
		for (auto enttID : view)
		{
			ECS::Entity entity = s_ActivePhysicsWorld->m_Scene->GetEntityByEnttID(enttID);
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

	RaycastResult Physics2DService::Raycast(Math::vec2 startPoint, Math::vec2 endPoint)
	{
		RayCastCallback newCallback;

		if (std::isnan(startPoint.x) || std::isnan(startPoint.y) || std::isnan(endPoint.y) || std::isnan(endPoint.y))
		{
			KG_WARN("A not a number float was found as input to a 2D raycast call!");
			return RaycastResult(false, Assets::EmptyHandle);
		}

		s_ActivePhysicsWorld->m_PhysicsWorld->RayCast(&newCallback, b2Vec2(startPoint.x, startPoint.y), b2Vec2(endPoint.x, endPoint.y));
		if (newCallback.m_Fixture)
		{
			return RaycastResult(true,
				newCallback.m_Fixture->GetBody()->GetUserData().UUID,
				{ newCallback.m_NormalVector.x, newCallback.m_NormalVector.y },
				{ newCallback.m_ContactPoint.x, newCallback.m_ContactPoint.y });
		}
		else
		{
			return RaycastResult(false, Assets::EmptyHandle);
		}
	}

	void Physics2DService::SetActiveGravity(const Math::vec2& gravity)
	{
		s_ActivePhysicsWorld->m_PhysicsWorld->SetGravity(b2Vec2(gravity.x, gravity.y));
	}

	float RayCastCallback::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)
	{
		// Skip all sensors for raycasts
		if (fixture->IsSensor())
		{
			return -1.0f;
		}

		m_Fixture = fixture;
		m_ContactPoint = point;
		m_NormalVector = normal;
		m_Fraction = fraction;
		return fraction;
	}
}
