#include "kgpch.h"

#include "Modules/Core/Components/TransformComponent.h"

namespace Kargono
{
	Math::mat4 TransformComponent::GetTransform() const
	{
		const Math::mat4 rotation{ glm::toMat4(Math::quat(m_Rotation)) };

		return glm::translate(Math::mat4(1.0f), m_Translation)
			* rotation
			* glm::scale(Math::mat4(1.0f), m_Scale);
	}

	Math::mat4 TransformComponent::GetTranslation() const
	{
		return glm::translate(Math::mat4(1.0f), m_Translation);
	}

	Math::mat4 TransformComponent::GetRotation() const
	{
		return glm::toMat4(Math::quat(m_Rotation));
	}
}