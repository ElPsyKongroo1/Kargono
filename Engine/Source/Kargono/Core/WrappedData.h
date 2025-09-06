#pragma once

#include "Kargono/Core/Buffer.h"
#include "Kargono/Utility/Conversions.h"
#include "Kargono/Math/Math.h"
#include "Modules/FileSystem/FileSystem.h"

#include <cstdint>
#include <functional>
#include <string>
#include <sstream>
#include <concepts>


namespace Kargono
{
	//==============================
	// Wrapped Variables
	//==============================
	enum class WrappedVarType
	{
		None = 0,
		Void,
		Bool,
		Float,
		Integer16,
		Integer32,
		Integer64,
		UInteger16,
		UInteger32,
		UInteger64,
		Vector2,
		Vector3,
		Vector4,
		IVector2,
		IVector3,
		IVector4,
		String,
		Entity
	};

	inline WrappedVarType s_AllWrappedVarTypes[] =
	{
		WrappedVarType::Integer16,
		WrappedVarType::Integer32,
		WrappedVarType::Integer64,
		WrappedVarType::UInteger16,
		WrappedVarType::UInteger32,
		WrappedVarType::UInteger64,
		WrappedVarType::Vector2,
		WrappedVarType::Vector3,
		WrappedVarType::Vector4,
		WrappedVarType::IVector2,
		WrappedVarType::IVector3,
		WrappedVarType::IVector4,
		WrappedVarType::String,
		WrappedVarType::Void,
		WrappedVarType::Bool,
		WrappedVarType::Float,
		WrappedVarType::Entity
	};

	class WrappedVariable
	{
	public:
		virtual ~WrappedVariable() = default;
	public:
		template<typename T>
		T& GetWrappedValue()
		{
			return *reinterpret_cast<T*>(GetValue());
		}
		template <typename T>
		void SetWrappedValue(T value)
		{
			SetValue(reinterpret_cast<void*>(&value));
		}

		virtual WrappedVarType Type() = 0;
		virtual void* GetValue() = 0;
		const virtual std::string GetValueAsString() = 0;
		virtual void SetValue(void* value) = 0;
	};

	class WrappedInteger32 : public WrappedVariable
	{
	public:
		WrappedInteger32() = default;
		WrappedInteger32(int32_t value) : m_Value{value} {}
	public:
		virtual WrappedVarType Type() override { return WrappedVarType::Integer32; }
	public:
		void* GetValue() override
		{
			return (void*)&m_Value;
		}

		const std::string GetValueAsString() override
		{
			return std::to_string(m_Value);
		}

		virtual void SetValue(void* value) override
		{
			m_Value = *(int32_t*)value;
		}
		int32_t m_Value{};
	};

	class WrappedUInteger16 : public WrappedVariable
	{
	public:
		WrappedUInteger16() = default;
		WrappedUInteger16(uint16_t value) : m_Value{ value } {}
	public:
		virtual WrappedVarType Type() override { return WrappedVarType::UInteger16; }
	public:
		void* GetValue() override
		{
			return (void*)&m_Value;
		}

		const std::string GetValueAsString() override
		{
			return std::to_string(m_Value);
		}

		virtual void SetValue(void* value) override
		{
			m_Value = *(uint16_t*)value;
		}
		uint16_t m_Value{};
	};

	class WrappedUInteger32 : public WrappedVariable
	{
	public:
		WrappedUInteger32() = default;
		WrappedUInteger32(uint32_t value) : m_Value{ value } {}
	public:
		virtual WrappedVarType Type() override { return WrappedVarType::UInteger32; }
	public:
		void* GetValue() override
		{
			return (void*)&m_Value;
		}

		const std::string GetValueAsString() override
		{
			return std::to_string(m_Value);
		}

		virtual void SetValue(void* value) override
		{
			m_Value = *(uint32_t*)value;
		}
		uint32_t m_Value{};
	};

	class WrappedUInteger64 : public WrappedVariable
	{
	public:
		WrappedUInteger64() = default;
		WrappedUInteger64(uint64_t value) : m_Value{ value } {}
	public:
		virtual WrappedVarType Type() override { return WrappedVarType::UInteger64; }
	public:
		void* GetValue() override
		{
			return (void*)&m_Value;
		}

		const std::string GetValueAsString() override
		{
			return std::to_string(m_Value);
		}

		virtual void SetValue(void* value) override
		{
			m_Value = *(uint64_t*)value;
		}
		uint64_t m_Value{};
	};

	class WrappedVector3 : public WrappedVariable
	{
	public:
		WrappedVector3() = default;
		WrappedVector3(Math::vec3 value) : m_Value{ value } {}
	public:
		virtual WrappedVarType Type() override { return WrappedVarType::Vector3; }
	public:
		void* GetValue() override
		{
			return (void*)&m_Value;
		}

		const std::string GetValueAsString() override
		{
			std::stringstream localStream {};
			localStream << m_Value.x << " " << m_Value.y << " " << m_Value.z;
			return localStream.str();
		}

		virtual void SetValue(void* value) override
		{
			m_Value = *(Math::vec3*)value;
		}
		Math::vec3 m_Value{};
	};

	class WrappedString : public WrappedVariable
	{
	public:
		WrappedString() = default;
		WrappedString(const std::string& value) : m_Value{ value } {}
	public:
		virtual WrappedVarType Type() override { return WrappedVarType::String; }
	public:
		void* GetValue() override
		{
			return (void*)&m_Value;
		}

		const std::string GetValueAsString() override
		{
			return m_Value;
		}

		virtual void SetValue(void* value) override
		{
			m_Value = *(std::string*)value;
		}
		std::string m_Value{};
	};

	class WrappedBool : public WrappedVariable
	{
	public:
		WrappedBool() = default;
		WrappedBool(bool value) : m_Value{ value } {}
	public:
		virtual WrappedVarType Type() override { return WrappedVarType::Bool; }
	public:
		void* GetValue() override
		{
			return (void*)&m_Value;
		}

		const std::string GetValueAsString() override
		{
			return m_Value ? "True" : "False";
		}

		virtual void SetValue(void* value) override
		{
			m_Value = *(bool*)value;
		}
		bool m_Value{};
	};

	class WrappedFloat : public WrappedVariable
	{
	public:
		WrappedFloat() = default;
		WrappedFloat(float value) : m_Value{ value } {}
	public:
		virtual WrappedVarType Type() override { return WrappedVarType::Float; }
	public:
		void* GetValue() override
		{
			return (void*)&m_Value;
		}

		const std::string GetValueAsString() override
		{
			return std::to_string(m_Value);
		}

		virtual void SetValue(void* value) override
		{
			m_Value = *(float*)value;
		}
		float m_Value{};
	};

	class WrappedEntity : public WrappedVariable
	{
	public:
		WrappedEntity() = default;
		WrappedEntity(uint64_t value) : m_Value{ value } {}
	public:
		virtual WrappedVarType Type() override { return WrappedVarType::Entity; }
	public:
		void* GetValue() override
		{
			return (void*)&m_Value;
		}

		const std::string GetValueAsString() override
		{
			return std::to_string(m_Value);
		}

		virtual void SetValue(void* value) override
		{
			m_Value = *(uint64_t*)value;
		}
		uint64_t m_Value{};
	};
	//==============================
	// Wrapped Functions
	//==============================
	enum class WrappedFuncType
	{
		None = 0,
		Void_None,
		Void_String,
		Void_Float,
		Void_UInt16,
		Void_UInt32,
		Void_UInt32UInt32,
		Void_Entity,
		Void_EntityFloat,
		Void_UInt32EntityEntityFloat,
		Void_Bool,
		Bool_None,
		Bool_Entity,
		Bool_EntityEntity,

		ArbitraryFunction
	};


	inline WrappedFuncType s_AllWrappedFuncs[] = 
	{
		WrappedFuncType::Void_None,
		WrappedFuncType::Void_String,
		WrappedFuncType::Void_Float,
		WrappedFuncType::Void_UInt16,
		WrappedFuncType::Void_UInt32,
		WrappedFuncType::Void_UInt32UInt32,
		WrappedFuncType::Void_Entity, 
		WrappedFuncType::Void_EntityFloat,
		WrappedFuncType::Void_UInt32EntityEntityFloat,
		WrappedFuncType::Void_Bool,
		WrappedFuncType::Bool_None,
		WrappedFuncType::Bool_Entity,
		WrappedFuncType::Bool_EntityEntity
	};

	typedef void (*void_none)();
	typedef void (*void_string)(std::string);
	typedef void (*void_float)(float);
	typedef void (*void_uint16)(uint16_t);
	typedef void (*void_uint32)(uint32_t);
	typedef void (*void_uint32uint32)(uint32_t, uint32_t);
	typedef void (*void_uint64)(uint64_t);
	typedef void (*void_uint64float)(uint64_t, float);
	typedef void (*void_uint32uint64uint64float)(uint32_t, uint64_t, uint64_t, float);
	typedef void (*void_bool)(bool);
	typedef bool (*bool_none)();
	typedef bool (*bool_uint64)(uint64_t);
	typedef bool (*bool_uint64uint64)(uint64_t, uint64_t);
	typedef bool (*bool_uint64uint16uint64)(uint64_t, uint16_t, uint64_t);

	class WrappedFunction
	{
	public:
		virtual ~WrappedFunction() = default;
	public:
		virtual WrappedFuncType Type() = 0;
	};

	class WrappedVoidNone : public WrappedFunction
	{
	public:
		WrappedVoidNone() = default;
		WrappedVoidNone(void_none value) : m_Value {value} {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_None; }
	public:
		void_none m_Value{};
	};

	class WrappedVoidString : public WrappedFunction
	{
	public:
		WrappedVoidString() = default;
		WrappedVoidString(void_string value) : m_Value{ value } {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_String; }
	public:
		void_string m_Value{};
	};

	class WrappedVoidFloat : public WrappedFunction
	{
	public:
		WrappedVoidFloat() = default;
		WrappedVoidFloat(void_float value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_Float; }
	public:
		void_float m_Value{};
	};

	class WrappedVoidUInt16 : public WrappedFunction
	{
	public:
		WrappedVoidUInt16() = default;
		WrappedVoidUInt16(void_uint16 value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_UInt16; }
	public:
		void_uint16 m_Value{};
	};

	class WrappedVoidUInt32 : public WrappedFunction
	{
	public:
		WrappedVoidUInt32() = default;
		WrappedVoidUInt32(void_uint32 value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_UInt32; }
	public:
		void_uint32 m_Value{};
	};

	class WrappedVoidUInt32UInt32 : public WrappedFunction
	{
	public:
		WrappedVoidUInt32UInt32() = default;
		WrappedVoidUInt32UInt32(void_uint32uint32 value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_UInt32UInt32; }
	public:
		void_uint32uint32 m_Value{};
	};

	class WrappedVoidEntity : public WrappedFunction
	{
	public:
		WrappedVoidEntity() = default;
		WrappedVoidEntity(void_uint64 value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_Entity; }
	public:
		void_uint64 m_Value{};
	};

	class WrappedVoidEntityFloat : public WrappedFunction
	{
	public:
		WrappedVoidEntityFloat() = default;
		WrappedVoidEntityFloat(void_uint64float value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_EntityFloat; }
	public:
		void_uint64float m_Value{};
	};

	class WrappedVoidUInt32EntityEntityFloat : public WrappedFunction
	{
	public:
		WrappedVoidUInt32EntityEntityFloat() = default;
		WrappedVoidUInt32EntityEntityFloat(void_uint32uint64uint64float value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_UInt32EntityEntityFloat; }
	public:
		void_uint32uint64uint64float m_Value{};
	};

	class WrappedVoidBool : public WrappedFunction
	{
	public:
		WrappedVoidBool() = default;
		WrappedVoidBool(void_bool value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_Bool; }
	public:
		void_bool m_Value{};
	};

	class WrappedBoolNone : public WrappedFunction
	{
	public:
		WrappedBoolNone() = default;
		WrappedBoolNone(bool_none value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Bool_None; }
	public:
		bool_none m_Value{};
	};

	class WrappedBoolEntity : public WrappedFunction
	{
	public:
		WrappedBoolEntity() = default;
		WrappedBoolEntity(bool_uint64 value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Bool_Entity; }
	public:
		bool_uint64 m_Value{};
	};

	class WrappedBoolEntityEntity : public WrappedFunction
	{
	public:
		WrappedBoolEntityEntity() = default;
		WrappedBoolEntityEntity(bool_uint64uint64 value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Bool_EntityEntity; }
	public:
		bool_uint64uint64 m_Value{};
	};

	namespace Utility
	{
		//==============================
		// Wrapped Variable Utility Functions
		//==============================
		inline const char* WrappedVarTypeToString(WrappedVarType type)
		{
			switch (type)
			{
			case WrappedVarType::Integer16: return "Integer16";
			case WrappedVarType::Integer32: return "Integer32";
			case WrappedVarType::Integer64: return "Integer64";
			case WrappedVarType::UInteger16: return "UInteger16";
			case WrappedVarType::UInteger32: return "UInteger32";
			case WrappedVarType::UInteger64: return "UInteger64";
			case WrappedVarType::Vector2: return "Vector2";
			case WrappedVarType::Vector3: return "Vector3";
			case WrappedVarType::Vector4: return "Vector4";
			case WrappedVarType::IVector2: return "IVector2";
			case WrappedVarType::IVector3: return "IVector3";
			case WrappedVarType::IVector4: return "IVector4";
			case WrappedVarType::String: return "String";
			case WrappedVarType::Void: return "Void";
			case WrappedVarType::Bool: return "Bool";
			case WrappedVarType::Float: return "Float";
			case WrappedVarType::Entity: return "Entity";
			case WrappedVarType::None: return "None";
			}
			KG_ERROR("Unknown Type of WrappedVariableType.");
			return "";
		}

		inline const char* WrappedVarTypeToCategory(WrappedVarType type)
		{
			switch (type)
			{
			case WrappedVarType::Integer16:
			case WrappedVarType::Integer32:
			case WrappedVarType::Integer64:
			case WrappedVarType::UInteger16:
			case WrappedVarType::UInteger32:
			case WrappedVarType::UInteger64:
			case WrappedVarType::IVector2:
			case WrappedVarType::IVector3:
			case WrappedVarType::IVector4:
				return "Integer Type";
			case WrappedVarType::Float:
			case WrappedVarType::Vector2:
			case WrappedVarType::Vector3:
			case WrappedVarType::Vector4:
				return "Float Type";
			case WrappedVarType::String:
				return "String Type";
			case WrappedVarType::Bool:
				return "Bool Type";
			case WrappedVarType::Entity:
				return "Other Type";
			case WrappedVarType::Void:
			case WrappedVarType::None:
				return "Clear Type";
			}
			KG_ERROR("Unknown Type of WrappedVariableType.");
			return "";
		}

		inline Ref<WrappedVariable> WrappedVarTypeToWrappedVariable(WrappedVarType type)
		{
			switch (type)
			{
			case WrappedVarType::Integer32: return CreateRef<WrappedInteger32>();
			case WrappedVarType::UInteger16: return CreateRef<WrappedUInteger16>();
			case WrappedVarType::UInteger32: return CreateRef<WrappedUInteger32>();
			case WrappedVarType::UInteger64: return CreateRef<WrappedUInteger64>();
			case WrappedVarType::Vector3: return CreateRef<WrappedVector3>();
			case WrappedVarType::String: return CreateRef<WrappedString>();
			case WrappedVarType::Bool: return CreateRef<WrappedBool>();
			case WrappedVarType::Float: return CreateRef<WrappedFloat>();
			case WrappedVarType::Entity: return CreateRef<WrappedEntity>();
			case WrappedVarType::Void:
			case WrappedVarType::None:
				return nullptr;
			}
			KG_ERROR("Unknown Type of WrappedVariableType.");
			return nullptr;
		}

		inline const char* WrappedVarTypeToCPPString(WrappedVarType type)
		{
			switch (type)
			{
			case WrappedVarType::Integer16: return "int16_t";
			case WrappedVarType::Integer32: return "int32_t";
			case WrappedVarType::Integer64: return "int64_t";
			case WrappedVarType::UInteger16: return "uint16_t";
			case WrappedVarType::UInteger32: return "uint32_t";
			case WrappedVarType::UInteger64: return "uint64_t";
			case WrappedVarType::Vector2: return "Math::vec2";
			case WrappedVarType::Vector3: return "Math::vec3";
			case WrappedVarType::Vector4: return "Math::vec4";
			case WrappedVarType::IVector2: return "Math::ivec2";
			case WrappedVarType::IVector3: return "Math::ivec3";
			case WrappedVarType::IVector4: return "Math::ivec4";
			case WrappedVarType::String: return "std::string";
			case WrappedVarType::Void: return "void";
			case WrappedVarType::Bool: return "bool";
			case WrappedVarType::Float: return "float";
			case WrappedVarType::Entity: return "uint64_t";
			case WrappedVarType::None: return "None";
			}
			KG_ERROR("Unknown Type of WrappedVariableType.");
			return "";
		}

		inline const char* WrappedVarTypeToCPPParameter(WrappedVarType type)
		{
			switch (type)
			{
			case WrappedVarType::Integer16: return "int16_t";
			case WrappedVarType::Integer32: return "int32_t";
			case WrappedVarType::Integer64: return "int64_t";
			case WrappedVarType::UInteger16: return "uint16_t";
			case WrappedVarType::UInteger32: return "uint32_t";
			case WrappedVarType::UInteger64: return "uint64_t";
			case WrappedVarType::Vector2: return "Math::vec2";
			case WrappedVarType::Vector3: return "Math::vec3";
			case WrappedVarType::Vector4: return "Math::vec4";
			case WrappedVarType::IVector2: return "Math::ivec2";
			case WrappedVarType::IVector3: return "Math::ivec3";
			case WrappedVarType::IVector4: return "Math::ivec4";
			case WrappedVarType::String: return "const std::string&";
			case WrappedVarType::Void: return "void";
			case WrappedVarType::Bool: return "bool";
			case WrappedVarType::Float: return "float";
			case WrappedVarType::Entity: return "uint64_t";
			case WrappedVarType::None: return "None";
			}
			KG_ERROR("Unknown Type of WrappedVariableType.");
			return "";
		}

		inline size_t WrappedVarTypeToDataSizeBytes(WrappedVarType type)
		{
			switch (type)
			{
			case WrappedVarType::Integer16: return sizeof(int16_t);
			case WrappedVarType::Integer32: return sizeof(int32_t);
			case WrappedVarType::Integer64: return sizeof(int64_t);
			case WrappedVarType::UInteger16: return sizeof(uint16_t);
			case WrappedVarType::UInteger32: return sizeof(uint32_t);
			case WrappedVarType::UInteger64: return sizeof(uint64_t);
			case WrappedVarType::Vector2: return sizeof(Math::vec2);
			case WrappedVarType::Vector3: return sizeof(Math::vec3);
			case WrappedVarType::Vector4: return sizeof(Math::vec4);
			case WrappedVarType::IVector2: return sizeof(Math::ivec2);
			case WrappedVarType::IVector3: return sizeof(Math::ivec3);
			case WrappedVarType::IVector4: return sizeof(Math::ivec4);
			case WrappedVarType::String: return sizeof(std::string);
			case WrappedVarType::Bool: return sizeof(bool);
			case WrappedVarType::Float: return sizeof(float);
			case WrappedVarType::Entity: return sizeof(uint64_t);
			case WrappedVarType::Void:
			case WrappedVarType::None:
			default:
				KG_ERROR("Invalid wrapped variable type presented when trying to get it's data size in bytes");
				return std::numeric_limits<size_t>::max();
			}
		}

		inline const char* WrappedVarTypeToKGScript(WrappedVarType type)
		{
			switch (type)
			{
			case WrappedVarType::Integer16: return "int16";
			case WrappedVarType::Integer32: return "int32";
			case WrappedVarType::Integer64: return "int64";
			case WrappedVarType::UInteger16: return "uint16";
			case WrappedVarType::UInteger32: return "uint32";
			case WrappedVarType::UInteger64: return "uint64";
			case WrappedVarType::Vector2: return "vector2";
			case WrappedVarType::Vector3: return "vector3";
			case WrappedVarType::Vector4: return "vector4";
			case WrappedVarType::IVector2: return "ivector2";
			case WrappedVarType::IVector3: return "ivector3";
			case WrappedVarType::IVector4: return "ivector4";
			case WrappedVarType::String: return "string";
			case WrappedVarType::Void: return "void";
			case WrappedVarType::Bool: return "bool";
			case WrappedVarType::Float: return "float";
			case WrappedVarType::Entity: return "entity";
			case WrappedVarType::None: return "None";
			}
			KG_ERROR("Unknown Type of WrappedVariableType.");
			return "";
		}

		inline WrappedVarType KGScriptToWrappedVarType(const std::string& scriptType)
		{

			if (scriptType == "int16") { return WrappedVarType::Integer16; }
			if (scriptType == "int32") { return WrappedVarType::Integer32; }
			if (scriptType == "int64") { return WrappedVarType::Integer64; }
			if (scriptType == "uint16") { return WrappedVarType::UInteger16; }
			if (scriptType == "uint32") { return WrappedVarType::UInteger32; }
			if (scriptType == "uint64") { return WrappedVarType::UInteger64; }
			if (scriptType == "vector2") { return WrappedVarType::Vector2; }
			if (scriptType == "vector3") { return WrappedVarType::Vector3; }
			if (scriptType == "vector4") { return WrappedVarType::Vector4; }
			if (scriptType == "ivector2") { return WrappedVarType::IVector2; }
			if (scriptType == "ivector3") { return WrappedVarType::IVector3; }
			if (scriptType == "ivector4") { return WrappedVarType::IVector4; }
			if (scriptType == "string") { return WrappedVarType::String; }
			if (scriptType == "float") { return WrappedVarType::Float; }
			if (scriptType == "bool") { return WrappedVarType::Bool; }
			if (scriptType == "entity") { return WrappedVarType::Entity; }
			if (scriptType == "void") { return WrappedVarType::Void; }

			KG_ERROR("Unknown Type of WrappedVariableType String.");
			return WrappedVarType::None;
		}

		inline WrappedVarType StringToWrappedVarType(std::string type)
		{
			if (type == "Integer16") { return WrappedVarType::Integer16; }
			if (type == "Integer32") { return WrappedVarType::Integer32; }
			if (type == "Integer64") { return WrappedVarType::Integer64; }
			if (type == "UInteger16") { return WrappedVarType::UInteger16; }
			if (type == "UInteger32") { return WrappedVarType::UInteger32; }
			if (type == "UInteger64") { return WrappedVarType::UInteger64; }
			if (type == "Vector2") { return WrappedVarType::Vector2; }
			if (type == "Vector3") { return WrappedVarType::Vector3; }
			if (type == "Vector4") { return WrappedVarType::Vector4; }
			if (type == "IVector2") { return WrappedVarType::IVector2; }
			if (type == "IVector3") { return WrappedVarType::IVector3; }
			if (type == "IVector4") { return WrappedVarType::IVector4; }
			if (type == "String") { return WrappedVarType::String; }
			if (type == "Void") { return WrappedVarType::Void; }
			if (type == "Bool") { return WrappedVarType::Bool; }
			if (type == "Float") { return WrappedVarType::Float; }
			if (type == "Entity") { return WrappedVarType::Entity; }
			if (type == "None") { return WrappedVarType::None; }

			KG_ERROR("Unknown Type of WrappedVariableType String.");
			return WrappedVarType::None;
		}

		inline void InitializeDataForWrappedVarBuffer(WrappedVarType type, void* buffer)
		{
			switch (type)
			{
			case WrappedVarType::Integer16:
				*(int16_t*)buffer = 0;
				return;
			case WrappedVarType::Integer32:
				*(int32_t*)buffer = 0;
				return;
			case WrappedVarType::Integer64:
				*(int64_t*)buffer = 0;
				return;
			case WrappedVarType::UInteger16:
				*(uint16_t*)buffer = 0;
				return;
			case WrappedVarType::UInteger32:
				*(uint32_t*)buffer = 0;
				return;
			case WrappedVarType::UInteger64:
			case WrappedVarType::Entity:
				*(uint64_t*)buffer = 0;
				return;
			case WrappedVarType::Vector2:
				*(Math::vec2*)buffer = { 0.0f, 0.0f };
				return;
			case WrappedVarType::Vector3:
				*(Math::vec3*)buffer = { 0.0f, 0.0f, 0.0f };
				return;
			case WrappedVarType::Vector4:
				*(Math::vec4*)buffer = { 0.0f, 0.0f, 0.0f, 0.0f };
				return;
			case WrappedVarType::IVector2:
				*(Math::ivec2*)buffer = { 0, 0 };
				return;
			case WrappedVarType::IVector3:
				*(Math::ivec3*)buffer = { 0, 0, 0 };
				return;
			case WrappedVarType::IVector4:
				*(Math::ivec4*)buffer = { 0, 0, 0, 0 };
				return;
			case WrappedVarType::String:
				*(std::string*)buffer = std::string();
				return;
			case WrappedVarType::Bool:
				*(bool*)buffer = false;
				return;
			case WrappedVarType::Float:
				*(float*)buffer = 0.0f;
				return;
			case WrappedVarType::Void:
			case WrappedVarType::None:
				KG_ERROR("Invalid type provided when setting data of wrapped variable type");
			}
			KG_ERROR("Unknown Type of WrappedVariableType.");
		}

		inline void TransferDataForWrappedVarBuffer(WrappedVarType type, void* source, void* destination)
		{
			switch (type)
			{
			case WrappedVarType::Integer16:
				*(int16_t*)destination = *(int16_t*)source;
				return;
			case WrappedVarType::Integer32: 
				*(int32_t*)destination = *(int32_t*)source;
				return;
			case WrappedVarType::Integer64:
				*(int64_t*)destination = *(int64_t*)source;
				return;
			case WrappedVarType::UInteger16:
				*(uint16_t*)destination = *(uint16_t*)source;
				return;
			case WrappedVarType::UInteger32: 
				*(uint32_t*)destination = *(uint32_t*)source;
				return;
			case WrappedVarType::UInteger64:
				*(uint64_t*)destination = *(uint64_t*)source;
				return;
			case WrappedVarType::Entity:
				*(uint64_t*)destination = *(uint64_t*)source;
				return;
			case WrappedVarType::Vector2:
				*(Math::vec2*)destination = *(Math::vec2*)source;
				return;
			case WrappedVarType::Vector3:
				*(Math::vec3*)destination = *(Math::vec3*)source;
				return;
			case WrappedVarType::Vector4:
				*(Math::vec4*)destination = *(Math::vec4*)source;
				return;
			case WrappedVarType::IVector2:
				*(Math::ivec2*)destination = *(Math::ivec2*)source;
				return;
			case WrappedVarType::IVector3:
				*(Math::ivec3*)destination = *(Math::ivec3*)source;
				return;
			case WrappedVarType::IVector4:
				*(Math::ivec4*)destination = *(Math::ivec4*)source;
				return;
			case WrappedVarType::String:
				*(std::string*)destination = *(std::string*)source;
				return;
			case WrappedVarType::Bool:
				*(bool*)destination = *(bool*)source;
				return;
			case WrappedVarType::Float:
				*(float*)destination = *(float*)source;
				return;
			case WrappedVarType::Void:
			case WrappedVarType::None:
				KG_ERROR("Invalid type provided when setting data of wrapped variable type");
			}
			KG_ERROR("Unknown Type of WrappedVariableType.");
		}

		inline bool FillBufferWithWrappedVarString(Ref<WrappedVariable> variable, Buffer buffer)
		{
			buffer.SetDataToByte(0);
			const std::string variableAsString { variable->GetValueAsString() };
			memcpy(buffer.Data, variableAsString.data(), variableAsString.size());
			return true;
		}

		//==============================
		// Wrapped Function Utility Functions
		//==============================
		inline const char* WrappedFuncTypeToString(WrappedFuncType type)
		{
			switch (type)
			{
			case WrappedFuncType::Void_None: return "Void_None";
			case WrappedFuncType::Void_String: return "Void_String";
			case WrappedFuncType::Void_Float: return "Void_Float";
			case WrappedFuncType::Void_UInt16: return "Void_UInt16";
			case WrappedFuncType::Void_UInt32: return "Void_UInt32";
			case WrappedFuncType::Void_UInt32UInt32: return "Void_UInt32UInt32";
			case WrappedFuncType::Void_Entity: return "Void_Entity";
			case WrappedFuncType::Void_Bool: return "Void_Bool";
			case WrappedFuncType::Void_EntityFloat: return "Void_EntityFloat";
			case WrappedFuncType::Void_UInt32EntityEntityFloat: return "Void_UInt32EntityEntityFloat";
			case WrappedFuncType::Bool_None: return "Bool_None";
			case WrappedFuncType::Bool_Entity: return "Bool_Entity";
			case WrappedFuncType::Bool_EntityEntity: return "Bool_EntityEntity";
			case WrappedFuncType::ArbitraryFunction: return "ArbitraryFunction";
			case WrappedFuncType::None: return "None";
			}
			KG_ERROR("Unknown Type of WrappedType.");
			return "";
		}

		inline WrappedFuncType StringToWrappedFuncType(std::string_view type)
		{
			if (type == "Void_None") { return WrappedFuncType::Void_None; }
			if (type == "Void_String") { return WrappedFuncType::Void_String; }
			if (type == "Void_Float") { return WrappedFuncType::Void_Float; }
			if (type == "Void_UInt16") { return WrappedFuncType::Void_UInt16; }
			if (type == "Void_UInt32") { return WrappedFuncType::Void_UInt32; }
			if (type == "Void_UInt32UInt32") { return WrappedFuncType::Void_UInt32UInt32; }
			if (type == "Void_Entity") { return WrappedFuncType::Void_Entity; }
			if (type == "Void_Bool") { return WrappedFuncType::Void_Bool; }
			if (type == "Void_EntityFloat") { return WrappedFuncType::Void_EntityFloat; }
			if (type == "Void_UInt32EntityEntityFloat") { return WrappedFuncType::Void_UInt32EntityEntityFloat; }
			if (type == "Bool_None") { return WrappedFuncType::Bool_None; }
			if (type == "Bool_Entity") { return WrappedFuncType::Bool_Entity; }
			if (type == "Bool_EntityEntity") { return WrappedFuncType::Bool_EntityEntity; }
			if (type == "ArbitraryFunction") { return WrappedFuncType::ArbitraryFunction; }
			if (type == "None") { return WrappedFuncType::None; }

			KG_ERROR("Unknown Type of WrappedFuncType String.");
			return WrappedFuncType::None;
		}

		template<typename T>
		concept DerivedFromWrappedFunction = std::is_base_of_v<WrappedFunction, T>;

		template<DerivedFromWrappedFunction t_WrappedFuncType, typename... Args>
		auto CallWrapped(Ref<WrappedFunction> wrappedFunction, Args&&... args)
		{
			return static_cast<t_WrappedFuncType*>(wrappedFunction.get())->m_Value(std::forward<Args>(args)...);
		}

		inline WrappedVarType WrappedFuncTypeToReturnType(WrappedFuncType type)
		{
			switch (type)
			{
			case WrappedFuncType::Void_None:
			case WrappedFuncType::Void_String:
			case WrappedFuncType::Void_Float:
			case WrappedFuncType::Void_UInt16:
			case WrappedFuncType::Void_UInt32:
			case WrappedFuncType::Void_UInt32UInt32:
			case WrappedFuncType::Void_Entity:
			case WrappedFuncType::Void_Bool:
			case WrappedFuncType::Void_EntityFloat:
			case WrappedFuncType::Void_UInt32EntityEntityFloat:
				return WrappedVarType::Void;
			case WrappedFuncType::Bool_None:
			case WrappedFuncType::Bool_Entity:
			case WrappedFuncType::Bool_EntityEntity:
				return WrappedVarType::Bool;
			case WrappedFuncType::ArbitraryFunction:
				return WrappedVarType::None;
			case WrappedFuncType::None:
			{
				KG_ERROR("None type provided to return type utility function");
				return WrappedVarType::None;
			}
			default:
				KG_ERROR("Unknown Type of WrappedType.");
				return WrappedVarType::None;
			}
		}

		inline std::vector<WrappedVarType> WrappedFuncTypeToParameterTypes(WrappedFuncType type)
		{
			switch (type)
			{
			case WrappedFuncType::Void_None:
			case WrappedFuncType::Bool_None:
				return {};
			case WrappedFuncType::Void_String:
				return { WrappedVarType::String };
			case WrappedFuncType::Void_Float:
				return { WrappedVarType::Float };
			case WrappedFuncType::Void_UInt16:
				return { WrappedVarType::UInteger16 };
			case WrappedFuncType::Void_Bool:
				return { WrappedVarType::Bool };
			case WrappedFuncType::Void_UInt32:
				return { WrappedVarType::UInteger32 };
			case WrappedFuncType::Void_UInt32UInt32:
				return { WrappedVarType::UInteger32, WrappedVarType::UInteger32 };
			case WrappedFuncType::Void_EntityFloat:
				return { WrappedVarType::Entity, WrappedVarType::Float };
			case WrappedFuncType::Void_UInt32EntityEntityFloat:
				return { WrappedVarType::UInteger32, WrappedVarType::Entity, WrappedVarType::Entity, WrappedVarType::Float };
			case WrappedFuncType::Void_Entity:
			case WrappedFuncType::Bool_Entity:
				return { WrappedVarType::Entity };
			case WrappedFuncType::Bool_EntityEntity:
				return { WrappedVarType::Entity, WrappedVarType::Entity };
			case WrappedFuncType::ArbitraryFunction:
				return {};
			case WrappedFuncType::None:
			{
				KG_ERROR("None type provided to parameter list utility function");
				return {};
			}
			default:
				KG_ERROR("Unknown Type of WrappedFuncType.")
				return {};
			}
		}

		inline WrappedFuncType ExplicitSignatureToWrappedFuncType(WrappedVarType queryReturnType, const std::vector<WrappedVarType>& queryParameterList)
		{
			// Check every predefined func type
			for (WrappedFuncType funcType : s_AllWrappedFuncs)
			{
				// Check return type
				if (queryReturnType != WrappedFuncTypeToReturnType(funcType))
				{
					continue;
				}

				// Check parameter list 
				std::vector<WrappedVarType> funcParameterList = WrappedFuncTypeToParameterTypes(funcType);
				if (queryParameterList != funcParameterList)
				{
					continue;
				}

				// Return indicated return type is successful
				return funcType;

			}

			// If no wrapped func type is found, return null type
			return WrappedFuncType::None;
		}
	}
	
}
