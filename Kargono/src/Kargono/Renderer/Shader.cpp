#include "kgpch.h"

#include "Kargono/Renderer/Shader.h"
#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Scene/Scene.h"
#include "Kargono/Core/FileSystem.h"
#include "API/OpenGL/OpenGLShader.h"

#include <spdlog/fmt/ostr.h>


namespace Kargono {
	Ref<Shader> Shader::Create(const std::filesystem::path& filepath)
	{
		return CreateRef<OpenGLShader>(filepath);
	}

	Ref<Shader> Shader::Create(const std::string& name, const Shader::ShaderSource& fullSource)
	{
		return  CreateRef<OpenGLShader>(name, fullSource);

	}

	Ref<Shader> Shader::Create(const std::string& name, const Shader::ShaderSpecification shaderSpec)
	{
		return  CreateRef<OpenGLShader>(name, shaderSpec);
	}

	Ref<Shader> Shader::Create(const std::string& name, const std::unordered_map<GLenum, std::vector<uint32_t>>& shaderBinaries)
	{
		return  CreateRef<OpenGLShader>(name, shaderBinaries);
	}

	//============================================================
	// Primitives
	//============================================================

	// glsl Specific Statements
	static constexpr std::string_view s_VersionLine = "#version 450 core";
	static constexpr std::string_view s_ShaderType = "#type {0}";
	static constexpr std::string_view s_VertexLayout = "layout(location = {0}) {1} {2} {3};";
	static constexpr std::string_view s_BindingLayoutStart = "layout({0}, binding = {1}) uniform {2}";
	static constexpr std::string_view s_DiscardStatement = "Discard;";

	// Semicolons
	static constexpr std::string_view s_CloseParentheses = "}";
	static constexpr std::string_view s_CloseParenthesesWithSemi = "};";
	static constexpr std::string_view s_OpenParentheses = "{";

	// Declarations
	static constexpr std::string_view s_MainDeclaration = "void main()";
	static constexpr std::string_view s_Declaration = "{0} {1};";
	static constexpr std::string_view s_StructDeclaration = "struct {0}";
	static constexpr std::string_view s_SwitchDeclaration = "switch ({0})";
	static constexpr std::string_view s_SwitchCase = "case {0}:";
	static constexpr std::string_view s_ClassDeclaration = "class {0}";
	static constexpr std::string_view s_DeclarationWithAssignment = "{0} {1} = {2};";
	static constexpr std::string_view s_ConstructorCall = "{0}({1})";

	// Unary Operators
	static constexpr std::string_view s_Negation = "-{0}";

	// Binary Operators
	static constexpr std::string_view s_Multiplication = "{0} * {1}";
	static constexpr std::string_view s_Division = "{0} / {1}";
	static constexpr std::string_view s_Addition = "{0} + {1}";
	static constexpr std::string_view s_Subtraction = "{0} - {1}";
	static constexpr std::string_view s_Modulo = "{0} % {1}";
	static constexpr std::string_view s_MultiplyEquals = "{0} *= {1}";
	static constexpr std::string_view s_DivideEquals = "{0} /= {1}";
	static constexpr std::string_view s_SubtractEquals = "{0} -= {1}";
	static constexpr std::string_view s_AddEquals = "{0} += {1}";
	static constexpr std::string_view s_ModuloEquals = "{0} %= {1}";

	// Logical Operators
	static constexpr std::string_view s_LessThanOrEquals = "{0} <= {1}";
	static constexpr std::string_view s_GreaterThanOrEquals = "{0} >= {1}";
	static constexpr std::string_view s_LogicallyEquivalent = "{0} == {1}";
	static constexpr std::string_view s_NotLogicallyEquivalent = "{0} != {1}";
	static constexpr std::string_view s_NotOperator = "!{1}";

	// Control Statements
	static constexpr std::string_view s_BreakStatement = "Break;";
	static constexpr std::string_view s_ContinueStatement = "Continue;";

	// Arguments
	static constexpr std::string_view s_SingleArgument = "{0}";
	static constexpr std::string_view s_DoubleArgument = "{0},{1}";
	static constexpr std::string_view s_TripleArgument = "{0},{1},{2}";
	static constexpr std::string_view s_QuadArgument = "{0},{1},{2},{3}";

	using Priority = uint8_t;

	static std::stringstream s_OutputStream;
	static InputBufferLayout s_InputBufferLayout;
	static UniformBufferList s_UniformBufferLayout;

	// Maps to Hold Lambdas
	static std::map<Priority, std::function<void()>> s_VertexStructs {};
	static std::map<Priority, std::function<void(uint16_t count)>> s_VertexInput {};
	static std::map<Priority, std::function<void()>> s_VertexToFragmentSmooth {};
	static std::map<Priority, std::function<void(uint16_t count, std::string_view inputOrOutput)>> s_VertexToFragmentFlat {};
	static std::map<Priority, std::function<void(uint16_t count)>> s_VertexUniforms {};
	static std::map<Priority, std::function<void()>> s_VertexMain {};

	static std::map<Priority, std::function<void()>> s_FragmentStructs {};
	static std::map<Priority, std::function<void()>> s_FragmentFunctions {};
	static std::map<Priority, std::function<void(uint16_t count)>> s_FragmentOutput {};
	static std::map<Priority, std::function<void(uint16_t count)>> s_FragmentUniforms {};
	static std::map<Priority, std::function<void()>> s_FragmentMain {};

	//============================================================
	// Support Functions
	//============================================================

	static void UpdateInputBuffer(std::string_view name, std::string_view type)
	{
		s_InputBufferLayout.AddBufferElement(InputBufferElement(StringToInputDataType(type), name));
	}
	static void UpdateUniformBuffer(std::string_view name, std::string_view type)
	{
		s_UniformBufferLayout.AddBufferElement(UniformElement(StringToUniformDataType(type), name));
	}

	// Function to Run all lambdas in each map
	// TODO: Consider templating these functions if they get crazy!
	static uint16_t RunFunctions(std::map<Priority, std::function<void()>>& map)
	{
		uint16_t iterator{ 0 };
		for (const auto& [priority, function] : map)
		{
			function();
			iterator++;
		}
		return iterator;
	}
	// Function to Run all lambdas in each map
	static uint16_t RunFunctionsWithCount(std::map<Priority, std::function<void(uint16_t)>>& map, uint16_t initialCount = 0)
	{
		uint16_t iterator{ initialCount };
		for (const auto& [priority, function] : map)
		{
			function(iterator);
			iterator++;
		}
		return iterator;
	}

	// Function to Run all lambdas in each map
	static uint16_t RunFunctionsWithTypeAndCount(std::map<Priority, std::function<void(uint16_t, std::string_view)>>& map, std::string_view type, uint16_t initialCount = 0)
	{
		uint16_t iterator{ initialCount };
		for (const auto& [priority, function] : map)
		{
			function(iterator, type);
			iterator++;
		}
		return iterator;
	}

	// Function to ensure no collisions when adding an element into a map!
	template <typename Map, typename Lambda>
	static void InsertMap(Map& map, Priority key, Lambda lambda)
	{
		while(map.contains(key))
		{
			key++;
		}
		map.insert({ key, lambda });
	}

	// TODO: Figure out what these functions are...

	static void BeginShader(std::string_view shaderName )
	{
		s_OutputStream << "//============================================================\r\n";
		s_OutputStream << "// " << shaderName << " shader\r\n";
		s_OutputStream << "//============================================================\r\n";

		s_OutputStream << fmt::format(s_ShaderType, shaderName) << "\r\n";
		s_OutputStream << s_VersionLine << "\r\n";
	}

	static void BeginMain()
	{
		s_OutputStream << s_MainDeclaration << "\r\n";
		s_OutputStream << "{\r\n";
	}
	static void EndMain()
	{
		s_OutputStream << "}\r\n";
	}

	static void LineSpace()
	{
		s_OutputStream << "\r\n";
	}

	//============================================================
	// Functions to Populate Maps
	//============================================================

	//=================
	// Core Features
	//=================

	static void AddSimpleVertexOutput()
	{
		InsertMap(s_VertexInput, 50, [&](uint16_t count)
			{
				const std::string name = "a_Position";
				const std::string type = "vec3";
				s_OutputStream << "layout(location = " << count << ") in " << type << " " << name << ";\r\n";
				UpdateInputBuffer(name, type);
			} );

		InsertMap(s_VertexMain, 10, [&]()
			{
				s_OutputStream << "\tgl_Position = vec4(a_Position, 1.0);\r\n";
			} );
	}

	static void AddSimpleFragmentOutput()
	{
		InsertMap(s_FragmentOutput, 50, [&](uint16_t count)
			{
				s_OutputStream << "layout(location = " << count << ") out vec4 o_Color;\r\n";
			});

		InsertMap(s_FragmentMain, 10, [&]()
			{
				s_OutputStream << "\to_Color = vec4(1.0f, 1.0f, 1.0f, 1.0f);\r\n";
			});
	}

	//=================
	// Pixel Color Features
	//=================

	static void AddColorOutput()
	{
		InsertMap(s_VertexInput, 70, [&](uint16_t count)
			{
				const std::string name = "a_Color";
				const std::string type = "vec4";
				s_OutputStream << "layout(location = " << count << ") in " << type << " " << name << ";\r\n";
				UpdateInputBuffer(name, type);
			});

		InsertMap(s_VertexToFragmentSmooth, 50, [&]()
			{
				s_OutputStream << "\tvec4 Color;\r\n";
			} );

		InsertMap(s_VertexMain, 30, [&]()
			{
				s_OutputStream << "\tOutput.Color = a_Color;\r\n";
			} );

		InsertMap(s_FragmentMain, 70, [&]()
			{
				s_OutputStream << "\to_Color *= Input.Color;\r\n";
			} );
	}

	static void AddTextureOutput()
	{
		InsertMap(s_VertexInput, 70, [&](uint16_t count)
			{
				const std::string name = "a_TexCoord";
				const std::string type = "vec2";
				s_OutputStream << "layout(location = " << count << ") in " << type << " " << name << ";\r\n";
				UpdateInputBuffer(name, type);
			});

		InsertMap(s_VertexInput, 70, [&](uint16_t count)
			{
				const std::string name = "a_TexIndex";
				const std::string type = "float";
				s_OutputStream << "layout(location = " << count << ") in " << type << " " << name << ";\r\n";
				UpdateInputBuffer(name, type);
			});

		InsertMap(s_VertexInput, 70, [&](uint16_t count)
			{
				const std::string name = "a_TilingFactor";
				const std::string type = "float";
				s_OutputStream << "layout(location = " << count << ") in " << type << " " << name << ";\r\n";
				UpdateInputBuffer(name, type);
			});

		InsertMap(s_VertexToFragmentSmooth, 70, [&]()
			{
				s_OutputStream << "\tvec2 TexCoord;\r\n";
			});
		InsertMap(s_VertexToFragmentSmooth, 70, [&]()
			{
				s_OutputStream << "\tfloat TilingFactor;\r\n";
			});

		InsertMap(s_VertexToFragmentFlat, 70, [&](uint16_t count, std::string_view type)
			{
				s_OutputStream << "layout (location = " << count <<") " << type << " flat float v_TexIndex;\r\n";
			});

		InsertMap(s_VertexMain, 40, [&]()
			{
				s_OutputStream << "\tOutput.TexCoord = a_TexCoord;\r\n";
				s_OutputStream << "\tOutput.TilingFactor = a_TilingFactor;\r\n";
				s_OutputStream << "\tv_TexIndex = a_TexIndex;\r\n";
			});

		InsertMap(s_FragmentUniforms, 40, [&](uint16_t count)
			{
				const std::string name = "u_Textures";
				const std::string type = "sampler2D";
				s_OutputStream << "layout(binding = " << count <<") uniform " << type << " " << name <<"[32];\r\n";
				UpdateUniformBuffer(name, type);
			});
		InsertMap(s_FragmentFunctions, 30, [&]()
			{
				s_OutputStream << "vec4 GetTextureColor(vec2 texCoordinate)\r\n";
				s_OutputStream << "{\r\n";
				s_OutputStream << "\tvec4 texColor;\r\n";
				s_OutputStream << "\tswitch (int(v_TexIndex))\r\n";
				s_OutputStream << "\t{\r\n";
				s_OutputStream << "\t\tcase  0: texColor = texture(u_Textures[0], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase  1: texColor = texture(u_Textures[1], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase  2: texColor = texture(u_Textures[2], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase  3: texColor = texture(u_Textures[3], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase  4: texColor = texture(u_Textures[4], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase  5: texColor = texture(u_Textures[5], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase  6: texColor = texture(u_Textures[6], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase  7: texColor = texture(u_Textures[7], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase  8: texColor = texture(u_Textures[8], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase  9: texColor = texture(u_Textures[9], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 10: texColor = texture(u_Textures[10], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 11: texColor = texture(u_Textures[11], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 12: texColor = texture(u_Textures[12], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 13: texColor = texture(u_Textures[13], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 14: texColor = texture(u_Textures[14], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 15: texColor = texture(u_Textures[15], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 16: texColor = texture(u_Textures[16], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 17: texColor = texture(u_Textures[17], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 18: texColor = texture(u_Textures[18], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 19: texColor = texture(u_Textures[19], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 20: texColor = texture(u_Textures[20], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 21: texColor = texture(u_Textures[21], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 22: texColor = texture(u_Textures[22], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 23: texColor = texture(u_Textures[23], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 24: texColor = texture(u_Textures[24], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 25: texColor = texture(u_Textures[25], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 26: texColor = texture(u_Textures[26], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 27: texColor = texture(u_Textures[27], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 28: texColor = texture(u_Textures[28], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 29: texColor = texture(u_Textures[29], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 30: texColor = texture(u_Textures[30], texCoordinate); break;\r\n";
				s_OutputStream << "\t\tcase 31: texColor = texture(u_Textures[31], texCoordinate); break;\r\n";
				s_OutputStream << "\t}\r\n";
				s_OutputStream << "\treturn texColor;\r\n";
				s_OutputStream << "}\r\n";
			} );

		InsertMap(s_FragmentMain, 30, [&]()
			{
				s_OutputStream << "\tvec2 texCoordinate = Input.TexCoord;\r\n";
				s_OutputStream << "\ttexCoordinate = texCoordinate * Input.TilingFactor;\r\n";
				s_OutputStream << "\tvec4 texColor = GetTextureColor(texCoordinate);\r\n";
				s_OutputStream << "\to_Color *= texColor;\r\n";
			});

		InsertMap(s_FragmentMain, 100, [&]()
			{
				s_OutputStream << "\tif (o_Color.a <= 0.3)\r\n";
				s_OutputStream << "\t{\r\n";
				s_OutputStream << "\t\tdiscard;\r\n";
				s_OutputStream << "\t}\r\n";
			});
	}

	static void AddTextTextureOutput()
	{
		InsertMap(s_VertexInput, 70, [&](uint16_t count)
			{
				const std::string name = "a_TexCoord";
				const std::string type = "vec2";
				s_OutputStream << "layout(location = " << count << ") in " << type << " " << name << ";\r\n";
				UpdateInputBuffer(name, type);
			});

		InsertMap(s_VertexInput, 70, [&](uint16_t count)
			{
				const std::string name = "a_TexIndex";
				const std::string type = "float";
				s_OutputStream << "layout(location = " << count << ") in " << type << " " << name << ";\r\n";
				UpdateInputBuffer(name, type);
			});


		InsertMap(s_VertexToFragmentSmooth, 70, [&]()
			{
				s_OutputStream << "\tvec2 TexCoord;\r\n";
			});

		InsertMap(s_VertexToFragmentFlat, 70, [&](uint16_t count, std::string_view type)
			{
				s_OutputStream << "layout (location = " << count << ") " << type << " flat float v_TexIndex;\r\n";
			});

		InsertMap(s_VertexMain, 40, [&]()
			{
				s_OutputStream << "\tOutput.TexCoord = a_TexCoord;\r\n";
				s_OutputStream << "\tv_TexIndex = a_TexIndex;\r\n";
			});

		InsertMap(s_FragmentUniforms, 40, [&](uint16_t count)
			{
				const std::string name = "u_Textures";
				const std::string type = "sampler2D";
				s_OutputStream << "layout(binding = " << count << ") uniform " << type << " " << name << "[32];\r\n";
				UpdateUniformBuffer(name, type);
			});
		InsertMap(s_FragmentFunctions, 30, [&]()
			{
				s_OutputStream << "vec4 GetTextureColor(vec2 texCoordinate)\r\n";
				s_OutputStream << "{\r\n";
				s_OutputStream << "\tvec4 texColor;\r\n";
				s_OutputStream << "\tswitch (int(v_TexIndex))\r\n";
				s_OutputStream << "\t{\r\n";
				s_OutputStream << "\t\tcase  0: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[0], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase  1: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[1], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase  2: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[2], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase  3: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[3], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase  4: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[4], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase  5: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[5], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase  6: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[6], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase  7: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[7], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase  8: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[8], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase  9: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[9], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 10: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[10], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 11: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[11], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 12: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[12], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 13: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[13], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 14: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[14], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 15: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[15], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 16: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[16], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 17: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[17], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 18: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[18], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 19: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[19], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 20: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[20], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 21: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[21], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 22: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[22], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 23: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[23], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 24: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[24], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 25: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[25], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 26: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[26], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 27: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[27], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 28: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[28], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 29: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[29], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 30: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[30], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t\tcase 31: texColor = vec4(1.0, 1.0, 1.0, texture(u_Textures[31], texCoordinate).r); break;\r\n";
				s_OutputStream << "\t}\r\n";
				s_OutputStream << "\treturn texColor;\r\n";
				s_OutputStream << "}\r\n";
			});

		InsertMap(s_FragmentMain, 30, [&]()
			{
				s_OutputStream << "\tvec2 texCoordinate = Input.TexCoord;\r\n";
				s_OutputStream << "\tvec4 texColor = GetTextureColor(texCoordinate);\r\n";
				s_OutputStream << "\to_Color *= texColor;\r\n";
			});
	}

	//=================
	// Modification Feature
	//=================

	static void AddCircleShape()
	{
		InsertMap(s_VertexInput, 50, [&](uint16_t count)
			{
				const std::string name = "a_LocalPosition";
				const std::string type = "vec3";
				s_OutputStream << "layout(location = " << count << ") in " << type << " " << name << ";\r\n";
				UpdateInputBuffer(name, type);
			});
		InsertMap(s_VertexInput, 50, [&](uint16_t count)
			{
				const std::string name = "a_Thickness";
				const std::string type = "float";
				s_OutputStream << "layout(location = " << count << ") in " << type << " " << name << ";\r\n";
				UpdateInputBuffer(name, type);
			});
		InsertMap(s_VertexInput, 50, [&](uint16_t count)
			{
				const std::string name = "a_Fade";
				const std::string type = "float";
				s_OutputStream << "layout(location = " << count << ") in " << type << " " << name << ";\r\n";
				UpdateInputBuffer(name, type);
			});

		InsertMap(s_VertexToFragmentSmooth, 50, [&]()
			{
				s_OutputStream << "\tvec3 LocalPosition;\r\n";
			});
		InsertMap(s_VertexToFragmentSmooth, 50, [&]()
			{
				s_OutputStream << "\tfloat Thickness;\r\n";
			});
		InsertMap(s_VertexToFragmentSmooth, 50, [&]()
			{
				s_OutputStream << "\tfloat Fade;\r\n";
			});

		InsertMap(s_VertexMain, 40, [&]()
			{
				s_OutputStream << "\tOutput.LocalPosition = a_LocalPosition;\r\n";
				s_OutputStream << "\tOutput.Thickness = a_Thickness;\r\n";
				s_OutputStream << "\tOutput.Fade = a_Fade;\r\n";
			});

		InsertMap(s_FragmentFunctions, 50, [&]()
			{
				s_OutputStream << "float CalculateCircle(vec3 localPosition, float thickness, float fade)\r\n";
				s_OutputStream << "{\r\n";
				s_OutputStream << "\tfloat distance = 1.0 - length(localPosition);\r\n";
				s_OutputStream << "\tfloat circle = smoothstep(0.0, fade, distance);\r\n";
				s_OutputStream << "\tcircle *= smoothstep(thickness + fade, thickness, distance);\r\n";
				s_OutputStream << "\tif (circle < 0.01) { discard; }\r\n";
				s_OutputStream << "\treturn circle;\r\n";
				s_OutputStream << "}\r\n";
			});

		InsertMap(s_FragmentMain, 30, [&]()
			{
				s_OutputStream << "\tfloat circle = CalculateCircle(Input.LocalPosition, Input.Thickness, Input.Fade);\r\n";
			});

		InsertMap(s_FragmentMain, 80, [&]()
			{
				s_OutputStream << "\to_Color.a *= circle;\r\n";
			});

	}

	//=================
	// Other Features
	//=================

	static void AddProjectionMatrix()
	{
		InsertMap(s_VertexUniforms, 10, [&](uint16_t count)
			{
				const std::string name = "u_ViewProjection";
				const std::string type = "mat4";
				s_OutputStream << "layout(std140, binding = " << count << ") uniform Camera\r\n";
				s_OutputStream << "{\r\n";
				s_OutputStream << type << " " << name << ";\r\n";
				s_OutputStream << "};\r\n";
				UpdateUniformBuffer(name, type);
			});

		InsertMap(s_VertexMain, 70, [&]()
			{
				s_OutputStream << "\tgl_Position = u_ViewProjection * gl_Position;\r\n";
			});
	}

	static void AddEntityID()
	{
		InsertMap(s_VertexInput, 70, [&](uint16_t count)
			{
				const std::string name = "a_EntityID";
				const std::string type = "int";
				s_OutputStream << "layout(location = " << count << ") in " << type <<" " << name <<";\r\n";
				UpdateInputBuffer(name, type);
			});

		InsertMap(s_VertexToFragmentFlat, 30, [&](uint16_t count, std::string_view type)
			{
				s_OutputStream << "layout (location = " << count <<") " << type << " flat int v_EntityID;\r\n";
			});

		InsertMap(s_VertexMain, 70, [&]()
			{
				s_OutputStream << "\tv_EntityID = a_EntityID;\r\n";
			});

		InsertMap(s_FragmentOutput, 50, [&](uint16_t count)
			{
				s_OutputStream << "layout(location = " << count << ") out int o_EntityID;\r\n";
			});

		InsertMap(s_FragmentMain, 90, [&]()
			{
				s_OutputStream << "\to_EntityID = v_EntityID;\r\n";
			});
	}

	std::tuple<Shader::ShaderSource, InputBufferLayout, UniformBufferList> Shader::BuildShader(const ShaderSpecification& shaderSpec)
	{
		uint16_t countBuffer = 0;

		//=================
		// Reset Static Resources
		//=================

		// Clear static Output
		s_OutputStream.str("");
		s_InputBufferLayout.Clear();
		s_UniformBufferLayout.Clear();

		// Clear All Lambda Maps
		s_VertexStructs.clear();
		s_VertexInput.clear();
		s_VertexToFragmentSmooth.clear();
		s_VertexToFragmentFlat.clear();
		s_VertexUniforms.clear();
		s_VertexMain.clear();
		s_FragmentStructs.clear();
		s_FragmentFunctions.clear();
		s_FragmentOutput.clear();
		s_FragmentUniforms.clear();
		s_FragmentMain.clear();

		//=================
		// Set up Lambda Maps based on shaderSpec
		//=================

		// Core
		AddSimpleVertexOutput();
		AddSimpleFragmentOutput();

		if (shaderSpec.ColorInput == Shader::ColorInputType::FlatColor || shaderSpec.ColorInput == Shader::ColorInputType::VertexColor)
		{
			AddColorOutput();
		}
		if (shaderSpec.AddProjectionMatrix) { AddProjectionMatrix(); }
		if (shaderSpec.AddEntityID) { AddEntityID(); }
		if (shaderSpec.AddCircleShape) { AddCircleShape(); }
		if (shaderSpec.TextureInput == Shader::TextureInputType::ColorTexture) { AddTextureOutput(); }
		if (shaderSpec.TextureInput == Shader::TextureInputType::TextTexture) { AddTextTextureOutput(); }

		//=================
		// Start Building Shader
		//=================

		// ==== Vertex Shader ====
		s_OutputStream << "// Rendering Type: " << Shape::RenderingTypeToString(shaderSpec.RenderType) << "\r\n";
		s_OutputStream << "// Color Type: " << Shader::ColorInputTypeToString(shaderSpec.ColorInput) << "\r\n";
		s_OutputStream << "// Draw Outline: " << (shaderSpec.DrawOutline ? "true" : "false") << "\r\n";
		BeginShader("vertex");
		// Structs/Classes
		RunFunctions(s_VertexStructs);
		LineSpace();
		if (s_VertexToFragmentSmooth.size() > 0) { s_OutputStream << "struct VertexOutput\r\n"; }
		if (s_VertexToFragmentSmooth.size() > 0) { s_OutputStream << "{\r\n"; }
		countBuffer = RunFunctions(s_VertexToFragmentSmooth);
		if (s_VertexToFragmentSmooth.size() > 0) { s_OutputStream << "};\r\n"; }
		LineSpace();
		// Input
		RunFunctionsWithCount(s_VertexInput);
		RunFunctionsWithCount(s_VertexUniforms);
		// Output
		if (s_VertexToFragmentSmooth.size() > 0) { s_OutputStream << "layout(location = 0) out VertexOutput Output;\r\n"; }
		RunFunctionsWithTypeAndCount(s_VertexToFragmentFlat, "out", countBuffer);
		LineSpace();
		// Main
		BeginMain();
		RunFunctions(s_VertexMain);
		EndMain();
		LineSpace();
		LineSpace();

		// ==== Fragment Shader ====
		BeginShader("fragment");
		// Structs/Classes
		RunFunctions(s_FragmentStructs);
		LineSpace();
		if (s_VertexToFragmentSmooth.size() > 0) { s_OutputStream << "struct VertexOutput\r\n"; }
		if (s_VertexToFragmentSmooth.size() > 0) { s_OutputStream << "{\r\n"; }
		RunFunctions(s_VertexToFragmentSmooth);
		if (s_VertexToFragmentSmooth.size() > 0) { s_OutputStream << "};\r\n"; }
		LineSpace();
		// Input
		if (s_VertexToFragmentSmooth.size() > 0) { s_OutputStream << "layout(location = 0) in VertexOutput Input;\r\n"; }
		RunFunctionsWithTypeAndCount(s_VertexToFragmentFlat, "in", countBuffer);
		RunFunctionsWithCount(s_FragmentUniforms);
		// Output
		RunFunctionsWithCount(s_FragmentOutput);
		LineSpace();
		// Functions
		RunFunctions(s_FragmentFunctions);
		LineSpace();
		// Main
		BeginMain();
		RunFunctions(s_FragmentMain);
		EndMain();

		return { s_OutputStream.str(), s_InputBufferLayout, s_UniformBufferLayout };
	}
	void Shader::SetSpecification(const Shader::ShaderSpecification& shaderSpec)
	{
		m_ShaderSpecification = shaderSpec;
		FillRenderFunctionList();
	}

	void Shader::SetInputLayout(const InputBufferLayout& shaderInputLayout)
	{
		m_InputBufferLayout = shaderInputLayout;
		m_VertexArray = VertexArray::Create();

		auto quadVertexBuffer = VertexBuffer::Create(Renderer::s_MaxVertexBufferSize);
		quadVertexBuffer->SetLayout(m_InputBufferLayout);
		m_VertexArray->AddVertexBuffer(quadVertexBuffer);
	}



	void Shader::FillRenderFunctionList()
	{

		if (m_ShaderSpecification.RenderType == Shape::RenderingType::DrawLine)
		{
			m_DrawFunctions.push_back(Renderer::DrawBufferLine);
		}

		if (m_ShaderSpecification.RenderType == Shape::RenderingType::DrawLine ||
			m_ShaderSpecification.TextureInput == Shader::TextureInputType::TextTexture)
		{
			m_FillDataPerVertex.push_back(Renderer::FillWorldPositionNoTransform);
			
		}
		else
		{
			m_FillDataPerVertex.push_back(Renderer::FillWorldPosition);
		}

		if (m_ShaderSpecification.AddEntityID)
		{
			m_FillDataInScene.push_back(Scene::FillEntityID);
		}
		if (m_ShaderSpecification.TextureInput == Shader::TextureInputType::ColorTexture ||
			m_ShaderSpecification.TextureInput == Shader::TextureInputType::TextTexture)
		{
			m_FillDataPerObject.push_back( Renderer::FillTextureIndex);
			m_FillDataPerVertex.push_back(Renderer::FillTextureCoordinate);
			m_SubmitUniforms.push_back(Renderer::FillTextureUniform);
		}

		if (m_ShaderSpecification.ColorInput == Shader::ColorInputType::VertexColor)
		{
			m_FillDataPerVertex.push_back(Renderer::FillVertexColor);
		}

		if (m_ShaderSpecification.AddCircleShape)
		{
			m_FillDataPerVertex.push_back(Renderer::FillLocalPosition);
		}

		if (m_ShaderSpecification.RenderType == Shape::RenderingType::DrawIndex)
		{
			m_FillDataPerObject.push_back(Renderer::FillIndicesData);
			m_DrawFunctions.push_back(Renderer::DrawBufferIndices);
		}

		if (m_ShaderSpecification.RenderType == Shape::RenderingType::DrawTriangle)
		{
			m_DrawFunctions.push_back(Renderer::DrawBufferTriangles);
		}

		KG_CORE_ASSERT(sizeof(Shader::ShaderSpecification) == sizeof(uint8_t) * 20, "Please Update Render section in Shader Code! It looks like you updated the shaderspecification");
	}
}
