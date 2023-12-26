#pragma once

#include "Kargono/Scene/Components.h"
#include "Kargono/Math/Math.h"

#include "box2d/b2_body.h"
#include <yaml-cpp/yaml.h>

namespace YAML
{
	//==============================
	// Engine Specific YAML Conversions
	//==============================
	// These templates allow the YAML serialization library (yaml-cpp) to work with
	//		engine specific data structures (mostly glm). These are mostly useful
	//		for deserialization.
	template<>
	struct convert<Kargono::Math::vec2>
	{
		static Node encode(const Kargono::Math::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}
		static bool decode(const Node& node, Kargono::Math::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2) { return false; }

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Kargono::Math::vec3>
	{
		static Node encode(const Kargono::Math::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}
		static bool decode(const Node& node, Kargono::Math::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3) { return false; }

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Kargono::Math::vec4>
	{
		static Node encode(const Kargono::Math::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}
		static bool decode(const Node& node, Kargono::Math::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4) { return false; }

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Kargono::Math::ivec2>
	{
		static Node encode(const Kargono::Math::ivec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}
		static bool decode(const Node& node, Kargono::Math::ivec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2) { return false; }

			rhs.x = node[0].as<int32_t>();
			rhs.y = node[1].as<int32_t>();
			return true;
		}
	};

	template<>
	struct convert<Kargono::Math::ivec3>
	{
		static Node encode(const Kargono::Math::ivec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}
		static bool decode(const Node& node, Kargono::Math::ivec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3) { return false; }

			rhs.x = node[0].as<int32_t>();
			rhs.y = node[1].as<int32_t>();
			rhs.z = node[2].as<int32_t>();
			return true;
		}
	};

	template<>
	struct convert<Kargono::Math::ivec4>
	{
		static Node encode(const Kargono::Math::ivec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}
		static bool decode(const Node& node, Kargono::Math::ivec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4) { return false; }

			rhs.x = node[0].as<int32_t>();
			rhs.y = node[1].as<int32_t>();
			rhs.z = node[2].as<int32_t>();
			rhs.w = node[3].as<int32_t>();
			return true;
		}
	};

	template<>
	struct convert<Kargono::UUID>
	{
		static Node encode(const Kargono::UUID& uuid)
		{
			Node node;
			node.push_back((uint64_t)uuid);
			return node;
		}
		static bool decode(const Node& node, Kargono::UUID& uuid)
		{
			uuid = node.as<uint64_t>();
			return true;
		}
	};

	// These operator overloads serve a similar purpose to the templates,
	//		however, they are more useful for the Serialization part
	inline YAML::Emitter& operator<<(YAML::Emitter& out, const Kargono::Math::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const Kargono::Math::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const Kargono::Math::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const Kargono::Math::ivec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const Kargono::Math::ivec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const Kargono::Math::ivec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

}

namespace Kargono::Utility
{
	//==============================
	// Scripting Engine Serialization Macros
	//==============================
	// These macros allow for easy serialization of different field types for script entities. 

#define WRITE_SCRIPT_FIELD(FieldType, Type)\
					case FieldType:\
						out << scriptField.GetValue<Type>();\
						break 

#define READ_SCRIPT_FIELD(FieldType, Type)                  \
	case FieldType:                            \
	{                                                       \
		Type data = scriptField["Data"].as<Type>();       \
		fieldInstance.SetValue(data);                       \
		break;                                              \
	}
	//==============================
	// Extra Conversions
	//==============================
	// These are simply here to help with serialization for the rigid body components in an entity
	inline std::string RigidBody2DBodyTypeToString(Rigidbody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
		case Rigidbody2DComponent::BodyType::Static:	return "Static";
		case Rigidbody2DComponent::BodyType::Dynamic:	return "Dynamic";
		case Rigidbody2DComponent::BodyType::Kinematic:	return "Kinematic";
		}

		KG_CORE_ASSERT(false, "Unknown body type")
			return {};
	}

	inline Rigidbody2DComponent::BodyType StringToRigidBody2DBodyType(const std::string& bodyTypeString)
	{
		if (bodyTypeString == "Static") return Rigidbody2DComponent::BodyType::Static;
		if (bodyTypeString == "Dynamic") return Rigidbody2DComponent::BodyType::Dynamic;
		if (bodyTypeString == "Kinematic") return Rigidbody2DComponent::BodyType::Kinematic;

		KG_CORE_ASSERT(false, "Unknown body type")
			return Rigidbody2DComponent::BodyType::Static;
	}
}
