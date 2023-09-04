#include "Kargono/kgpch.h"
#include "Kargono/Scripting/ScriptGlue.h"

#include "Kargono/Core/Input.h"
#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Scripting/ScriptEngine.h"

#include "Kargono/Scene/Entity.h"
#include "Kargono/Scene/Scene.h"

#include "mono/jit/jit.h"
#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

namespace Kargono
{
	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

#define KG_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Kargono.InternalCalls::" #Name, Name)

	static void NativeLog(MonoString* string, int parameter)
	{
		char* cStr = mono_string_to_utf8(string);
		std::string str(cStr);
		mono_free(cStr);
		std::cout << str << ", " << parameter << '\n';
	}

	static void NativeLog_Vector(glm::vec3* parameter, glm::vec3* outResult)
	{
		KG_CORE_WARN("Value: {0}", *parameter);

		*outResult = glm::normalize(*parameter);
	}

	static float NativeLog_VectorDot(glm::vec3* parameter)
	{
		KG_CORE_WARN("Value: {0}", *parameter);
		return glm::dot(*parameter, *parameter);
	}

	

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

	static void Entity_GetTranslation(UUID entityID, glm::vec3* outTranslation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);

		*outTranslation = entity.GetComponent<TransformComponent>().Translation;

	}
	
	static void Entity_SetTranslation(UUID entityID, glm::vec3* translation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);

		entity.GetComponent<TransformComponent>().Translation = *translation;
	}

	static bool Input_IsKeyDown(KeyCode keycode)
	{
		return Input::IsKeyPressed(keycode);
	}

	/*template<typename ... Component>
	static void RegisterComponent()
	{
		([&]() {}(), ...);
		std::string_view typeName =  typeid(Component).name();
		size_t pos = typeName.find_last_of(':');
		std::string_view structName = typeName.substr(pos + 1);
		std::string managedTypename = fmt::format("Kargono.{}", structName);

		MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());
		KG_CORE_ASSERT(managedType)
		s_EntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<Component>(); };
	}

	template<typename ... Component>
	static void RegisterComponent(ComponentGroup<Component ...>)
	{
		
	}*/

	void ScriptGlue::RegisterComponents()
	{
		//RegisterComponent < AllComponents{} > ();
	}

	void ScriptGlue::RegisterFunctions()
    {
		KG_ADD_INTERNAL_CALL(NativeLog);
		KG_ADD_INTERNAL_CALL(NativeLog_Vector);
		KG_ADD_INTERNAL_CALL(NativeLog_VectorDot);

		KG_ADD_INTERNAL_CALL(Entity_HasComponent);
		KG_ADD_INTERNAL_CALL(Entity_GetTranslation);
		KG_ADD_INTERNAL_CALL(Entity_SetTranslation);

		KG_ADD_INTERNAL_CALL(Input_IsKeyDown);
    }
}
