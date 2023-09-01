#include "Kargono/kgpch.h"
#include "Kargono/Scripting/ScriptGlue.h"

#include "mono/jit/jit.h"
#include "mono/metadata/object.h"

namespace Kargono
{
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

    void ScriptGlue::RegisterFunctions()
    {
		KG_ADD_INTERNAL_CALL(NativeLog);
		KG_ADD_INTERNAL_CALL(NativeLog_Vector);
		KG_ADD_INTERNAL_CALL(NativeLog_VectorDot);
    }
}
