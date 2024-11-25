#pragma once

#include "Kargono/Math/Math.h"

#include <string>
#include <vector>

namespace Kargono::Rendering
{
	enum class RenderingType
	{
		None = 0, DrawIndex, DrawTriangle, DrawLine, DrawPoint
	};

	enum class ShapeTypes
	{
		None = 0, Quad, Cube, Pyramid
	};

	using Vertex = Math::vec3;

	class Shape
	{
	public:
		//============================================================
		// Constructors
		//============================================================
		Shape() = default;
		Shape(std::string_view name, const std::vector<Vertex>& indexVertices, const std::vector<Math::vec2>& indexTexCoordinates,
			const std::vector<uint32_t>& indices,
			const std::vector<Vertex>& triangleVertices, const std::vector<Math::vec2>& triangleTexCoordinates,
			RenderingType renderingType, ShapeTypes shapeType,
			const std::vector<std::string>& restrictedSpecs)
			: m_Name{ name }, m_IndexVertices{ indexVertices }, m_IndexTextureCoordinates{ indexTexCoordinates },m_Indices{ indices },
			m_TriangleVertices{triangleVertices}, m_TriangleTextureCoordinates{triangleTexCoordinates},
		m_RenderingType{ renderingType }, m_ShapeType{shapeType}, m_RestrictedSpecs(restrictedSpecs) {}
		std::string_view GetName() const { return m_Name; }
		//============================================================
		// Getters/Setters
		//============================================================
		// Index Getter/Setters
		const std::vector<Vertex>& GetIndexVertices() const { return m_IndexVertices; }
		const std::vector<uint32_t>& GetIndices() const { return m_Indices; }
		const std::vector<Math::vec2>& GetIndexTextureCoordinates() const { return m_IndexTextureCoordinates; }

		const std::vector<Vertex>& GetTriangleVertices() const { return m_TriangleVertices; }
		const std::vector<Math::vec2>& GetTriangleTextureCoordinates() const { return m_TriangleTextureCoordinates; }

		// Triangle Getters/Setters

		// Type Information Getters
		RenderingType GetRenderingType() const { return m_RenderingType; }
		ShapeTypes GetShapeType() const { return m_ShapeType; }
		const std::vector<std::string>& GetRestrictedSpecs() const { return m_RestrictedSpecs; }

	public:
		//============================================================
		// Static Object Declarations
		//============================================================
		static Shape s_Pyramid;
		static Shape s_Quad;
		static Shape s_Cube;
		static Shape s_None;

		inline static std::vector<Shape*> s_AllShapes {&s_Quad, &s_Cube, &s_None, &s_Pyramid};
	private:
		std::string m_Name {};
		// Indices Data
		std::vector<Vertex> m_IndexVertices {};
		std::vector<Math::vec2> m_IndexTextureCoordinates {};
		std::vector<uint32_t> m_Indices {};
		// Triangle Rendering Data
		std::vector<Vertex> m_TriangleVertices {};
		std::vector<Math::vec2> m_TriangleTextureCoordinates {};
		// Type Information
		RenderingType m_RenderingType { RenderingType::None};
		ShapeTypes m_ShapeType{ ShapeTypes::None };

		// Editor Information
		// This field labels shader specifications that should not be visible in the SceneHierarchy Panel
		// for this particular object.
		std::vector<std::string> m_RestrictedSpecs {};
	};

}

namespace Kargono::Utility
{
	static std::string RenderingTypeToString(Rendering::RenderingType renderType)
	{
		switch (renderType)
		{
		case Rendering::RenderingType::None: return "None";
		case Rendering::RenderingType::DrawIndex: return "DrawIndex";
		case Rendering::RenderingType::DrawTriangle: return "DrawTriangle";
		case Rendering::RenderingType::DrawPoint: return "DrawPoint";
		case Rendering::RenderingType::DrawLine: return "DrawLine";
		}
		KG_ERROR("Unknown Data Type sent to RenderingTypeToString Function");
		return "None";
	}

	static Rendering::RenderingType StringToRenderingType(std::string_view string)
	{
		if (string == "None") { return Rendering::RenderingType::None; }
		if (string == "DrawIndex") { return Rendering::RenderingType::DrawIndex; }
		if (string == "DrawTriangle") { return Rendering::RenderingType::DrawTriangle; }
		if (string == "DrawLine") { return Rendering::RenderingType::DrawLine; }
		if (string == "DrawPoint") { return Rendering::RenderingType::DrawPoint; }

		KG_ERROR("Unknown Data Type sent to StringToRenderingType Function");
		return Rendering::RenderingType::None;
	}

	static std::string ShapeTypeToString(Rendering::ShapeTypes shapeType)
	{
		switch (shapeType)
		{
		case Rendering::ShapeTypes::None: return "None";
		case Rendering::ShapeTypes::Quad: return "Quad";
		case Rendering::ShapeTypes::Cube: return "Cube";
		case Rendering::ShapeTypes::Pyramid: return "Pyramid";
		}
		KG_ERROR("Unknown Data Type sent to ShapeTypeToString Function");
		return "None";
	}

	static Rendering::ShapeTypes StringToShapeType(std::string_view string)
	{
		if (string == "None") { return Rendering::ShapeTypes::None; }
		if (string == "Quad") { return Rendering::ShapeTypes::Quad; }
		if (string == "Cube") { return Rendering::ShapeTypes::Cube; }
		if (string == "Pyramid") { return Rendering::ShapeTypes::Pyramid; }

		KG_ERROR("Unknown Data Type sent to StringToShapeType Function");
		return Rendering::ShapeTypes::None;
	}

	static const Rendering::Shape& ShapeTypeToShape(Rendering::ShapeTypes shapeType)
	{
		switch (shapeType)
		{
		case Rendering::ShapeTypes::Pyramid: return Rendering::Shape::s_Pyramid;
		case Rendering::ShapeTypes::Quad: return Rendering::Shape::s_Quad;
		case Rendering::ShapeTypes::Cube: return Rendering::Shape::s_Cube;
		case Rendering::ShapeTypes::None: return Rendering::Shape::s_None;
		}

		KG_ERROR("Unknown Data Type sent to StringToShapeType Function");
		return Rendering::Shape::s_None;
	}
}
