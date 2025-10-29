#include "kgpch.h"

#include "Modules/Rendering/Shape.h"

namespace Kargono::Rendering
{
	Shape::Shape(std::string_view name, const std::vector<Vertex>& indexVertices, const std::vector<Math::vec2>& indexTexCoordinates,
		const std::vector<uint32_t>& indices,
		const std::vector<Vertex>& triangleVertices, const std::vector<Math::vec2>& triangleTexCoordinates,
		RenderingType renderingType, ShapeTypes shapeType,
		const std::vector<std::string>& restrictedSpecs)
	: m_Name{ name }, m_IndexVertices{ indexVertices }, m_IndexTextureCoordinates{ indexTexCoordinates }, m_Indices{ indices },
		m_TriangleVertices{ triangleVertices }, m_TriangleTextureCoordinates{ triangleTexCoordinates },
		m_RenderingType{ renderingType }, m_ShapeType{ shapeType }, m_RestrictedSpecs(restrictedSpecs) {}

	Shape Shape::s_None {};
	Shape Shape::s_Quad
	{
		"Quad",
		{
			{-0.5f, -0.5f, 0.0f},		// 0
			{0.5f, -0.5f, 0.0f},		// 1
			{0.5f, 0.5f, 0.0f},		// 2
			{-0.5f, 0.5f, 0.0f}		// 3
		},
		{
			{0.0f, 0.0f},					// 0
			{1.0f, 0.0f},					// 1
			{1.0f, 1.0f},					// 2
			{0.0f, 1.0f}					// 3
		},
		{
			0, 1, 2, 0, 3, 2
		},
		{}, {},
		RenderingType::DrawIndex,
		ShapeTypes::Quad,
		{}
	};

	Shape Shape::s_Cube
	{
		"Cube",
		{
			{-0.5f, -0.5f, 0.5f},		// 0
			{0.5f, -0.5f, 0.5f},		// 1
			{0.5f, 0.5f, 0.5f},		// 2
			{-0.5f, 0.5f, 0.5f},		// 3
			{-0.5f, -0.5f, -0.5f},		// 4
			{0.5f, -0.5f, -0.5f},		// 5
			{0.5f, 0.5f, -0.5f},		// 6
			{-0.5f, 0.5f, -0.5f}		// 7
		},
		{
			{0.0f, 0.0f},					// 0
			{1.0f, 0.0f},					// 1
			{1.0f, 1.0f},					// 2
			{0.0f, 1.0f},					// 3
			{0.0f, 0.0f},					// 4
			{1.0f, 0.0f},					// 5
			{1.0f, 1.0f},					// 6
			{0.0f, 1.0f}					// 7
		},
		{
			0, 1, 2, 0, 3, 2,
			4, 5, 6, 4, 7, 6,
			1, 5, 6, 1, 2, 6,
			0, 4, 7, 0, 3, 7,
			0, 1, 4, 1, 5, 4,
			3, 2, 7, 2, 6, 7
		},
		{
			{-0.5f, -0.5f, 0.5f},		// 0
			{0.5f, -0.5f, 0.5f},		// 1
			{0.5f, 0.5f, 0.5f},		// 2

			{-0.5f, -0.5f, 0.5f},		// 0
			{-0.5f, 0.5f, 0.5f},		// 3
			{0.5f, 0.5f, 0.5f},		// 2

			{-0.5f, -0.5f, -0.5f},		// 4
			{0.5f, -0.5f, -0.5f},		// 5
			{0.5f, 0.5f, -0.5f},		// 6

			{-0.5f, -0.5f, -0.5f},		// 4
			{-0.5f, 0.5f, -0.5f},		// 7
			{0.5f, 0.5f, -0.5f},		// 6

			{0.5f, -0.5f, 0.5f},		// 1
			{0.5f, -0.5f, -0.5f},		// 5
			{0.5f, 0.5f, -0.5f},		// 6

			{0.5f, -0.5f, 0.5f},		// 1
			{0.5f, 0.5f, 0.5f},		// 2
			{0.5f, 0.5f, -0.5f},		// 6

			{-0.5f, -0.5f, 0.5f},		// 0
			{-0.5f, -0.5f, -0.5f},		// 4
			{-0.5f, 0.5f, -0.5f},		// 7

			{-0.5f, -0.5f, 0.5f},		// 0
			{-0.5f, 0.5f, 0.5f},		// 3
			{-0.5f, 0.5f, -0.5f},		// 7

			{-0.5f, -0.5f, 0.5f},		// 0
			{0.5f, -0.5f, 0.5f},		// 1
			{-0.5f, -0.5f, -0.5f},		// 4

			{0.5f, -0.5f, 0.5f},		// 1
			{0.5f, -0.5f, -0.5f},		// 5
			{-0.5f, -0.5f, -0.5f},		// 4

			{-0.5f, 0.5f, 0.5f},		// 3
			{0.5f, 0.5f, 0.5f},		// 2
			{-0.5f, 0.5f, -0.5f},		// 7

			{0.5f, 0.5f, 0.5f},		// 2
			{0.5f, 0.5f, -0.5f},		// 6
			{-0.5f, 0.5f, -0.5f}		// 7
		},
		{
			
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},

			{0.0f, 0.0f},
			{0.0f, 1.0f},
			{1.0f, 1.0f},

			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},

			{0.0f, 0.0f},
			{0.0f, 1.0f},
			{1.0f, 1.0f},

			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},

			{0.0f, 0.0f},
			{0.0f, 1.0f},
			{1.0f, 1.0f},

			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},

			{0.0f, 0.0f},
			{0.0f, 1.0f},
			{1.0f, 1.0f},

			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{0.0f, 1.0f},

			{1.0f, 0.0f},
			{1.0f, 1.0f},
			{0.0f, 1.0f},

			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{0.0f, 1.0f},

			{1.0f, 0.0f},
			{1.0f, 1.0f},
			{0.0f, 1.0f}
		},
			RenderingType::DrawIndex,
			ShapeTypes::Cube,
		{"AddTexture", "AddCircleShape"}
	};
	Shape Shape::s_Pyramid
	{
		"Pyramid",
		{
			{-0.5f, -0.5f, 0.5f},		// 0
			{0.5f, -0.5f, 0.5f},		// 1
			{0.5f, -0.5f, -0.5f},		// 2
			{-0.5f, -0.5f, -0.5f},		// 3
			{0.0f, 0.5f, 0.0f},		// 4
		},
		{},
		{
			0, 1, 4,
			1, 2, 4,
			2, 3, 4,
			3, 0, 4,
			0, 1, 3,
			1, 2, 3
		},
		{
			{-0.5f, -0.5f, 0.5f},		// 0
			{0.5f, -0.5f, 0.5f},		// 1
			{0.0f, 0.5f, 0.0f},		// 4

			{0.5f, -0.5f, 0.5f},		// 1
			{0.5f, -0.5f, -0.5f},		// 2
			{0.0f, 0.5f, 0.0f},		// 4

			{0.5f, -0.5f, -0.5f},		// 2
			{-0.5f, -0.5f, -0.5f},		// 3
			{0.0f, 0.5f, 0.0f},		// 4

			{-0.5f, -0.5f, -0.5f},		// 3
			{-0.5f, -0.5f, 0.5f},		// 0
			{0.0f, 0.5f, 0.0f},		// 4

			{-0.5f, -0.5f, 0.5f},		// 0
			{0.5f, -0.5f, 0.5f},		// 1
			{-0.5f, -0.5f, -0.5f},		// 3

			{0.5f, -0.5f, 0.5f},		// 1
			{0.5f, -0.5f, -0.5f},		// 2
			{-0.5f, -0.5f, -0.5f},		// 3
		},
		{
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{0.5f, 1.0f},

			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{0.5f, 1.0f},

			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{0.5f, 1.0f},

			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{0.5f, 1.0f},

			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{0.0f, 1.0f},

			{1.0f, 0.0f},
			{1.0f, 1.0f},
			{0.0f, 1.0f}

		},
			RenderingType::DrawIndex,
			ShapeTypes::Pyramid,
		{ "AddTexture", "AddCircleShape" }
	};
}
