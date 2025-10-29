#include "kgpch.h"

#include "Modules/Cameras/Components/CameraComponent.h"

namespace Kargono::Cameras
{
	void CameraComponent::Serialize(void* context)
	{
		// Get context
		ECSInternal::SerializeComponentContext* serializeContext =
			(ECSInternal::SerializeComponentContext*)context;
		KG_ASSERT(serializeContext, "Context cannot be null");
		KG_ASSERT(serializeContext->m_Serializer, "Serializer cannot be null");
		// Get serializer
		YAML::Emitter& out = *serializeContext->m_Serializer;
		// Serialize component
		out << YAML::Key << "CameraComponent";
		out << YAML::BeginMap; // Component Map

		out << YAML::Key << "Camera" << YAML::Value;
		out << YAML::BeginMap;
		out << YAML::Key << "ProjectionType" << YAML::Value << (int)m_Camera.GetProjectionType();
		out << YAML::Key << "PerspectiveFOV" << YAML::Value << m_Camera.GetPerspectiveVerticalFOV();
		out << YAML::Key << "PerspectiveNear" << YAML::Value << m_Camera.GetPerspectiveNearClip();
		out << YAML::Key << "PerspectiveFar" << YAML::Value << m_Camera.GetPerspectiveFarClip();

		out << YAML::Key << "OrthographicSize" << YAML::Value << m_Camera.GetOrthographicSize();
		out << YAML::Key << "OrthographicNear" << YAML::Value << m_Camera.GetOrthographicNearClip();
		out << YAML::Key << "OrthographicFar" << YAML::Value << m_Camera.GetOrthographicFarClip();
		out << YAML::EndMap;

		out << YAML::EndMap; // Component Map
	}
	void CameraComponent::Deserialize(void* context)
	{
		// Get context
		ECSInternal::DeserializeComponentContext* deserializeContext =
			(ECSInternal::DeserializeComponentContext*)context;
		KG_ASSERT(deserializeContext, "Context cannot be null");
		KG_ASSERT(deserializeContext->m_Node, "Registry node cannot be null");
		// Get node
		YAML::Node& node = *deserializeContext->m_Node;

		// Deserialize
		YAML::Node cameraProps = node["Camera"];
		m_Camera.SetProjectionType((Cameras::ProjectionType)cameraProps["ProjectionType"].as<int>());
		m_Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
		m_Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
		m_Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

		m_Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
		m_Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
		m_Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());
	}
}