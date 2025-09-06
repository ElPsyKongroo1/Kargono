#pragma once

#include "Modules/ECS/EngineComponents.h"
#include "Modules/Events/PhysicsEvent.h"
#include "Kargono/Core/Timestep.h"
#include "Kargono/Core/Base.h"
#include "Modules/Physics2D/Physics2DCommon.h"

#include "Modules/Physics2D/ExternalAPI/Box2DAPI.h"

namespace Kargono::Scenes { class Scene; }

namespace Kargono::Physics
{
	class ContactListener : public b2ContactListener
	{
	public:
		//=========================
		// Constructor(s)
		//=========================
		ContactListener();
		//=========================
		// Callback Event
		//=========================
		void SetEventCallback(const Events::EventCallbackFn& callback) { m_CallbackFunc = callback; }
		//=========================
		// Collision Detection
		//=========================
		// Implemented function that receives box2d physics event through V-table.
		virtual void BeginContact(b2Contact* contact) override;
		virtual void EndContact(b2Contact* contact) override;
	private:
		// Event callback function pointer
		Events::EventCallbackFn m_CallbackFunc;
	};

	class RayCastCallback : public b2RayCastCallback
	{
	public:
		virtual float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
			const b2Vec2& normal, float fraction) override;

		b2Fixture* m_Fixture{ nullptr };
		b2Vec2 m_ContactPoint;
		b2Vec2 m_NormalVector;
		float m_Fraction;
	};

	struct PhysicsSpecification
	{
		Math::vec2 Gravity { 0.0f, 0.0f };
	};

	class Physics2DWorld
	{
	public:
		//=========================
		// Constructors and Destructors
		//=========================
		Physics2DWorld() = default;
		~Physics2DWorld() = default;
	public:
		//=========================
		// Lifecycle Functions
		//=========================
		[[nodiscard]] bool Init(Scenes::Scene* scene, PhysicsSpecification& physicsSpec);
		[[nodiscard]] bool Terminate();

		//=========================
		// On Event Functionality
		//=========================
		void OnUpdate(Timestep ts);

		//=========================
		// Interact with Physics2DWorld
		//=========================
		RaycastResult Raycast(Math::vec2 startPoint, Math::vec2 endPoint);

		//=========================
		// Manage Active Physics2DWorld
		//=========================
		void SetActiveGravity(const Math::vec2& gravity);

	private:
		//=========================
		// Internal Fields
		//=========================
		// Box2D world reference
		Scope<b2World> m_PhysicsWorld{ nullptr };
		// Callback API
		Scope<ContactListener> m_ContactListener{ nullptr };

		//=========================
		// Dependency Injection(s)
		//=========================
		Scenes::Scene* i_Scene{ nullptr };
	private:
		friend class Physics2DService;
	};

	class Physics2DService // TODO: REMOVE EWWWWWWW
	{
	public:
		//==============================
		// Create Physics2D Context
		//==============================
		static void CreatePhysics2DWorld()
		{
			// Initialize Physics2DWorld
			if (!s_Physics2DWorld)
			{
				s_Physics2DWorld = CreateRef<Physics::Physics2DWorld>();
			}

			// Verify init is successful
			KG_VERIFY(s_Physics2DWorld, "Physics2D Service System Initiated");
		}
		static void RemovePhysics2DWorld()
		{
			// Clear Physics2DWorld
			s_Physics2DWorld.reset();
			s_Physics2DWorld = nullptr;

			// Verify terminate is successful
			KG_VERIFY(!s_Physics2DWorld, "Physics2D Service System Initiated");
		}
		//==============================
		// Getters/Setters
		//==============================
		static Physics2DWorld& GetActiveContext() { return *s_Physics2DWorld; }
		static bool IsContextActive() { return (bool)s_Physics2DWorld; }
	private:
		//==============================
		// Internal Fields
		//==============================
		static inline Ref<Physics2DWorld> s_Physics2DWorld{ nullptr };
	};
}
