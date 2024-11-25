#pragma once

#include "Kargono/Core/Buffer.h"
#include "Kargono/Utility/Conversions.h"
#include "Kargono/Math/Math.h"
#include "Kargono/Utility/FileSystem.h"

#include <cstdint>
#include <functional>
#include <string>
#include <sstream>


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
		Integer32,
		UInteger16,
		UInteger32,
		UInteger64,
		Vector3,
		String,
		Entity
	};

	inline WrappedVarType s_AllWrappedVarTypes[] =
	{
		WrappedVarType::Integer32,
		WrappedVarType::UInteger16,
		WrappedVarType::UInteger32,
		WrappedVarType::UInteger64,
		WrappedVarType::Vector3,
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
		Void_Entity,
		Void_EntityFloat,
		Void_UInt32EntityEntityFloat,
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
		WrappedFuncType::Void_Entity, 
		WrappedFuncType::Void_EntityFloat,
		WrappedFuncType::Void_UInt32EntityEntityFloat,
		WrappedFuncType::Bool_None,
		WrappedFuncType::Bool_Entity,
		WrappedFuncType::Bool_EntityEntity
	};

	typedef void (*void_none)();
	typedef void (*void_string)(std::string);
	typedef void (*void_float)(float);
	typedef void (*void_uint16)(uint16_t);
	typedef void (*void_uint32)(uint32_t);
	typedef void (*void_uint64)(uint64_t);
	typedef void (*void_uint64float)(uint64_t, float);
	typedef void (*void_uint32uint64uint64float)(uint32_t, uint64_t, uint64_t, float);
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
		WrappedVoidNone(std::function<void()> value) : m_Value {value} {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_None; }
	public:
		std::function<void()> m_Value{};
	};

	class WrappedVoidString : public WrappedFunction
	{
	public:
		WrappedVoidString() = default;
		WrappedVoidString(std::function<void(const std::string&)> value) : m_Value{ value } {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_String; }
	public:
		std::function<void(const std::string&)> m_Value{};
	};

	class WrappedVoidFloat : public WrappedFunction
	{
	public:
		WrappedVoidFloat() = default;
		WrappedVoidFloat(std::function<void(float)> value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_Float; }
	public:
		std::function<void(float)> m_Value{};
	};

	class WrappedVoidUInt16 : public WrappedFunction
	{
	public:
		WrappedVoidUInt16() = default;
		WrappedVoidUInt16(std::function<void(uint16_t)> value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_UInt16; }
	public:
		std::function<void(uint16_t)> m_Value{};
	};

	class WrappedVoidUInt32 : public WrappedFunction
	{
	public:
		WrappedVoidUInt32() = default;
		WrappedVoidUInt32(std::function<void(uint32_t)> value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_UInt32; }
	public:
		std::function<void(uint32_t)> m_Value{};
	};

	class WrappedVoidEntity : public WrappedFunction
	{
	public:
		WrappedVoidEntity() = default;
		WrappedVoidEntity(std::function<void(uint64_t)> value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_Entity; }
	public:
		std::function<void(uint64_t)> m_Value{};
	};

	class WrappedVoidEntityFloat : public WrappedFunction
	{
	public:
		WrappedVoidEntityFloat() = default;
		WrappedVoidEntityFloat(std::function<void(uint64_t, float)> value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_EntityFloat; }
	public:
		std::function<void(uint64_t, float)> m_Value{};
	};

	class WrappedVoidUInt32EntityEntityFloat : public WrappedFunction
	{
	public:
		WrappedVoidUInt32EntityEntityFloat() = default;
		WrappedVoidUInt32EntityEntityFloat(std::function<void(uint32_t, uint64_t, uint64_t, float)> value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_UInt32EntityEntityFloat; }
	public:
		std::function<void(uint32_t, uint64_t, uint64_t, float)> m_Value{};
	};

	class WrappedBoolNone : public WrappedFunction
	{
	public:
		WrappedBoolNone() = default;
		WrappedBoolNone(std::function<bool()> value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Bool_None; }
	public:
		std::function<bool()> m_Value{};
	};

	class WrappedBoolEntity : public WrappedFunction
	{
	public:
		WrappedBoolEntity() = default;
		WrappedBoolEntity(std::function<bool(uint64_t)> value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Bool_Entity; }
	public:
		std::function<bool(uint64_t)> m_Value{};
	};

	class WrappedBoolEntityEntity : public WrappedFunction
	{
	public:
		WrappedBoolEntityEntity() = default;
		WrappedBoolEntityEntity(std::function<bool(uint64_t, uint64_t)> value) : m_Value(value) {}
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Bool_EntityEntity; }
	public:
		std::function<bool(uint64_t, uint64_t)> m_Value{};
	};

	namespace Utility
	{
		//==============================
		// Wrapped Variable Utility Functions
		//==============================
		inline std::string WrappedVarTypeToString(WrappedVarType type)
		{
			switch (type)
			{
			case WrappedVarType::Integer32: return "Integer32";
			case WrappedVarType::UInteger16: return "UInteger16";
			case WrappedVarType::UInteger32: return "UInteger32";
			case WrappedVarType::UInteger64: return "UInteger64";
			case WrappedVarType::Vector3: return "Vector3";
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

		inline std::string WrappedVarTypeToCPPString(WrappedVarType type)
		{
			switch (type)
			{
			case WrappedVarType::Integer32: return "int32_t";
			case WrappedVarType::UInteger16: return "uint16_t";
			case WrappedVarType::UInteger32: return "uint32_t";
			case WrappedVarType::UInteger64: return "uint64_t";
			case WrappedVarType::Vector3: return "Math::vec3";
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

		inline std::string WrappedVarTypeToCPPParameter(WrappedVarType type)
		{
			switch (type)
			{
			case WrappedVarType::Integer32: return "int32_t";
			case WrappedVarType::UInteger16: return "uint16_t";
			case WrappedVarType::UInteger32: return "uint32_t";
			case WrappedVarType::UInteger64: return "uint64_t";
			case WrappedVarType::Vector3: return "Math::vec3";
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
			case WrappedVarType::Integer32: return sizeof(int32_t);
			case WrappedVarType::UInteger16: return sizeof(uint16_t);
			case WrappedVarType::UInteger32: return sizeof(uint32_t);
			case WrappedVarType::UInteger64: return sizeof(uint64_t);
			case WrappedVarType::Vector3: return sizeof(Math::vec3);
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
			KG_ERROR("Invalid wrapped variable type presented when trying to get it's data size in bytes");
			return std::numeric_limits<size_t>::max();
		}

		inline std::string WrappedVarTypeToKGScript(WrappedVarType type)
		{
			switch (type)
			{
			case WrappedVarType::Integer32: return "int32";
			case WrappedVarType::UInteger16: return "uint16";
			case WrappedVarType::UInteger32: return "uint32";
			case WrappedVarType::UInteger64: return "uint64";
			case WrappedVarType::Vector3: return "vector3";
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

			if (scriptType == "int32") { return WrappedVarType::Integer32; }
			if (scriptType == "uint16") { return WrappedVarType::UInteger16; }
			if (scriptType == "uint32") { return WrappedVarType::UInteger32; }
			if (scriptType == "uint64") { return WrappedVarType::UInteger64; }
			if (scriptType == "vector3") { return WrappedVarType::Vector3; }
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
			if (type == "Integer32") { return WrappedVarType::Integer32; }
			if (type == "UInteger16") { return WrappedVarType::UInteger16; }
			if (type == "UInteger32") { return WrappedVarType::UInteger32; }
			if (type == "UInteger64") { return WrappedVarType::UInteger64; }
			if (type == "Vector3") { return WrappedVarType::Vector3; }
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
			case WrappedVarType::Integer32:
			case WrappedVarType::UInteger16:
			case WrappedVarType::UInteger32:
			case WrappedVarType::UInteger64:
			case WrappedVarType::Entity:
				*(int32_t*)buffer = 0;
				return;
			case WrappedVarType::Vector3:
				*(Math::vec3*)buffer = { 0.0f, 0.0f, 0.0f };
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
			case WrappedVarType::Integer32: 
				*(int32_t*)destination = *(int32_t*)source;
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
			case WrappedVarType::Vector3:
				*(Math::vec3*)destination = *(Math::vec3*)source;
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

		inline bool FillWrappedVarWithStringBuffer(Ref<WrappedVariable> variable, Buffer buffer)
		{
			// Func Resources
			static std::string valueString {};

			bool success{ false };
			switch (variable->Type())
			{
				case WrappedVarType::Integer32:
				{
					success = Conversions::CharBufferToVariable(buffer,
						variable->GetWrappedValue<int32_t>());
					break;
				}
				case WrappedVarType::UInteger16:
				{
					success = Conversions::CharBufferToVariable(buffer,
						variable->GetWrappedValue<uint16_t>());
					break;
				}
				case WrappedVarType::UInteger32:
				{
					success = Conversions::CharBufferToVariable(buffer,
						variable->GetWrappedValue<uint32_t>());
					break;
				}
				case WrappedVarType::UInteger64:
				{
					success = Conversions::CharBufferToVariable(buffer,
						variable->GetWrappedValue<uint64_t>());
					break;
				}
				case WrappedVarType::Entity:
				{
					success = Conversions::CharBufferToVariable(buffer,
						variable->GetWrappedValue<uint64_t>());
					break;
				}
				case WrappedVarType::Vector3:
				{
					// TODO Figure out what happens to the vector
					success = Conversions::CharBufferToVariable(buffer,
						variable->GetWrappedValue<Math::vec3>());
					break;
				}
				case WrappedVarType::String:
				{
					valueString = buffer.GetString();
					variable->SetWrappedValue<std::string>(valueString);
					valueString = {};
					success = true;
					break;
				}
				case WrappedVarType::Bool:
				{
					valueString = buffer.GetString();
					variable->SetWrappedValue<bool>(Utility::Conversions::StringToBool(valueString));
					success = true;
					break;
				}
				case WrappedVarType::Float:
				{
					success = Conversions::CharBufferToVariable(buffer,
						variable->GetWrappedValue<float>());
					break;
				}
				case WrappedVarType::Void:
				case WrappedVarType::None:
				{
					KG_WARN("Invalid WrappedVarType provided to FillStringBuffer Function");
					return false;
				}
			}
			return success;
		}
		//==============================
		// Wrapped Function Utility Functions
		//==============================
		inline std::string WrappedFuncTypeToString(WrappedFuncType type)
		{
			switch (type)
			{
			case WrappedFuncType::Void_None: return "Void_None";
			case WrappedFuncType::Void_String: return "Void_String";
			case WrappedFuncType::Void_Float: return "Void_Float";
			case WrappedFuncType::Void_UInt16: return "Void_UInt16";
			case WrappedFuncType::Void_UInt32: return "Void_UInt32";
			case WrappedFuncType::Void_Entity: return "Void_Entity";
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

		inline WrappedFuncType StringToWrappedFuncType(std::string type)
		{
			if (type == "Void_None") { return WrappedFuncType::Void_None; }
			if (type == "Void_String") { return WrappedFuncType::Void_String; }
			if (type == "Void_Float") { return WrappedFuncType::Void_Float; }
			if (type == "Void_UInt16") { return WrappedFuncType::Void_UInt16; }
			if (type == "Void_UInt32") { return WrappedFuncType::Void_UInt32; }
			if (type == "Void_Entity") { return WrappedFuncType::Void_Entity; }
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

		inline void CallWrappedVoidNone(Ref<WrappedFunction> wrappedFunction)
		{
			KG_ASSERT(wrappedFunction->Type() == WrappedFuncType::Void_None, "Invalid wrapped function type provided");
			((WrappedVoidNone*)wrappedFunction.get())->m_Value();
		}

		inline void CallWrappedVoidString(Ref<WrappedFunction> wrappedFunction, const std::string& argumentOne)
		{
			KG_ASSERT(wrappedFunction->Type() == WrappedFuncType::Void_String, "Invalid wrapped function type provided");
			((WrappedVoidString*)wrappedFunction.get())->m_Value(argumentOne);
		}

		inline void CallWrappedVoidFloat(Ref<WrappedFunction> wrappedFunction, float argumentOne)
		{
			KG_ASSERT(wrappedFunction->Type() == WrappedFuncType::Void_Float, "Invalid wrapped function type provided");
			((WrappedVoidFloat*)wrappedFunction.get())->m_Value(argumentOne);
		}

		inline void CallWrappedVoidUInt16(Ref<WrappedFunction> wrappedFunction, uint16_t argumentOne)
		{
			KG_ASSERT(wrappedFunction->Type() == WrappedFuncType::Void_UInt16, "Invalid wrapped function type provided");
			((WrappedVoidUInt16*)wrappedFunction.get())->m_Value(argumentOne);
		}

		inline void CallWrappedVoidUInt32(Ref<WrappedFunction> wrappedFunction, uint32_t argumentOne)
		{
			KG_ASSERT(wrappedFunction->Type() == WrappedFuncType::Void_UInt32, "Invalid wrapped function type provided");
			((WrappedVoidUInt32*)wrappedFunction.get())->m_Value(argumentOne);
		}

		inline void CallWrappedVoidEntity(Ref<WrappedFunction> wrappedFunction, uint64_t argumentOne)
		{
			KG_ASSERT(wrappedFunction->Type() == WrappedFuncType::Void_Entity, "Invalid wrapped function type provided");
			((WrappedVoidEntity*)wrappedFunction.get())->m_Value(argumentOne);
		}

		inline void CallWrappedVoidEntityFloat(Ref<WrappedFunction> wrappedFunction, uint64_t argumentOne, float argumentTwo)
		{
			KG_ASSERT(wrappedFunction->Type() == WrappedFuncType::Void_EntityFloat, "Invalid wrapped function type provided");
			((WrappedVoidEntityFloat*)wrappedFunction.get())->m_Value(argumentOne, argumentTwo);
		}

		inline void CallWrappedVoidUInt32EntityEntityFloat(Ref<WrappedFunction> wrappedFunction, uint32_t argumentOne, uint64_t argumentTwo, uint64_t argumentThree, float argumentFour)
		{
			KG_ASSERT(wrappedFunction->Type() == WrappedFuncType::Void_UInt32EntityEntityFloat, "Invalid wrapped function type provided");
			((WrappedVoidUInt32EntityEntityFloat*)wrappedFunction.get())->m_Value(argumentOne, argumentTwo, argumentThree, argumentFour);
		}

		inline bool CallWrappedBoolNone(Ref<WrappedFunction> wrappedFunction)
		{
			KG_ASSERT(wrappedFunction->Type() == WrappedFuncType::Bool_None, "Invalid wrapped function type provided");
			return ((WrappedBoolNone*)wrappedFunction.get())->m_Value();
		}

		inline bool CallWrappedBoolEntity(Ref<WrappedFunction> wrappedFunction, uint64_t argumentOne)
		{
			KG_ASSERT(wrappedFunction->Type() == WrappedFuncType::Bool_Entity, "Invalid wrapped function type provided");
			return ((WrappedBoolEntity*)wrappedFunction.get())->m_Value(argumentOne);
		}

		inline bool CallWrappedBoolEntityEntity(Ref<WrappedFunction> wrappedFunction, uint64_t argumentOne, uint64_t argumentTwo)
		{
			KG_ASSERT(wrappedFunction->Type() == WrappedFuncType::Bool_EntityEntity, "Invalid wrapped function type provided");
			return ((WrappedBoolEntityEntity*)wrappedFunction.get())->m_Value(argumentOne, argumentTwo);
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
			case WrappedFuncType::Void_Entity:
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
			case WrappedFuncType::Void_UInt32:
				return { WrappedVarType::UInteger32 };
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
			bool foundFuncType{ false };
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
