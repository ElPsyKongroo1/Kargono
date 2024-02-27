#pragma once

#include <cstdint>
#include <functional>

namespace Kargono
{
	enum class WrappedVarType
	{
		None = 0,
		Void,
		Bool,
		Integer32,
		UInteger64
	};

	class WrappedVariable
	{
	public:
		virtual ~WrappedVariable() = default;
	public:
		virtual WrappedVarType Type() = 0;
	};

	class WrappedInteger32 : public WrappedVariable
	{
	public:
		virtual WrappedVarType Type() override { return WrappedVarType::Integer32; }
	public:
		int32_t m_Value{};
	};

	class WrappedUInteger64 : public WrappedVariable
	{
	public:
		virtual WrappedVarType Type() override { return WrappedVarType::UInteger64; }
	public:
		int32_t m_Value{};
	};

	enum class WrappedFuncType
	{
		None = 0,
		Void_None,
		Bool_None
	};

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

	class WrappedBoolNone : public WrappedFunction
	{
	public:
		virtual WrappedFuncType Type() override { return WrappedFuncType::Bool_None; }
	public:
		std::function<bool()> m_Value{};
	};

	namespace Utility
	{
		inline std::string WrappedVarTypeToString(WrappedVarType type)
		{
			switch (type)
			{
			case WrappedVarType::Integer32: return "Integer32";
			case WrappedVarType::UInteger64: return "UInteger64";
			case WrappedVarType::Void: return "Void";
			case WrappedVarType::Bool: return "Bool";
			case WrappedVarType::None: return "None";
			}
			KG_ASSERT(false, "Unknown Type of WrappedVariableType.");
			return "";
		}

		inline std::string WrappedVarTypeToCPPString(WrappedVarType type)
		{
			switch (type)
			{
			case WrappedVarType::Integer32: return "int32_t";
			case WrappedVarType::UInteger64: return "int64_t";
			case WrappedVarType::Void: return "void";
			case WrappedVarType::Bool: return "bool";
			case WrappedVarType::None: return "None";
			}
			KG_ASSERT(false, "Unknown Type of WrappedVariableType.");
			return "";
		}

		inline WrappedVarType StringToWrappedVarType(std::string type)
		{
			if (type == "Integer32") { return WrappedVarType::Integer32; }
			if (type == "UInteger64") { return WrappedVarType::UInteger64; }
			if (type == "Void") { return WrappedVarType::Void; }
			if (type == "Bool") { return WrappedVarType::Bool; }
			if (type == "None") { return WrappedVarType::None; }

			KG_ASSERT(false, "Unknown Type of WrappedVariableType String.");
			return WrappedVarType::None;
		}

		inline std::string WrappedFuncTypeToString(WrappedFuncType type)
		{
			switch (type)
			{
			case WrappedFuncType::Void_None: return "Void_None";
			case WrappedFuncType::Bool_None: return "Bool_None";
			case WrappedFuncType::None: return "None";
			}
			KG_ASSERT(false, "Unknown Type of WrappedType.");
			return "";
		}

		inline WrappedFuncType StringToWrappedFuncType(std::string type)
		{
			if (type == "Void_None") { return WrappedFuncType::Void_None; }
			if (type == "Bool_None") { return WrappedFuncType::Bool_None; }
			if (type == "None") { return WrappedFuncType::None; }

			KG_ASSERT(false, "Unknown Type of WrappedFuncType String.");
			return WrappedFuncType::None;
		}
	}
	
}
