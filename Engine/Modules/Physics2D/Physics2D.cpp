#include "kgpch.h"

#include "Modules/Physics2D/Physics2D.h"

#include "Modules/Core/Engine.h"
#include "Modules/Scenes/Assets/Scene.h"
#include "Modules/ECS/Entity.h"
#include "Modules/Core/Components/Transform.h"
#include "Modules/Physics2D/Components/BoxCollider2DComponent.h"
#include "Modules/Physics2D/Components/CircleCollider2DComponent.h"
#include "Modules/Physics2D/Components/RigidBody2DComponent.h"

#include "Modules/Physics2D/ExternalAPI/Box2DBackend.h"

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



	bool Physics2DWorld::Init(Scenes::Scene* scene, PhysicsSpecification& physicsSpec)
	{
		KG_ASSERT(scene);

		// Initialize Physics2DWorld with selected settings.
		i_Scene = scene;
		m_PhysicsWorld = CreateScope<b2World>(b2Vec2(physicsSpec.Gravity.x, physicsSpec.Gravity.y));
		m_PhysicsWorld->SetAllowSleeping(false);
		m_ContactListener = CreateScope<ContactListener>();
		EngineService::GetActiveEngine().RegisterCollisionEventListener(*m_ContactListener);
		m_PhysicsWorld->SetContactListener(m_ContactListener.get());

		// Register each entity into the Physics2DWorld
		auto rigidBodyView = scene->m_EntityRegistry.GetView<Physics2D::Rigidbody2DComponent>();
		for (auto enttID : rigidBodyView)
		{
			ECS::Entity entity = scene->m_EntityRegistry.GetEntityByECSID(enttID);
			Transform& transform = entity.GetComponent<Transform>();
			Physics2D::Rigidbody2DComponent& rb2d = entity.GetComponent<Physics2D::Rigidbody2DComponent>();

			b2BodyDef bodyDef;
			bodyDef.type = Utility::Rigidbody2DTypeToBox2DBody(rb2d.m_Type);
			bodyDef.position.Set(transform.m_Translation.x, transform.m_Translation.y);
			bodyDef.angle = transform.m_Rotation.z;

			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			body->SetFixedRotation(rb2d.m_FixedRotation);
			b2BodyUserData& bodyUser = body->GetUserData();
			bodyUser.UUID = entity.GetUUID();
			rb2d.m_RuntimeBody = body;

			if (entity.HasComponent<Physics2D::BoxCollider2DComponent>())
			{
				Physics2D::BoxCollider2DComponent& boxColliderComp = entity.GetComponent<Physics2D::BoxCollider2DComponent>();
				b2Vec2 offsets{ boxColliderComp.m_Offset.y, -boxColliderComp.m_Offset.x };
				b2PolygonShape boxShape;
				boxShape.SetAsBox(boxColliderComp.m_Size.x * transform.m_Scale.x, boxColliderComp.m_Size.y * transform.m_Scale.y,
					offsets, 0);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = boxColliderComp.m_Density;
				fixtureDef.friction = boxColliderComp.m_Friction;
				fixtureDef.restitution = boxColliderComp.m_Restitution;
				fixtureDef.restitutionThreshold = boxColliderComp.m_RestitutionThreshold;
				fixtureDef.isSensor = boxColliderComp.m_IsSensor;
				body->CreateFixture(&fixtureDef);
			}

			if (entity.HasComponent<Physics2D::CircleCollider2DComponent>())
			{
				Physics2D::CircleCollider2DComponent& circleColliderComponent = entity.GetComponent<Physics2D::CircleCollider2DComponent>();

				b2CircleShape circleShape;
				circleShape.m_p.Set(circleColliderComponent.m_Offset.x, circleColliderComponent.m_Offset.y);
				circleShape.m_radius = transform.m_Scale.x * circleColliderComponent.m_Radius;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circleShape;
				fixtureDef.density = circleColliderComponent.m_Density;
				fixtureDef.friction = circleColliderComponent.m_Friction;
				fixtureDef.restitution = circleColliderComponent.m_Restitution;
				fixtureDef.restitutionThreshold = circleColliderComponent.m_RestitutionThreshold;
				fixtureDef.isSensor = circleColliderComponent.m_IsSensor;
				body->CreateFixture(&fixtureDef);
			}
		}

		return true;
	}

	bool Physics2DWorld::Terminate()
	{
		m_ContactListener = {};
		m_PhysicsWorld = nullptr;

		return true;
	}

	void Physics2DWorld::OnUpdate(Timestep ts)
	{
		const int32_t velocityIterations = 6;
		const int32_t positionIterations = 2;
		m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

		// Retrieve transform from Box2D
		auto view = i_Scene->m_EntityRegistry.GetView<Physics2D::Rigidbody2DComponent>();
		for (auto enttID : view)
		{
			ECS::Entity entity = i_Scene->m_EntityRegistry.GetEntityByECSID(enttID);
			Transform& transform = entity.GetComponent<Transform>();
			Physics2D::Rigidbody2DComponent& rb2d = entity.GetComponent<Physics2D::Rigidbody2DComponent>();

			b2Body* body = (b2Body*)rb2d.m_RuntimeBody;
			const auto& position = body->GetPosition();
			transform.m_Translation.x = position.x;
			transform.m_Translation.y = position.y;
			transform.m_Rotation.z = body->GetAngle();
			// TODO FOR DEBUGGING
			KG_ASSERT(!std::isnan(position.x) && !std::isnan(position.y) && !std::isnan(body->GetAngle()));
		}
	}

	RaycastResult Physics2DWorld::Raycast(Math::vec2 startPoint, Math::vec2 endPoint)
	{
		RayCastCallback newCallback;

		if (std::isnan(startPoint.x) || std::isnan(startPoint.y) || std::isnan(endPoint.y) || std::isnan(endPoint.y))
		{
			KG_WARN("A not a number float was found as input to a 2D raycast call!");
			return RaycastResult(false, Assets::k_EmptyHandle);
		}

		m_PhysicsWorld->RayCast(&newCallback, b2Vec2(startPoint.x, startPoint.y), b2Vec2(endPoint.x, endPoint.y));
		if (newCallback.m_Fixture)
		{
			return RaycastResult(true,
				newCallback.m_Fixture->GetBody()->GetUserData().UUID,
				{ newCallback.m_NormalVector.x, newCallback.m_NormalVector.y },
				{ newCallback.m_ContactPoint.x, newCallback.m_ContactPoint.y });
		}
		else
		{
			return RaycastResult(false, Assets::k_EmptyHandle);
		}
	}

	void Physics2DWorld::SetActiveGravity(const Math::vec2& gravity)
	{
		m_PhysicsWorld->SetGravity(b2Vec2(gravity.x, gravity.y));
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
