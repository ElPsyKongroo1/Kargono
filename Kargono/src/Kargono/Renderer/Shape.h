#pragma once

#include "glm/glm.hpp"

#include <string>
#include <vector>

namespace Kargono
{
	class Shape
	{
	public:

		enum class RenderingType
		{
			None = 0, DrawIndex, DrawTriangle, DrawLine
		};

		static std::string RenderingTypeToString(Shape::RenderingType renderType)
		{
			switch (renderType)
			{
			case Shape::RenderingType::None: return "None";
			case Shape::RenderingType::DrawIndex: return "DrawIndex";
			case Shape::RenderingType::DrawTriangle: return "DrawTriangle";
			case Shape::RenderingType::DrawLine: return "DrawLine";
			}
			KG_CORE_ASSERT(false, "Unknown Data Type sent to RenderingTypeToString Function");
			return "None";
		}

		static Shape::RenderingType StringToRenderingType(std::string_view string)
		{
			if (string == "None") { return Shape::RenderingType::None; }
			if (string == "DrawIndex") { return Shape::RenderingType::DrawIndex; }
			if (string == "DrawTriangle") { return Shape::RenderingType::DrawTriangle; }
			if (string == "DrawLine") { return Shape::RenderingType::DrawLine; }

			KG_CORE_ASSERT(false, "Unknown Data Type sent to StringToRenderingType Function");
			return Shape::RenderingType::None;
		}

		enum class ShapeTypes
		{
			None = 0, Quad, Cube
		};

		static std::string ShapeTypeToString(Shape::ShapeTypes shapeType)
		{
			switch (shapeType)
			{
			case Shape::ShapeTypes::None: return "None";
			case Shape::ShapeTypes::Quad: return "Quad";
			case Shape::ShapeTypes::Cube: return "Cube";
			}
			KG_CORE_ASSERT(false, "Unknown Data Type sent to ShapeTypeToString Function");
			return "None";
		}

		static Shape::ShapeTypes StringToShapeType(std::string_view string)
		{
			if (string == "None") { return Shape::ShapeTypes::None; }
			if (string == "Quad") { return Shape::ShapeTypes::Quad; }
			if (string == "Cube") { return Shape::ShapeTypes::Cube; }

			KG_CORE_ASSERT(false, "Unknown Data Type sent to StringToShapeType Function");
			return Shape::ShapeTypes::None;
		}

		static const Shape& ShapeTypeToShape(Shape::ShapeTypes shapeType)
		{
			switch (shapeType)
			{
			case Shape::ShapeTypes::Quad: return Shape::Quad;
			case Shape::ShapeTypes::Cube: return Shape::Cube;
			case Shape::ShapeTypes::None: return Shape::None;
			}

			KG_CORE_ASSERT(false, "Unknown Data Type sent to StringToShapeType Function");
			return Shape::None;
		}


		using Vertex = glm::vec3;
	public:
		Shape() = default;
		Shape(std::string_view name, const std::vector<Vertex>& vertices, const std::vector<glm::vec2>& texCoordinates, const std::vector<uint32_t>& indices, Shape::RenderingType type)
			: m_Name{ name }, m_Vertices{ vertices }, m_TextureCoordinates{texCoordinates},  m_Indices{ indices }, m_RenderingType{type} {}
		std::string_view GetName() const { return m_Name; }
		const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
		const std::vector<glm::vec2>& GetTextureCoordinates() const { return m_TextureCoordinates; }
		const std::vector<uint32_t>& GetIndices() const { return m_Indices; }
		Shape::RenderingType GetType() const { return m_RenderingType; }
	public:
		static Shape Quad;
		static Shape Cube;
		static Shape None;
	private:
		std::string m_Name;
		std::vector<Vertex> m_Vertices;
		std::vector<glm::vec2> m_TextureCoordinates;
		std::vector<uint32_t> m_Indices;
		RenderingType m_RenderingType;
	};

}
