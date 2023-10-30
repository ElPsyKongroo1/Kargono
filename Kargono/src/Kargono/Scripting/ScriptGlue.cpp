#include "kgpch.h"

#include "Kargono/Core/Input.h"
#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Scripting/ScriptGlue.h"
#include "Kargono/Scripting/ScriptEngine.h"
#include "Kargono/Physics/Physics2D.h"
#include "Kargono/Scene/Entity.h"
#include "Kargono/Scene/Scene.h"

#include "box2d/b2_body.h"
#include "mono/jit/jit.h"
#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

namespace Kargono
{
	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

#define KG_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Kargono.InternalCalls::" #Name, Name)

	static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		KG_CORE_ASSERT(scene)
		Entity entity = scene->GetEntityByUUID(entityID);
		KG_CORE_ASSERT(entity)

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		KG_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end())
		return s_EntityHasComponentFuncs.at(managedType)(entity);
	}

	static uint64_t Entity_FindEntityByName(MonoString* name)
	{
		char* cStr = mono_string_to_utf8(name);

		Scene* scene = ScriptEngine::GetSceneContext();
		KG_CORE_ASSERT(scene)
		Entity entity = scene->FindEntityByName(cStr);
		mono_free(cStr);

		if (!entity) { return 0; }

		return entity.GetUUID();
	}

	static MonoObject* GetScriptInstance(UUID entityID)
	{
		return ScriptEngine::GetManagedInstance(entityID);
	}

	

	static void TransformComponent_GetTranslation(UUID entityID, glm::vec3* outTranslation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);

		*outTranslation = entity.GetComponent<TransformComponent>().Translation;

	}
	
	static void TransformComponent_SetTranslation(UUID entityID, glm::vec3* translation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);

		entity.GetComponent<TransformComponent>().Translation = *translation;

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			body->SetTransform({ translation->x, translation->y }, body->GetAngle());
		}
		
	}

	static void Rigidbody2DComponent_ApplyLinearImpulse(UUID entityID, glm::vec2* impulse, glm::vec2* point, bool wake)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		KG_CORE_ASSERT(scene)
		Entity entity = scene->GetEntityByUUID(entityID);
		KG_CORE_ASSERT(entity)

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulse(b2Vec2(impulse->x, impulse->y), b2Vec2(point->x, point->y), wake);
	}

	static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID entityID, glm::vec2* impulse, bool wake)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		KG_CORE_ASSERT(scene)
			Entity entity = scene->GetEntityByUUID(entityID);
		KG_CORE_ASSERT(entity)

			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
	}

	static void Rigidbody2DComponent_GetLinearVelocity(UUID entityID, glm::vec2* outLinearVelocity)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		KG_CORE_ASSERT(scene)
			Entity entity = scene->GetEntityByUUID(entityID);
		KG_CORE_ASSERT(entity)

			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		const b2Vec2& linearVelocity = body->GetLinearVelocity();
		*outLinearVelocity = glm::vec2(linearVelocity.x, linearVelocity.y);
	}

	static void Rigidbody2DComponent_SetLinearVelocity(UUID entityID, glm::vec2* linearVelocity)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		KG_CORE_ASSERT(scene)
		Entity entity = scene->GetEntityByUUID(entityID);
		KG_CORE_ASSERT(entity)

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->SetLinearVelocity(b2Vec2(linearVelocity->x, linearVelocity->y));
	}

	static void Rigidbody2DComponent_SetType(UUID entityID, Rigidbody2DComponent::BodyType bodyType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		KG_CORE_ASSERT(scene)
			Entity entity = scene->GetEntityByUUID(entityID);
		KG_CORE_ASSERT(entity)

			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->SetType(Utils::Rigidbody2DTypeToBox2DBody(bodyType));
	}

	static Rigidbody2DComponent::BodyType Rigidbody2DComponent_GetType(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		KG_CORE_ASSERT(scene)
			Entity entity = scene->GetEntityByUUID(entityID);
		KG_CORE_ASSERT(entity)

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		return Utils::Rigidbody2DTypeFromBox2DBody(body->GetType());
	}
	

	static bool Input_IsKeyDown(KeyCode keycode)
	{
		return Input::IsKeyPressed(keycode);
	}

	template<typename ... Component>
	static void RegisterComponent()
	{
		([]() 
			{
		std::string_view typeName =  typeid(Component).name();
		size_t pos = typeName.find_last_of(':');
		std::string_view structName = typeName.substr(pos + 1);
		std::string managedTypename = fmt::format("Kargono.{}", structName);

		MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());
			if (!managedType)
			{
				KG_CORE_ERROR("Could not find component type {}", managedTypename);
				return;
			}
		s_EntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<Component>(); };
			}(), ...);
	}

	template<typename ... Component>
	static void RegisterComponent(ComponentGroup<Component ...>)
	{
		RegisterComponent<Component ...>();
	}

	void ScriptGlue::RegisterComponents()
	{
		s_EntityHasComponentFuncs.clear();
		RegisterComponent (AllComponents{});
	}

	void ScriptGlue::RegisterFunctions()
    {
		//KG_ADD_INTERNAL_CALL(NativeLog);
		//KG_ADD_INTERNAL_CALL(NativeLog_Vector);
		//KG_ADD_INTERNAL_CALL(NativeLog_VectorDot);

		KG_ADD_INTERNAL_CALL(Entity_HasComponent);
		KG_ADD_INTERNAL_CALL(Entity_FindEntityByName);
		KG_ADD_INTERNAL_CALL(GetScriptInstance);

		KG_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		KG_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);

		KG_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
		KG_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
		KG_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetLinearVelocity);
		KG_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetLinearVelocity);
		KG_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetType);
		KG_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetType);

		KG_ADD_INTERNAL_CALL(Input_IsKeyDown);
    }
}
