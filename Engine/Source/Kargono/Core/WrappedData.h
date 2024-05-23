#pragma once

#include "Kargono/Core/Buffer.h"
#include "Kargono/Utility/Conversions.h"

#include <cstdint>
#include <functional>
#include <string>

namespace Kargono
{
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
		String
	};

	inline WrappedVarType s_AllWrappedVarTypes[] =
	{
		WrappedVarType::Integer32,
		WrappedVarType::UInteger16,
		WrappedVarType::UInteger32,
		WrappedVarType::UInteger64,
		WrappedVarType::String,
		WrappedVarType::Void,
		WrappedVarType::Bool,
		WrappedVarType::Float
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

	enum class WrappedFuncType
	{
		None = 0,
		Void_None,
		Void_String,
		Void_Float,
		Void_UInt16,
		Void_UInt32,
		Void_UInt64,
		Bool_None,
		Bool_UInt64
	};

	inline WrappedFuncType s_AllWrappedFuncs[] = 
	{
		WrappedFuncType::Void_None,
		WrappedFuncType::Void_String,
		WrappedFuncType::Void_Float,
		WrappedFuncType::Void_UInt16,
		WrappedFuncType::Void_UInt32,
		WrappedFuncType::Void_UInt64,
		WrappedFuncType::Bool_None,
		WrappedFuncType::Bool_UInt64,
	};

	typedef void (*void_none)();
	typedef void (*void_string)(std::string);
	typedef void (*void_float)(float);
	typedef void (*void_uint16)(uint16_t);
	typedef void (*void_uint32)(uint32_t);
	typedef void (*void_uint64)(uint64_t);
	typedef bool (*bool_none)();
	typedef bool (*bool_uint64)(uint64_t);

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
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_None; }
	public:
		std::function<void()> m_Value{};
	};

	class WrappedVoidString : public WrappedFunction
	{
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_String; }
	public:
		std::function<void(const std::string&)> m_Value{};
	};

	class WrappedVoidFloat : public WrappedFunction
	{
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_Float; }
	public:
		std::function<void(float)> m_Value{};
	};

	class WrappedVoidUInt16 : public WrappedFunction
	{
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_UInt16; }
	public:
		std::function<void(uint16_t)> m_Value{};
	};

	class WrappedVoidUInt32 : public WrappedFunction
	{
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_UInt32; }
	public:
		std::function<void(uint32_t)> m_Value{};
	};

	class WrappedVoidUInt64 : public WrappedFunction
	{
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Void_UInt64; }
	public:
		std::function<void(uint64_t)> m_Value{};
	};

	class WrappedBoolNone : public WrappedFunction
	{
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Bool_None; }
	public:
		std::function<bool()> m_Value{};
	};

	class WrappedBoolUInt64 : public WrappedFunction
	{
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Bool_UInt64; }
	public:
		std::function<bool(uint64_t)> m_Value{};
	};

	

	namespace Utility
	{
		inline std::string WrappedVarTypeToString(WrappedVarType type)
		{
			switch (type)
			{
			case WrappedVarType::Integer32: return "Integer32";
			case WrappedVarType::UInteger16: return "UInteger16";
			case WrappedVarType::UInteger32: return "UInteger32";
			case WrappedVarType::UInteger64: return "UInteger64";
			case WrappedVarType::String: return "String";
			case WrappedVarType::Void: return "Void";
			case WrappedVarType::Bool: return "Bool";
			case WrappedVarType::Float: return "Float";
			case WrappedVarType::None: return "None";
			}
			KG_ASSERT(false, "Unknown Type of WrappedVariableType.");
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
			case WrappedVarType::String: return CreateRef<WrappedString>();
			case WrappedVarType::Bool: return CreateRef<WrappedBool>();
			case WrappedVarType::Float: return CreateRef<WrappedFloat>();
			case WrappedVarType::Void:
			case WrappedVarType::None:
				return nullptr;
			}
			KG_ASSERT(false, "Unknown Type of WrappedVariableType.");
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
			case WrappedVarType::String: return "std::string";
			case WrappedVarType::Void: return "void";
			case WrappedVarType::Bool: return "bool";
			case WrappedVarType::Float: return "float";
			case WrappedVarType::None: return "None";
			}
			KG_ASSERT(false, "Unknown Type of WrappedVariableType.");
			return "";
		}

		inline WrappedVarType StringToWrappedVarType(std::string type)
		{
			if (type == "Integer32") { return WrappedVarType::Integer32; }
			if (type == "UInteger16") { return WrappedVarType::UInteger16; }
			if (type == "UInteger32") { return WrappedVarType::UInteger32; }
			if (type == "UInteger64") { return WrappedVarType::UInteger64; }
			if (type == "String") { return WrappedVarType::String; }
			if (type == "Void") { return WrappedVarType::Void; }
			if (type == "Bool") { return WrappedVarType::Bool; }
			if (type == "Float") { return WrappedVarType::Float; }
			if (type == "None") { return WrappedVarType::None; }

			KG_ASSERT(false, "Unknown Type of WrappedVariableType String.");
			return WrappedVarType::None;
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
				case WrappedVarType::String:
				{
					variable->SetValue(buffer.As<char>());
					break;
				}
				case WrappedVarType::Bool:
				{
						// TODO Work on Bool Later
					/*success = Conversions::CharBufferToVariable(buffer,
						variable->GetWrappedValue<bool>());*/
					KG_TRACE("Attempt to use bool. Have not completed it!");
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
			return true;
		}

		inline std::string WrappedFuncTypeToString(WrappedFuncType type)
		{
			switch (type)
			{
			case WrappedFuncType::Void_None: return "Void_None";
			case WrappedFuncType::Void_String: return "Void_String";
			case WrappedFuncType::Void_Float: return "Void_Float";
			case WrappedFuncType::Void_UInt16: return "Void_UInt16";
			case WrappedFuncType::Void_UInt32: return "Void_UInt32";
			case WrappedFuncType::Void_UInt64: return "Void_UInt64";
			case WrappedFuncType::Bool_None: return "Bool_None";
			case WrappedFuncType::Bool_UInt64: return "Bool_UInt64";
			case WrappedFuncType::None: return "None";
			}
			KG_ASSERT(false, "Unknown Type of WrappedType.");
			return "";
		}

		inline WrappedFuncType StringToWrappedFuncType(std::string type)
		{
			if (type == "Void_None") { return WrappedFuncType::Void_None; }
			if (type == "Void_String") { return WrappedFuncType::Void_String; }
			if (type == "Void_Float") { return WrappedFuncType::Void_Float; }
			if (type == "Void_UInt16") { return WrappedFuncType::Void_UInt16; }
			if (type == "Void_UInt32") { return WrappedFuncType::Void_UInt32; }
			if (type == "Void_UInt64") { return WrappedFuncType::Void_UInt64; }
			if (type == "Bool_None") { return WrappedFuncType::Bool_None; }
			if (type == "Bool_UInt64") { return WrappedFuncType::Bool_UInt64; }
			if (type == "None") { return WrappedFuncType::None; }

			KG_ASSERT(false, "Unknown Type of WrappedFuncType String.");
			return WrappedFuncType::None;
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
			case WrappedFuncType::Void_UInt64:
				return WrappedVarType::Void;
			case WrappedFuncType::Bool_None:
			case WrappedFuncType::Bool_UInt64:
				return WrappedVarType::Bool;
			case WrappedFuncType::None:
			{
				KG_ERROR("None type provided to return type utility function");
				return WrappedVarType::None;
			}
			default:
				KG_ASSERT(false, "Unknown Type of WrappedType.")
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
			case WrappedFuncType::Void_UInt64:
			case WrappedFuncType::Bool_UInt64:
				return { WrappedVarType::UInteger64 };
			case WrappedFuncType::None:
			{
				KG_ERROR("None type provided to parameter list utility function");
				return {};
			}
			default:
				KG_ASSERT(false, "Unknown Type of WrappedFuncType.")
				return {};
			}
		}
	}
	
}
