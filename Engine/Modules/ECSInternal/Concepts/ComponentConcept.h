#pragma once
#include "Modules/Core/Concepts/IsCopyable.h"
#include "Modules/Core/Concepts/HasSerialization.h"

#include "API/Serialization/yamlcppAPI.h"

namespace Kargono::ECSInternal
{
	template <typename t_Type>
	concept ComponentConcept = IsCopyable<t_Type> && HasSerialization<t_Type>;

	struct DeserializeComponentContext
	{
		YAML::Node* m_Node{ nullptr };
	};

	struct SerializeComponentContext
	{
		YAML::Emitter* m_Serializer{ nullptr };
	};
}