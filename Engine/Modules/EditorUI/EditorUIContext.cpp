#include "kgpch.h"

#include "Modules/EditorUI/EditorUIContext.h"
#include "Modules/Core/Engine.h"
#include "Kargono/Utility/Regex.h"
#include "Kargono/Utility/Operations.h"
#include "Modules/Input/InputService.h"
#include "Modules/InputMap/InputMap.h"
#include "Kargono/Utility/FileDialogs.h"
#include "Modules/Rendering/Texture.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/ProjectData/ColorPalette.h"
#include "Modules/Assets/AssetService.h"

#include "Kargono/Utility/DebugGlobals.h"

#include "Modules/EditorUI/ExternalAPI/ImGuiBackendAPI.h"
#include "API/Platform/GlfwAPI.h"
#include "API/Platform/gladAPI.h"
#include "Modules/EditorUI/ExternalAPI/ImGuiNotifyAPI.h"
#include "Modules/EditorUI/ExternalAPI/ImPlotAPI.h"

namespace Kargono::EditorUI
{

	const char* EditorUIContext::GetHoveredWindowName()
	{
		ImGuiContext* context = ImGui::GetCurrentContext();
		if (context != nullptr)
		{
			// Get window pointer and ensure it is valid
			ImGuiWindow* hoveredWindow = context->HoveredWindow;
			if (hoveredWindow != nullptr) 
			{	
				// Get the name of the hovered window
				return hoveredWindow->Name;
			}
		}
		return nullptr;
	}
	void EditorUIContext::RenderInfoNotification(const char* text, int delayMS)
	{
		ImGuiToast toast{ ImGuiToastType::Info, delayMS, text };
		toast.setTitle("Kargono Editor Information");
		ImGui::InsertNotification(toast);
	}
	void EditorUIContext::RenderWarningNotification(const char* text, int delayMS)
	{
		ImGuiToast toast{ ImGuiToastType::Warning, delayMS, text };
		toast.setTitle("Kargono Editor Warning");
		ImGui::InsertNotification(toast);
	}

	void EditorUIContext::RenderCriticalNotification(const char* text, int delayMS)
	{
		ImGuiToast toast{ ImGuiToastType::Error, delayMS, text };
		toast.setTitle("Kargono Editor Critical");
		ImGui::InsertNotification(toast);
	}

	void EditorUIContext::CalculateViewportDimensions(Math::vec2 screenViewportBounds[2], ViewportData& viewportData, bool& viewportFocused, bool& viewportHovered, const Math::uvec2& aspectRatio)
	{
		// Get current cursor position and GLFW viewport size
		ImVec2 windowScreenOffset = ImGui::GetWindowPos();
		Math::vec2 localViewportBounds[2];
		viewportFocused = ImGui::IsWindowFocused();
		viewportHovered = ImGui::IsWindowHovered();

		// Adjust viewport size based on current aspect ratio
		ImVec2 windowSize = ImGui::GetContentRegionAvail();
		ImVec2 cursorPosition = ImGui::GetCursorPos();
		if (aspectRatio.x > aspectRatio.y && ((windowSize.x / aspectRatio.x) * aspectRatio.y) < windowSize.y)
		{
			viewportData.m_Width = (uint32_t)windowSize.x;
			viewportData.m_Height = (uint32_t)(windowSize.x / aspectRatio.x) * aspectRatio.y;
		}
		else
		{
			viewportData.m_Width = (uint32_t)(windowSize.y / aspectRatio.y) * aspectRatio.x;
			viewportData.m_Height = (uint32_t)windowSize.y;
		}

		localViewportBounds[0] = { cursorPosition.x + (windowSize.x - (float)viewportData.m_Width) * 0.5f, cursorPosition.y + (windowSize.y - (float)viewportData.m_Height) * 0.5f };
		localViewportBounds[1] = { localViewportBounds[0].x + (float)viewportData.m_Width,  localViewportBounds[0].y + (float)viewportData.m_Height };
		screenViewportBounds[0] = { localViewportBounds[0].x + windowScreenOffset.x, localViewportBounds[0].y + windowScreenOffset.y };
		screenViewportBounds[1] = { screenViewportBounds[0].x + (float)viewportData.m_Width, screenViewportBounds[0].y + viewportData.m_Height };
		ImGui::SetCursorPos(ImVec2(localViewportBounds[0].x, localViewportBounds[0].y));
	}

	void EditorUIContext::SkipMouseIconChange()
	{
		s_BlockMouseIconChange = true;
	}

	void EditorUIContext::RenderNotifications()
	{
		// Notifications style setup
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f); // Disable round borders
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f); // Disable borders

		// Notifications color setup
		ImGui::PushStyleColor(ImGuiCol_WindowBg, m_ConfigColors.m_DarkBackgroundColor); // Background color

		// Main rendering function
		ImGui::RenderNotifications();

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(1);
	}

	bool EditorUIContext::RenderColorPicker(const char* name, ImVec4& mainColor)
	{
		bool returnValue{ false };
		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const float square_sz = ImGui::GetFrameHeight();
		constexpr size_t k_InvalidPaletteID{ std::numeric_limits<size_t>().max() };

		static std::vector<std::pair<std::string, ProjectData::ColorPalette>> s_ActivePalettes;
		static std::string s_ActivePaletteLabel;
		static size_t s_ActivePaletteID;
		
		if (DrawColorPickerButtonInternal(name, mainColor, ImVec2(18.0f, 18.0f)))
		{
			// Store current color and open a picker
			g.ColorPickerRef = mainColor;
			ImGui::OpenPopup("##ColorPickerPopup");
			ImGui::SetNextWindowPos(g.LastItemData.Rect.GetBL() + ImVec2(0.0f, style.ItemSpacing.y));

			// Reset active palette
			s_ActivePaletteLabel = "None";
			s_ActivePaletteID = k_InvalidPaletteID;

			// Revalidate the active color palettes
			s_ActivePalettes.clear();

			// Add all color palettes from the active registry
			for (auto& [handle, assetInfo] : Assets::AssetService::GetColorPaletteRegistry())
			{
				Ref<ProjectData::ColorPalette> palette = Assets::AssetService::GetColorPalette(handle);
				KG_ASSERT(palette);
				s_ActivePalettes.push_back
				({
					assetInfo.Data.GetSpecificMetaData<Assets::ColorPaletteMetaData>()->Name,
					*palette
				});
			}
		}

		ImGui::SetNextWindowSize(ImVec2(290.0f, 260.0f));
		if (ImGui::BeginPopup("##ColorPickerPopup", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			if (g.CurrentWindow->BeginCount == 1)
			{
				BeginTabBar("##ColorPickerTabBar");

				if (BeginTabItem("Picker"))
				{
					ImGui::SetNextItemWidth(square_sz * 12.0f); // Use 256 + bar sizes?
					returnValue = DrawColorPickerPopupContents("##picker", (float*)&mainColor, &g.ColorPickerRef.x);
					EndTabItem();
				}
				if (BeginTabItem("Palette"))
				{
					ImGui::PushStyleColor(ImGuiCol_FrameBg, EditorUI::EditorUIContext::m_ConfigColors.m_BackgroundColor);
					ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, EditorUI::EditorUIContext::m_ConfigColors.m_HoveredColor);
					ImGui::PushStyleColor(ImGuiCol_FrameBgActive, EditorUI::EditorUIContext::m_ConfigColors.m_ActiveColor);

					if (ImGui::BeginCombo("##ChoosePalette", s_ActivePaletteLabel.c_str(), ImGuiComboFlags_NoArrowButton))
					{
						if (ImGui::Selectable("None"))
						{
							s_ActivePaletteLabel = "None";
							s_ActivePaletteID = k_InvalidPaletteID;
						}
						size_t iteration{ 0 };
						for (auto& [name, palette] : s_ActivePalettes)
						{
							if (ImGui::Selectable(name.c_str()))
							{
								s_ActivePaletteLabel = name;
								s_ActivePaletteID = iteration;
								break;
							}
							iteration++;
						}
						ImGui::EndCombo();
					}

					ImGui::PopStyleColor(3);

					// Get the current palette
					if (s_ActivePaletteID != k_InvalidPaletteID)
					{
						KG_ASSERT(s_ActivePaletteID < s_ActivePalettes.size());

						ImGui::BeginChild("##ChoosePaletteGrid", ImVec2(0, 0), false,
							ImGuiWindowFlags_AlwaysVerticalScrollbar);
						uint32_t widgetCount{ 0 };
						static WidgetID persistantID{ WidgetCounterService::GetActiveContext().GetID()};
						FixedString<16> id{ "##" };
						id.AppendInteger(persistantID);

						float iconSize = 60.0f;
						float cellPadding = 15.0f;

						// Calculate grid cell count using provided spec sizes
						float cellSize = iconSize + cellPadding;
						float panelWidth = ImGui::GetContentRegionAvail().x;
						int32_t columnCount = (int32_t)(panelWidth / cellSize);
						columnCount = columnCount > 0 ? columnCount : 1;

						// Get the active color palette
						ProjectData::ColorPalette& activePalette = s_ActivePalettes[s_ActivePaletteID].second;
						Spacing(SpacingAmount::Small);
						// Start drawing columns
						ImGui::Columns(columnCount, id.CString(), false);
						ImGui::PushStyleColor(ImGuiCol_Button, k_PureEmpty);
						size_t colorIteration{ 0 };
						for (auto& [name, color] : activePalette.m_Colors)
						{
							// Display grid icon
							ImGui::PushID(std::to_string(colorIteration).c_str());

							ImVec4 colorPickerValue{ Utility::MathVec4ToImVec4(Utility::HexToRGBA(color))};
							ImGuiColorEditFlags misc_flags = ImGuiColorEditFlags_NoInputs |
								ImGuiColorEditFlags_NoLabel |
								ImGuiColorEditFlags_AlphaPreviewHalf |
								ImGuiColorEditFlags_NoSidePreview |
								ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip;
							if (ImGui::ColorButton(("##" + std::to_string(colorIteration)).c_str(),
								colorPickerValue,
								misc_flags, ImVec2(cellSize, cellSize)))
							{
								mainColor = colorPickerValue;
								returnValue = true;
							}

							// Draw label for each cell
							ImGui::TextWrapped(name.CString());

							// Reset cell data for next call
							ImGui::NextColumn();
							ImGui::PopID();
							colorIteration++;
						}

						// End drawing columns
						ImGui::PopStyleColor();
						ImGui::Columns(1);

						ImGui::EndChild();
					}
					EndTabItem();
				}
				EndTabBar();
			}
			ImGui::EndPopup();
		}
		return returnValue;
	}

	bool EditorUIContext::DrawColorPickerButtonInternal(const char* desc_id, const ImVec4& col, const ImVec2& size_arg)
	{
		// Get the current window context and skip if indicated
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		// Get relevant dimensions information from context
		ImGuiContext& g = *GImGui;
		const ImGuiID id = window->GetID(desc_id);
		const float default_size = ImGui::GetFrameHeight();
		const ImVec2 size(size_arg.x == 0.0f ? default_size : size_arg.x, size_arg.y == 0.0f ? default_size : size_arg.y);
		const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
		ImGui::ItemSize(bb, (size.y >= default_size) ? g.Style.FramePadding.y : 0.0f);
		if (!ImGui::ItemAdd(bb, id))
			return false;

		// Query the state of interaction with the button
		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

		// Convert from HSV to RGBA if necessary
		ImVec4 col_rgb = col;

		ImVec4 col_rgb_without_alpha(col_rgb.x, col_rgb.y, col_rgb.z, 1.0f);
		float grid_step = ImMin(size.x, size.y) / 2.99f;
		float rounding = ImMin(g.Style.FrameRounding, grid_step * 0.5f);
		ImRect bb_inner = bb;
		float off = 0.0f;
		off = -0.75f; // The border (using Col_FrameBg) tends to look off when color is near-opaque and rounding is enabled. This offset seemed like a good middle ground to reduce those artifacts.
		bb_inner.Expand(off);
		

		// Draw the actual color display
		if (col_rgb.w < 1.0f)
		{
			float mid_x = IM_ROUND((bb_inner.Min.x + bb_inner.Max.x) * 0.5f);
			ImGui::RenderColorRectWithAlphaCheckerboard(window->DrawList, ImVec2(bb_inner.Min.x + grid_step, bb_inner.Min.y), bb_inner.Max, ImGui::GetColorU32(col_rgb), grid_step, ImVec2(-grid_step + off, off), rounding, ImDrawFlags_RoundCornersRight);
			window->DrawList->AddRectFilled(bb_inner.Min, ImVec2(mid_x, bb_inner.Max.y), ImGui::GetColorU32(col_rgb_without_alpha), rounding, ImDrawFlags_RoundCornersLeft);
		}
		else
		{
			// Because GetColorU32() multiplies by the global style Alpha and we don't want to display a checkerboard if the source code had no alpha
			ImVec4 col_source =  col_rgb;
			if (col_source.w < 1.0f)
				ImGui::RenderColorRectWithAlphaCheckerboard(window->DrawList, bb_inner.Min, bb_inner.Max, ImGui::GetColorU32(col_source), grid_step, ImVec2(off, off), rounding);
			else
				window->DrawList->AddRectFilled(bb_inner.Min, bb_inner.Max, ImGui::GetColorU32(col_source), rounding);
		}

		// Draw a frame border
		ImGui::RenderNavHighlight(bb, id);
		if (g.Style.FrameBorderSize > 0.0f)
			ImGui::RenderFrameBorder(bb.Min, bb.Max, rounding);
		else
			window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), rounding); // Color button are often in need of some sort of border
		

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::TextColored(EditorUI::EditorUIContext::m_ConfigColors.m_HighlightColor1, "Open Color Picker");
			ImGui::EndTooltip();
		}

		return pressed;
	}

	bool EditorUIContext::DrawColorPickerPopupContents(const char* label, float col[4], const float* ref_col)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImGuiColorEditFlags flags = ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf;
		if (window->SkipItems)
			return false;

		ImDrawList* draw_list = window->DrawList;
		ImGuiStyle& style = g.Style;
		ImGuiIO& io = g.IO;

		const float width = ImGui::CalcItemWidth();
		g.NextItemData.ClearFlags();

		ImGui::PushID(label);
		ImGui::BeginGroup();

		if (!(flags & ImGuiColorEditFlags_NoSidePreview))
			flags |= ImGuiColorEditFlags_NoSmallPreview;

		// Context menu: display and store options.
		if (!(flags & ImGuiColorEditFlags_NoOptions))
			ImGui::ColorPickerOptionsPopup(col, flags);

		// Read stored options
		if (!(flags & ImGuiColorEditFlags_PickerMask_))
			flags |= ((g.ColorEditOptions & ImGuiColorEditFlags_PickerMask_) ? g.ColorEditOptions : ImGuiColorEditFlags_DefaultOptions_) & ImGuiColorEditFlags_PickerMask_;
		if (!(flags & ImGuiColorEditFlags_InputMask_))
			flags |= ((g.ColorEditOptions & ImGuiColorEditFlags_InputMask_) ? g.ColorEditOptions : ImGuiColorEditFlags_DefaultOptions_) & ImGuiColorEditFlags_InputMask_;
		IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_PickerMask_)); // Check that only 1 is selected
		IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_InputMask_));  // Check that only 1 is selected
		if (!(flags & ImGuiColorEditFlags_NoOptions))
			flags |= (g.ColorEditOptions & ImGuiColorEditFlags_AlphaBar);

		// Setup
		int components = (flags & ImGuiColorEditFlags_NoAlpha) ? 3 : 4;
		bool alpha_bar = (flags & ImGuiColorEditFlags_AlphaBar) && !(flags & ImGuiColorEditFlags_NoAlpha);
		ImVec2 picker_pos = window->DC.CursorPos;
		float square_sz = ImGui::GetFrameHeight();
		float bars_width = square_sz; // Arbitrary smallish width of Hue/Alpha picking bars
		float sv_picker_size = ImMax(bars_width * 1, width - (alpha_bar ? 2 : 1) * (bars_width + style.ItemInnerSpacing.x)); // Saturation/Value picking box
		float bar0_pos_x = picker_pos.x + sv_picker_size + style.ItemInnerSpacing.x;
		float bar1_pos_x = bar0_pos_x + bars_width + style.ItemInnerSpacing.x;
		float bars_triangles_half_sz = IM_FLOOR(bars_width * 0.20f);

		float backup_initial_col[4];
		memcpy(backup_initial_col, col, components * sizeof(float));

		float wheel_thickness = sv_picker_size * 0.08f;
		float wheel_r_outer = sv_picker_size * 0.50f;
		float wheel_r_inner = wheel_r_outer - wheel_thickness;
		ImVec2 wheel_center(picker_pos.x + (sv_picker_size + bars_width) * 0.5f, picker_pos.y + sv_picker_size * 0.5f);

		// Note: the triangle is displayed rotated with triangle_pa pointing to Hue, but most coordinates stays unrotated for logic.
		float triangle_r = wheel_r_inner - (int)(sv_picker_size * 0.027f);
		ImVec2 triangle_pa = ImVec2(triangle_r, 0.0f); // Hue point.
		ImVec2 triangle_pb = ImVec2(triangle_r * -0.5f, triangle_r * -0.866025f); // Black point.
		ImVec2 triangle_pc = ImVec2(triangle_r * -0.5f, triangle_r * +0.866025f); // White point.

		float H = col[0], S = col[1], V = col[2];
		float R = col[0], G = col[1], B = col[2];
		
		// Hue is lost when converting from greyscale rgb (saturation=0). Restore it.
		ImGui::ColorConvertRGBtoHSV(R, G, B, H, S, V);
		ImGui::ColorEditRestoreHS(col, &H, &S, &V);
		

		bool value_changed = false, value_changed_h = false, value_changed_sv = false;

		ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
		// Draw color hue bar
		ImGui::InvisibleButton("sv", ImVec2(sv_picker_size, sv_picker_size));
		if (ImGui::IsItemActive())
		{
			S = ImSaturate((io.MousePos.x - picker_pos.x) / (sv_picker_size - 1));
			V = 1.0f - ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1));

			// Greatly reduces hue jitter and reset to 0 when hue == 255 and color is rapidly modified using SV square.
			if (g.ColorEditLastColor == ImGui::ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 0)))
				H = g.ColorEditLastHue;
			value_changed = value_changed_sv = true;
		}
		if (!(flags & ImGuiColorEditFlags_NoOptions))
			ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);

		// Hue bar logic
		ImGui::SetCursorScreenPos(ImVec2(bar0_pos_x, picker_pos.y));
		ImGui::InvisibleButton("hue", ImVec2(bars_width, sv_picker_size));
		if (ImGui::IsItemActive())
		{
			H = ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1));
			value_changed = value_changed_h = true;
		}

		// Alpha bar logic
		if (alpha_bar)
		{
			ImGui::SetCursorScreenPos(ImVec2(bar1_pos_x, picker_pos.y));
			ImGui::InvisibleButton("alpha", ImVec2(bars_width, sv_picker_size));
			if (ImGui::IsItemActive())
			{
				col[3] = 1.0f - ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1));
				value_changed = true;
			}
		}
		ImGui::PopItemFlag(); // ImGuiItemFlags_NoNav

		// Convert back color to RGB
		if (value_changed_h || value_changed_sv)
		{
			ImGui::ColorConvertHSVtoRGB(H, S, V, col[0], col[1], col[2]);
			g.ColorEditLastHue = H;
			g.ColorEditLastSat = S;
			g.ColorEditLastColor = ImGui::ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 0));
		}

		// R,G,B and H,S,V slider color editor
		bool value_changed_fix_hue_wrap = false;
		if ((flags & ImGuiColorEditFlags_NoInputs) == 0)
		{
			ImGui::PushItemWidth((alpha_bar ? bar1_pos_x : bar0_pos_x) + bars_width - picker_pos.x);
			ImGuiColorEditFlags sub_flags_to_forward = ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_InputMask_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf;
			ImGuiColorEditFlags sub_flags = (flags & sub_flags_to_forward) | ImGuiColorEditFlags_NoPicker;
			if (flags & ImGuiColorEditFlags_DisplayRGB || (flags & ImGuiColorEditFlags_DisplayMask_) == 0)
				if (ImGui::ColorEdit4("##rgb", col, sub_flags | ImGuiColorEditFlags_DisplayRGB))
				{
					// FIXME: Hackily differentiating using the DragInt (ActiveId != 0 && !ActiveIdAllowOverlap) vs. using the InputText or DropTarget.
					// For the later we don't want to run the hue-wrap canceling code. If you are well versed in HSV picker please provide your input! (See #2050)
					value_changed_fix_hue_wrap = (g.ActiveId != 0 && !g.ActiveIdAllowOverlap);
					value_changed = true;
				}
			if (flags & ImGuiColorEditFlags_DisplayHSV || (flags & ImGuiColorEditFlags_DisplayMask_) == 0)
				value_changed |= ImGui::ColorEdit4("##hsv", col, sub_flags | ImGuiColorEditFlags_DisplayHSV);
			if (flags & ImGuiColorEditFlags_DisplayHex || (flags & ImGuiColorEditFlags_DisplayMask_) == 0)
				value_changed |= ImGui::ColorEdit4("##hex", col, sub_flags | ImGuiColorEditFlags_DisplayHex);
			ImGui::PopItemWidth();
		}


		// Try to cancel hue wrap (after ColorEdit4 call), if any
		if (value_changed_fix_hue_wrap && (flags & ImGuiColorEditFlags_InputRGB))
		{
			float new_H, new_S, new_V;
			ImGui::ColorConvertRGBtoHSV(col[0], col[1], col[2], new_H, new_S, new_V);
			if (new_H <= 0 && H > 0)
			{
				if (new_V <= 0 && V != new_V)
					ImGui::ColorConvertHSVtoRGB(H, S, new_V <= 0 ? V * 0.5f : new_V, col[0], col[1], col[2]);
				else if (new_S <= 0)
					ImGui::ColorConvertHSVtoRGB(H, new_S <= 0 ? S * 0.5f : new_S, new_V, col[0], col[1], col[2]);
			}
		}

		if (value_changed)
		{
			R = col[0];
			G = col[1];
			B = col[2];
			ImGui::ColorConvertRGBtoHSV(R, G, B, H, S, V);
			ImGui::ColorEditRestoreHS(col, &H, &S, &V);   // Fix local Hue as display below will use it immediately.
		}

		const int style_alpha8 = IM_F32_TO_INT8_SAT(style.Alpha);
		const ImU32 col_black = IM_COL32(0, 0, 0, style_alpha8);
		const ImU32 col_white = IM_COL32(255, 255, 255, style_alpha8);
		const ImU32 col_midgrey = IM_COL32(128, 128, 128, style_alpha8);
		const ImU32 col_hues[6 + 1] = { IM_COL32(255,0,0,style_alpha8), IM_COL32(255,255,0,style_alpha8), IM_COL32(0,255,0,style_alpha8), IM_COL32(0,255,255,style_alpha8), IM_COL32(0,0,255,style_alpha8), IM_COL32(255,0,255,style_alpha8), IM_COL32(255,0,0,style_alpha8) };

		ImVec4 hue_color_f(1, 1, 1, style.Alpha); ImGui::ColorConvertHSVtoRGB(H, 1, 1, hue_color_f.x, hue_color_f.y, hue_color_f.z);
		ImU32 hue_color32 = ImGui::ColorConvertFloat4ToU32(hue_color_f);
		ImU32 user_col32_striped_of_alpha = ImGui::ColorConvertFloat4ToU32(ImVec4(R, G, B, style.Alpha)); // Important: this is still including the main rendering/style alpha!!

		ImVec2 sv_cursor_pos;
		// Render SV Square
		draw_list->AddRectFilledMultiColor(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size), col_white, hue_color32, hue_color32, col_white);
		draw_list->AddRectFilledMultiColor(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size), 0, 0, col_black, col_black);
		ImGui::RenderFrameBorder(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size), 0.0f);
		sv_cursor_pos.x = ImClamp(IM_ROUND(picker_pos.x + ImSaturate(S) * sv_picker_size), picker_pos.x + 2, picker_pos.x + sv_picker_size - 2); // Sneakily prevent the circle to stick out too much
		sv_cursor_pos.y = ImClamp(IM_ROUND(picker_pos.y + ImSaturate(1 - V) * sv_picker_size), picker_pos.y + 2, picker_pos.y + sv_picker_size - 2);

		// Render Hue Bar
		for (int i = 0; i < 6; ++i)
			draw_list->AddRectFilledMultiColor(ImVec2(bar0_pos_x, picker_pos.y + i * (sv_picker_size / 6)), ImVec2(bar0_pos_x + bars_width, picker_pos.y + (i + 1) * (sv_picker_size / 6)), col_hues[i], col_hues[i], col_hues[i + 1], col_hues[i + 1]);
		float bar0_line_y = IM_ROUND(picker_pos.y + H * sv_picker_size);
		ImGui::RenderFrameBorder(ImVec2(bar0_pos_x, picker_pos.y), ImVec2(bar0_pos_x + bars_width, picker_pos.y + sv_picker_size), 0.0f);
		ImGui::RenderArrowsForVerticalBar(draw_list, ImVec2(bar0_pos_x - 1, bar0_line_y), ImVec2(bars_triangles_half_sz + 1, bars_triangles_half_sz), bars_width + 2.0f, style.Alpha);
		

		// Render cursor/preview circle (clamp S/V within 0..1 range because floating points colors may lead HSV values to be out of range)
		float sv_cursor_rad = value_changed_sv ? 10.0f : 6.0f;
		draw_list->AddCircleFilled(sv_cursor_pos, sv_cursor_rad, user_col32_striped_of_alpha, 12);
		draw_list->AddCircle(sv_cursor_pos, sv_cursor_rad + 1, col_midgrey, 12);
		draw_list->AddCircle(sv_cursor_pos, sv_cursor_rad, col_white, 12);

		// Render alpha bar
		if (alpha_bar)
		{
			float alpha = ImSaturate(col[3]);
			ImRect bar1_bb(bar1_pos_x, picker_pos.y, bar1_pos_x + bars_width, picker_pos.y + sv_picker_size);
			ImGui::RenderColorRectWithAlphaCheckerboard(draw_list, bar1_bb.Min, bar1_bb.Max, 0, bar1_bb.GetWidth() / 2.0f, ImVec2(0.0f, 0.0f));
			draw_list->AddRectFilledMultiColor(bar1_bb.Min, bar1_bb.Max, user_col32_striped_of_alpha, user_col32_striped_of_alpha, user_col32_striped_of_alpha & ~IM_COL32_A_MASK, user_col32_striped_of_alpha & ~IM_COL32_A_MASK);
			float bar1_line_y = IM_ROUND(picker_pos.y + (1.0f - alpha) * sv_picker_size);
			ImGui::RenderFrameBorder(bar1_bb.Min, bar1_bb.Max, 0.0f);
			ImGui::RenderArrowsForVerticalBar(draw_list, ImVec2(bar1_pos_x - 1, bar1_line_y), ImVec2(bars_triangles_half_sz + 1, bars_triangles_half_sz), bars_width + 2.0f, style.Alpha);
		}

		ImGui::EndGroup();

		if (value_changed && memcmp(backup_initial_col, col, components * sizeof(float)) == 0)
			value_changed = false;
		if (value_changed && g.LastItemData.ID != 0) // In case of ID collision, the second EndGroup() won't catch g.ActiveId
			ImGui::MarkItemEdited(g.LastItemData.ID);

		ImGui::PopID();

		return value_changed;
	}

	void EditorUIContext::Init()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		API::InitImPlot();
		ImGuiIO& io = ImGui::GetIO();
		io.IniFilename = NULL;
		ImGui::LoadIniSettingsFromDisk("./Resources/EditorConfig.ini");
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		m_ConfigFonts.LoadFonts(io);

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		style.WindowMenuButtonPosition = -1;
		style.WindowPadding = { 7.0f, 4.0f };
		style.ItemSpacing = { 8.0f, 6.0f };
		style.WindowMinSize.x = 420.0f;

		ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float | 
			ImGuiColorEditFlags_DisplayHSV | 
			ImGuiColorEditFlags_PickerHueBar | 
			ImGuiColorEditFlags_NoInputs |
			ImGuiColorEditFlags_NoSidePreview |
			ImGuiColorEditFlags_AlphaBar);

		m_ConfigColors.SetColors();

		// Setup Platform/Renderer backends
		Engine& core = EngineService::GetActiveEngine();
		GLFWwindow* window = static_cast<GLFWwindow*>(core.GetWindow().GetNativeWindow());
		KG_ASSERT(window, "No window active when initializing EditorUI");
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

		// Initialize toast notification API
		io.Fonts->AddFontDefault();
		float baseFontSize = 16.0f;
		float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly
		static constexpr ImWchar iconsRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
		ImFontConfig iconsConfig;
		iconsConfig.MergeMode = true;
		iconsConfig.PixelSnapH = true;
		iconsConfig.GlyphMinAdvanceX = iconFontSize;
		io.Fonts->AddFontFromMemoryCompressedTTF(fa_solid_900_compressed_data, fa_solid_900_compressed_size, iconFontSize, &iconsConfig, iconsRanges);
		ImGui::InitNotificationSystem();

		const std::filesystem::path& workingDir
		{ 
			EngineService::GetActiveEngine().GetConfig().m_WorkingDirectory 
		};
		
		m_GenIcons.LoadIcons(workingDir);
		m_SceneIcons.LoadIcons(workingDir);
		m_ViewportIcons.LoadIcons(workingDir);
		m_RuntimeUIIcons.LoadIcons(workingDir);
		m_ScriptingIcons.LoadIcons(workingDir);
		m_ContentBrowserIcons.LoadIcons(workingDir);

		m_UIPresets.CreatePresets();

		s_Running = true;

		KG_VERIFY(s_Running && ImGui::GetCurrentContext(), "Editor UI Initiated")
	}

	void EditorUIContext::Terminate()
	{
		if (s_Running)
		{
			API::TerminateImPlot();
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
			s_Running = false;
		}

		// Icons
		m_ViewportIcons = {};
		m_GenIcons = {};
		m_RuntimeUIIcons = {};
		m_ScriptingIcons = {};
		m_SceneIcons = {};
		m_ContentBrowserIcons = {};

		m_UIPresets.m_SmallEditButton = {};
		m_UIPresets.m_SmallExpandButton = {};
		m_UIPresets.m_MediumOptionsButton = {};
		m_UIPresets.m_SmallCheckboxButton = {};
		m_UIPresets.m_SmallCheckboxDisabledButton = {};
		m_UIPresets.m_SmallLinkButton = {};
		m_UIPresets.m_LargeDeleteButton = {};
		m_UIPresets.m_LargeCancelButton = {};
		m_UIPresets.m_LargeConfirmButton = {};
		m_UIPresets.m_LargeSearchButton = {};

		m_UIPresets.m_TableEditButton = {};
		m_UIPresets.m_TableLinkButton = {};
		m_UIPresets.m_ListExpandButton = {};

		KG_VERIFY(!s_Running && !ImGui::GetCurrentContext(), "Editor UI Terminated")
	}

	void EditorUIContext::StartRendering()
	{
		ImGuiIO& io = ImGui::GetIO();
		if (s_BlockMouseIconChange)
		{
			io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
		}
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		if (s_BlockMouseIconChange)
		{
			io.ConfigFlags &= (~ImGuiConfigFlags_NoMouseCursorChange);
			s_BlockMouseIconChange = false;
		}
	}

	void EditorUIContext::EndRendering()
	{
		RenderNotifications();
		
		ImGuiContext& g = *GImGui; 
		ImGuiIO& io = ImGui::GetIO();

		// Handle ensuring invalid mouse icons are not held
		static bool overlapActive = false;
		if (overlapActive && !g.HoveredIdAllowOverlap)
		{
			
			EngineService::GetActiveEngine().GetWindow().SetMouseCursorIcon(CursorIconType::Standard);
		}
		overlapActive = g.HoveredIdAllowOverlap;
		
		Engine& app = EngineService::GetActiveEngine();
		io.DisplaySize = ImVec2(static_cast<float>(app.GetWindow().GetWidth()), static_cast<float>(app.GetWindow().GetHeight()));
		if (s_DisableLeftClick)
		{
			io.MouseClicked[0] = false;
			s_DisableLeftClick = false;
		}
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	void EditorUIContext::StartRenderWindow(const char* label, bool* closeWindow, int32_t flags)
	{
		// Start Window
		ImGui::Begin(label, closeWindow, flags);
		m_ActiveWindowData.RecalculateDimensions();
	}

	void EditorUIContext::EndRenderWindow()
	{
		ImGui::End();
	}

	void EditorUIContext::StartDockspaceWindow()
	{
		// Set dockspace as main viewport window
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		// Apply window padding and rounding settings
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		// Set window flags
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		// Create the dockspace window
		EditorUI::EditorUIContext::StartRenderWindow("DockSpace", nullptr, window_flags);

		// Clear the window padding and rounding settings
		ImGui::PopStyleVar(3);

		// Submit the DockSpace
		ImGuiDockNodeFlags dockspace_flags = (uint32_t)ImGuiDockNodeFlags_None | (uint32_t)ImGuiDockNodeFlags_NoCloseButton;
		ImGui::DockSpace(ImGui::GetID("MyDockSpace"), ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	void EditorUIContext::EndDockspaceWindow()
	{
		ImGui::End();
	}

	uint32_t EditorUIContext::GetActiveWidgetID()
	{
		return GImGui->ActiveId;
	}

	const char* EditorUIContext::GetFocusedWindowName()
	{
		if (GImGui->NavWindow)
		{
			return GImGui->NavWindow->Name;
		}
		return nullptr;
	}

	void EditorUIContext::SetFocusedWindow(const char* windowName)
	{
		ImGui::SetWindowFocus(windowName);
	}

	void EditorUIContext::BringWindowToFront(const char* windowName)
	{
		ImGui::BringWindowToFront(windowName);
	}

	void EditorUIContext::BringCurrentWindowToFront()
	{
		if (ImGui::IsWindowAppearing())
			ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
	}

	void EditorUIContext::ClearWindowFocus()
	{
		ImGui::FocusWindow(NULL);
	}

	bool EditorUIContext::IsCurrentWindowVisible()
	{
		return ImGui::IsWindowVisible();
	}

	void EditorUIContext::HighlightFocusedWindow()
	{
		ImGuiWindow* window = GImGui->NavWindow;
		if (window)
		{
			ImVec2 windowPos = window->Pos;
			ImVec2 windowSize = window->Size;
			ImGui::GetForegroundDrawList(window)->AddRect(windowPos,
				ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y), ImGui::ColorConvertFloat4ToU32(m_ConfigColors.m_HighlightColor1_Thin));
		}
	}

	bool EditorUIContext::IsAnyItemHovered()
	{
		return ImGui::IsAnyItemHovered();
	}

	void EditorUIContext::SetDisableLeftClick(bool option)
	{
		s_DisableLeftClick = option;
	}

	void EditorUIContext::BlockMouseEvents(bool block)
	{
		s_BlockMouseEvents = block;
	}

	bool EditorUIContext::OnInputEvent(Events::Event* e)
	{
		KG_PROFILE_FUNCTION();
		ImGuiIO& io = ImGui::GetIO();
		if (!s_BlockMouseEvents)
		{
			return e->IsInCategory(Events::Mouse) && io.WantCaptureMouse;
		}
		return e->IsInCategory(Events::Keyboard) && io.WantCaptureKeyboard;
	}

	WidgetID EditorUIContext::GetNextChildID(WidgetID& currentID)
	{
		return ++currentID;
	}

	void EditorUIContext::Spacing(float space)
	{
		ImGui::Dummy(ImVec2(0.0f, space));
	}

	void EditorUIContext::TitleText(const std::string& text)
	{
		ImGui::PushFont(EditorUI::EditorUIContext::m_ConfigFonts.m_Title);
		ImGui::PushStyleColor(ImGuiCol_Text, m_ConfigColors.m_HighlightColor1);
		ImGui::TextUnformatted(text.c_str());
		ImGui::PopStyleColor();
		ImGui::PopFont();
	}

	void EditorUIContext::Spacing(SpacingAmount space)
	{
		switch (space)
		{
			case SpacingAmount::Small:
			{
				ImGui::Dummy(ImVec2(0.0f, 3.0f));
				return;
			}
			case SpacingAmount::Medium:
			{
				ImGui::Dummy(ImVec2(0.0f, 8.0f));
				return;
			}
			case SpacingAmount::Large:
			{
				ImGui::Dummy(ImVec2(0.0f, 15.0f));
				return;
			}
			case SpacingAmount::None:
			default:
			{
				KG_WARN("Invalid SpacingAmount provided to Spacing() enum function");
				return;
			}
		}
	}

	void EditorUIContext::RenderImage(Ref<Rendering::Texture2D> image, float size, ImVec4 tint)
	{
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 4.0f, ImGui::GetCursorPosY() + 3.2f));

		ImGui::Image((ImTextureID)(uint64_t)image->GetRendererID(), ImVec2(size, size),
			ImVec2{ 0, 1 }, ImVec2(1, 0), tint,
			k_PureEmpty);
	}

	float EditorUIContext::SmallButtonRelativeLocation(uint32_t slot)
	{
		return -EditorUIContext::m_ConfigSpacing.m_SmallButtonRightOffset - (EditorUIContext::m_ConfigSpacing.m_SmallButtonSpacing * slot);
	}

	void EditorUIContext::RenderInlineButton(ImGuiID widgetID, std::function<void()> onPress, 
		const InlineButton& spec, bool active, ImVec4 tintColor)
	{
		switch (spec.m_XPositionType)
		{
			case PositionType::Inline:
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + spec.m_XPosition);
				break;
			}
			case PositionType::Absolute:
			{
				ImGui::SetCursorPosX(spec.m_XPosition);
				break;
			}
			case PositionType::Relative:
			{
				ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x + spec.m_XPosition);
				break;
			}
			default:
			{
				break;	
			}
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spec.m_YPosition);
		ImGui::PushStyleColor(ImGuiCol_Button, k_PureEmpty);
		if (spec.m_Disabled)
		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, k_PureEmpty);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, k_PureEmpty);
		}
		Ref<Rendering::Texture2D> iconChoice = active ? spec.m_ActiveIcon : spec.m_InactiveIcon;
		if (ImGui::ImageButtonEx(widgetID,
			(ImTextureID)(uint64_t)iconChoice->GetRendererID(),
			ImVec2(spec.m_IconSize, spec.m_IconSize), ImVec2{ 0, 1 }, ImVec2{ 1, 0 },
			k_PureEmpty,
			tintColor, 0))
		{
			if (onPress)
			{
				onPress();
			}
		}
		ImGui::PopStyleColor(spec.m_Disabled ? 3 : 1);

		if (!spec.m_Disabled)
		{
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_HighlightColor1);
				ImGui::TextUnformatted(active ? spec.m_ActiveTooltip.c_str() : spec.m_InactiveTooltip.c_str());
				ImGui::PopStyleColor();
				ImGui::EndTooltip();
			}
		}

	}

	void EditorUIContext::RenderMultiLineText(const std::string& text, float lineWidth, float xOffset, float yOffset)
	{
		std::string previewOutput{};
		std::string previewRemainder{ text };
		uint32_t iteration{ 0 };
		int32_t lineEndPosition;
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		do 
		{
			lineEndPosition = ImGui::FindPositionAfterLength(previewRemainder.c_str(), lineWidth);
			ImGui::SameLine(xOffset);
			ImGui::SetCursorPosY({ ImGui::GetCursorPosY() + yOffset + (iteration * 20.0f) });
			if (lineEndPosition == -1)
			{
				// Draw backgrounds
				ImVec2 screenPosition = ImGui::GetCursorScreenPos();
				draw_list->AddRectFilled(ImVec2(EditorUI::EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUI::EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne - 5.0f, screenPosition.y),
					ImVec2(EditorUI::EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUI::EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne + EditorUI::EditorUIContext::m_ActiveWindowData.m_SecondaryTextLargeWidth, screenPosition.y + EditorUI::EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
					ImColor(EditorUI::EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, iteration == 0 ? ImDrawFlags_RoundCornersAll: ImDrawFlags_RoundCornersBottom);
				
				// Draw the text
				ImGui::TextUnformatted(previewRemainder.c_str());
			}
			else
			{
				// Draw backgrounds
				ImVec2 screenPosition = ImGui::GetCursorScreenPos();
				draw_list->AddRectFilled(ImVec2(EditorUI::EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUI::EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne - 5.0f, screenPosition.y),
					ImVec2(EditorUI::EditorUIContext::m_ActiveWindowData.m_WindowPosition.x + EditorUI::EditorUIContext::m_ActiveWindowData.m_SecondaryTextPosOne + EditorUI::EditorUIContext::m_ActiveWindowData.m_SecondaryTextLargeWidth, screenPosition.y + EditorUI::EditorUIContext::m_ConfigSpacing.m_TextBackgroundHeight),
					ImColor(EditorUI::EditorUIContext::m_ActiveWindowData.m_ActiveBackgroundColor), 4.0f, iteration == 0 ? ImDrawFlags_RoundCornersTop: ImDrawFlags_RoundCornersNone);


				previewOutput = previewRemainder.substr(0, lineEndPosition);
				previewRemainder = previewRemainder.substr(lineEndPosition, std::string::npos);
				ImGui::TextUnformatted(previewOutput.c_str());
			}
			iteration++;
		} while (lineEndPosition != -1);
	}

	void EditorUIContext::RenderTruncatedText(const std::string& text, uint32_t maxTextSize)
	{
		if (text.size() > maxTextSize)
		{
			std::string outputText = text.substr(0, maxTextSize - 2) + "..";
			ImGui::TextUnformatted(outputText.c_str());
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::PushStyleColor(ImGuiCol_Text, EditorUIContext::m_ConfigColors.m_HighlightColor1);
				ImGui::TextUnformatted(text.c_str());
				ImGui::PopStyleColor();
				ImGui::EndTooltip();
			}
		}
		else
		{
			ImGui::TextUnformatted(text.c_str());
		}
	}

	void EditorUIContext::NewItemScreen(const std::string& label1, std::function<void()> onPress1, const std::string& label2, std::function<void()> onPress2)
	{
		ImGui::PushFont(EditorUI::EditorUIContext::m_ConfigFonts.m_HeaderLarge);
		ImVec2 screenDimensions = ImGui::GetContentRegionAvail();
		ImVec2 originalLocation = ImGui::GetCursorScreenPos();
		if (screenDimensions.x > 700.0f)
		{
			ImVec2 buttonDimensions {2.75f, 5.0f};
			ImVec2 screenLocation = ImVec2(originalLocation.x + screenDimensions.x / 2 - (screenDimensions.x / buttonDimensions.x), originalLocation.y + screenDimensions.y / 2 - (screenDimensions.y / (buttonDimensions.y * 2)));
			ImGui::SetCursorScreenPos(screenLocation);
			if (ImGui::Button(label1.c_str(), ImVec2(screenDimensions.x / buttonDimensions.x, screenDimensions.y / buttonDimensions.y)))
			{
				if (onPress1)
				{
					onPress1();
				}
			}
			ImGui::SameLine();
			if (ImGui::Button(label2.c_str(), ImVec2(screenDimensions.x / buttonDimensions.x, screenDimensions.y / buttonDimensions.y)))
			{
				if (onPress2)
				{
					onPress2();
				}
				
			}
		}
		else
		{
			ImVec2 buttonDimensions{ 1.5f, 8.0f };
			ImVec2 screenLocation = ImVec2(originalLocation.x + screenDimensions.x / 2 - (screenDimensions.x / (buttonDimensions.x * 2)), originalLocation.y + screenDimensions.y / 2 - (screenDimensions.y / buttonDimensions.y));
			ImGui::SetCursorScreenPos(screenLocation);
			if (ImGui::Button(label1.c_str(), ImVec2(screenDimensions.x / buttonDimensions.x, screenDimensions.y / buttonDimensions.y)))
			{
				onPress1();
			}
			EditorUIContext::Spacing(SpacingAmount::Small);
			ImGui::SetCursorScreenPos(ImVec2(screenLocation.x, ImGui::GetCursorScreenPos().y));
			if (ImGui::Button(label2.c_str(), ImVec2(screenDimensions.x / buttonDimensions.x, screenDimensions.y / buttonDimensions.y)))
			{
				onPress2();
			}
		}
		ImGui::PopFont();
	}

	void EditorUIContext::LabeledText(const char* label, const char* text, LabeledTextFlags flags)
	{
		// Display Menu Item
		if (flags & LabeledText_Indented)
		{
			ImGui::SetCursorPosX(m_ConfigSpacing.m_PrimaryTextIndent);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, m_ConfigColors.m_PrimaryTextColor);
		int32_t labelPosition = ImGui::FindPositionAfterLength(label, m_ActiveWindowData.m_PrimaryTextWidth);
		RenderTruncatedText(label, labelPosition == -1 ? std::numeric_limits<int32_t>::max() : labelPosition);
		ImGui::PopStyleColor();
		ImGui::PushStyleColor(ImGuiCol_Text, m_ConfigColors.m_SecondaryTextColor);
		RenderMultiLineText(text, m_ActiveWindowData.m_SecondaryTextLargeWidth, m_ActiveWindowData.m_SecondaryTextPosOne);
		ImGui::PopStyleColor();
	}
	void EditorUIContext::Text(const char* text)
	{
		ImGui::TextUnformatted(text);
	}

	void EditorUIContext::BeginTabBar(const char* title)
	{
		ImGui::BeginTabBar(title);
	}
	void EditorUIContext::EndTabBar()
	{
		ImGui::EndTabBar();
	}
	bool EditorUIContext::BeginTabItem(const char* title)
	{
		return ImGui::BeginTabItem(title);
	}
	void EditorUIContext::EndTabItem()
	{
		ImGui::EndTabItem();
	}

	void UndoStack::StoreUndo(const UndoMemento& memento)
	{
		m_Stack.push(memento);
	}
	bool UndoStack::Undo()
	{
		// Handle case where stack is empty
		if (m_Stack.size() == 0)
		{
			return false;
		}

		// Get the top memento to return
		UndoMemento topMemento = m_Stack.top();

		// Remove the top memento from the stack
		m_Stack.pop();

		// Handle the undo
		KG_ASSERT(topMemento.m_Widget);
		EditVec4Widget& widget = *topMemento.m_Widget;

		// Revert the indicated widget's value
		widget.m_CurrentVec4 = topMemento.m_Value;

		// Call the on-confirm function if necessary
		if (widget.m_ConfirmAction)
		{
			widget.m_ConfirmAction(widget);
		}

		return true;
	}
	void ActiveWindowData::RecalculateDimensions()
	{
		float contentRegionX{ ImGui::GetContentRegionMax().x };

		// Calculate Widget Spacing Values
		m_WindowPosition = ImGui::GetWindowPos();
		m_PrimaryTextWidth = (i_ConfigSpacing->m_SecondaryTextPosOne * contentRegionX) - 20.0f;
		m_PrimaryTextIndentedWidth = (i_ConfigSpacing->m_SecondaryTextPosOne * contentRegionX) - 20.0f - i_ConfigSpacing->m_PrimaryTextIndent;
		m_SecondaryTextSmallWidth = ((i_ConfigSpacing->m_SecondaryTextPosTwo - i_ConfigSpacing->m_SecondaryTextPosOne) * contentRegionX) - 10.0f;
		m_SecondaryTextMediumWidth = ((i_ConfigSpacing->m_SecondaryTextPosMiddle - i_ConfigSpacing->m_SecondaryTextPosOne) * contentRegionX) - 30.0f;
		m_SecondaryTextLargeWidth = ((i_ConfigSpacing->m_SecondaryTextPosFour - i_ConfigSpacing->m_SecondaryTextPosOne) * contentRegionX) + m_SecondaryTextSmallWidth;

		m_SecondaryTextPosOne = contentRegionX * i_ConfigSpacing->m_SecondaryTextPosOne;
		m_SecondaryTextPosTwo = contentRegionX * i_ConfigSpacing->m_SecondaryTextPosTwo;
		m_SecondaryTextPosThree = contentRegionX * i_ConfigSpacing->m_SecondaryTextPosThree;
		m_SecondaryTextPosFour = contentRegionX * i_ConfigSpacing->m_SecondaryTextPosFour;
		m_SecondaryTextPosMiddle = contentRegionX * i_ConfigSpacing->m_SecondaryTextPosMiddle;
	}
	void UIPresets::CreatePresets()
	{
		m_SmallEditButton =
		{
			-i_ConfigSpacing->m_SmallButtonRightOffset,
			0.0f,
			13.0f,
			i_GenIcons->m_Edit,
			i_GenIcons->m_Edit,
			"Cancel Editing",
			"Edit",
			PositionType::Relative
		};

		m_SmallExpandButton =
		{
			-1.2f,
			4.0f,
			14.0f,
			i_GenIcons->m_Down,
			i_GenIcons->m_Right,
			"Collapse",
			"Expand",
			PositionType::Inline
		};

		m_MediumOptionsButton =
		{
			-i_ConfigSpacing->m_MediumButtonRightOffset,
			1.0f,
			19.0f,
			i_GenIcons->m_Options,
			i_GenIcons->m_Options,
			"Options",
			"Options",
			PositionType::Relative
		};

		m_SmallCheckboxButton = 
		{
			0.0f,
			0.0f,
			14.0f,
			i_GenIcons->m_Checkbox_Enabled,
			i_GenIcons->m_Checkbox_Disabled,
			"Uncheck",
			"Check",
			PositionType::Inline
		};

		m_SmallCheckboxDisabledButton = 
		{
			0.0f,
			0.0f,
			14.0f,
			i_GenIcons->m_Checkbox_Enabled,
			i_GenIcons->m_Checkbox_Disabled,
			"",
			"",
			PositionType::Inline,
			true
		};

		m_SmallLinkButton = 
		{
			-i_ConfigSpacing->m_SmallButtonRightOffset,
			0.0f,
			14.0f,
			i_BrowserIcons->m_Forward,
			i_BrowserIcons->m_Forward,
			"Open",
			"Open",
			PositionType::Relative
		};

		m_LargeDeleteButton = 
		{
			-112.0f,
			-0.6f,
			28.0f,
			i_GenIcons->m_Delete,
			i_GenIcons->m_Delete,
			"Delete",
			"Delete",
			PositionType::Relative
		};

		m_LargeCancelButton = 
		{
			-75.0f,
			-0.6f,
			28.0f,
			i_GenIcons->m_Cancel,
			i_GenIcons->m_Cancel,
			"Cancel",
			"Cancel",
			PositionType::Relative
		};

		m_LargeConfirmButton = 
		{
			-38.0f,
			-0.6f,
			28.0f,
			i_GenIcons->m_Confirm,
			i_GenIcons->m_Confirm,
			"Confirm",
			"Confirm",
			PositionType::Relative
		};

		m_LargeSearchButton = 
		{
			-112.0f,
			-0.6f,
			28.0f,
			i_GenIcons->m_Cancel2,
			i_GenIcons->m_Search,
			"Cancel Search",
			"Search",
			PositionType::Relative
		};

		m_TableEditButton = m_SmallEditButton;
		m_TableEditButton.m_YPosition = -5.5f;

		m_TableLinkButton = m_SmallLinkButton;
		m_TableLinkButton.m_YPosition = -5.5f;

		m_ListExpandButton = m_SmallExpandButton;
	}
}
