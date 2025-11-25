#pragma once

#include "Kargono/Core/Base.h"
#include "Modules/RuntimeUI/Assets/Font.h"

#include <vector>
#include <filesystem>

namespace Kargono::RuntimeUI
{
	class FontContext
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		FontContext() = default;
		~FontContext() = default;
	public:
		//==============================
		// LifeCycle Functions
		//==============================
		void Init();
		void Terminate();
	public:
		void SetID(uint32_t id);
	public:
		//==============================
		// Public Fields
		//==============================
		Rendering::RendererInputSpec m_TextInputSpec{};
		Ref<std::vector<Math::vec3>> m_Vertices;
		Ref<std::vector<Math::vec2>> m_TexCoordinates;
	};

	class FontService // TODO: REMOVE EWWWWWWW
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		FontService() = delete;
		~FontService() = delete;
	public:
		//==============================
		// Getters/Setters
		//==============================
		static FontContext& GetActiveContext() { return s_Font; }
	private:
		//==============================
		// Internal Fields
		//==============================
		static inline RuntimeUI::FontContext s_Font{};
	};
}