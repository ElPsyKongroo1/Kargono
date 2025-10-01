#include "kgpch.h"

#include "Modules/Assets/AssetService.h"
#include "Modules/Assets/UserInterfaceManager.h"

#include "Modules/RuntimeUI/RuntimeUIContext.h"

namespace Kargono::Assets
{
	void UserInterfaceManager::CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		UNREFERENCED_PARAMETER(name);

		// Create Temporary UserInterface
		Ref<RuntimeUI::UserInterface> temporaryUserInterface = CreateRef<RuntimeUI::UserInterface>();

		// Save into File
		SerializeAsset(temporaryUserInterface, assetPath);

		// Load data into In-Memory Metadata object
		Ref<Assets::UserInterfaceMetaData> metadata = CreateRef<Assets::UserInterfaceMetaData>();
		asset.Data.SpecificFileData = metadata;
	}

	static bool RemoveScriptFromWidget(Ref<RuntimeUI::Widget> widgetRef, Assets::AssetHandle scriptHandle);

	bool RemoveScriptFromWidget(Ref<RuntimeUI::Widget> widgetRef, Assets::AssetHandle scriptHandle)
	{
		bool uiModified{ false };

		if (widgetRef->m_WidgetType == RuntimeUI::WidgetTypes::InputTextWidget)
		{
			RuntimeUI::InputTextWidget& inputTextWidget = *(RuntimeUI::InputTextWidget*)widgetRef.get();
			if (inputTextWidget.m_OnMoveCursorHandle == scriptHandle)
			{
				inputTextWidget.m_OnMoveCursorHandle = Assets::k_EmptyHandle;
				inputTextWidget.m_OnMoveCursor = nullptr;
				uiModified = true;
			}
		}

		if (widgetRef->m_WidgetType == RuntimeUI::WidgetTypes::SliderWidget)
		{
			RuntimeUI::SliderWidget& sliderWidget = *(RuntimeUI::SliderWidget*)widgetRef.get();
			if (sliderWidget.m_OnMoveSliderHandle == scriptHandle)
			{
				sliderWidget.m_OnMoveSliderHandle = Assets::k_EmptyHandle;
				sliderWidget.m_OnMoveSlider = nullptr;
				uiModified = true;
			}
		}

		if (widgetRef->m_WidgetType == RuntimeUI::WidgetTypes::DropDownWidget)
		{
			RuntimeUI::DropDownWidget& dropDownWidget = *(RuntimeUI::DropDownWidget*)widgetRef.get();
			if (dropDownWidget.m_OnSelectOptionHandle == scriptHandle)
			{
				dropDownWidget.m_OnSelectOptionHandle = Assets::k_EmptyHandle;
				dropDownWidget.m_OnSelectOption = nullptr;
				uiModified = true;
			}
		}

		RuntimeUI::ContainerData* containerData = widgetRef->GetContainerData();
		if (containerData)
		{
			for (Ref<RuntimeUI::Widget> currentWidget : containerData->m_ContainedWidgets)
			{
				bool modified = RemoveScriptFromWidget(currentWidget, scriptHandle);
				if (modified)
				{
					uiModified = true;
				}
			}
		}

		// Check if this widget has a selection data
		RuntimeUI::SelectionData* selectionData = widgetRef->GetSelectionData();
		if (!selectionData)
		{
			return uiModified;
		}

		// Remove script references from widget if necessary
		if (selectionData->m_FunctionPointers.m_OnPressHandle == scriptHandle)
		{
			selectionData->m_FunctionPointers.m_OnPressHandle = Assets::k_EmptyHandle;
			selectionData->m_FunctionPointers.m_OnPress = nullptr;
			uiModified = true;
		}

		return uiModified;
	}

	bool UserInterfaceManager::RemoveScript(Ref<RuntimeUI::UserInterface> userInterfaceRef, Assets::AssetHandle scriptHandle)
	{
		// Handle UI level function pointers
		bool uiModified{ false };
		if (userInterfaceRef->m_Config.m_FunctionPointers.m_OnMoveHandle == scriptHandle)
		{
			userInterfaceRef->m_Config.m_FunctionPointers.m_OnMoveHandle = Assets::k_EmptyHandle;
			userInterfaceRef->m_Config.m_FunctionPointers.m_OnMove = nullptr;
			uiModified = true;
		}
		if (userInterfaceRef->m_Config.m_FunctionPointers.m_OnHoverHandle == scriptHandle)
		{
			userInterfaceRef->m_Config.m_FunctionPointers.m_OnHoverHandle = Assets::k_EmptyHandle;
			userInterfaceRef->m_Config.m_FunctionPointers.m_OnHover = nullptr;
			uiModified = true;
		}

		// Handle all widgets in all windows
		for (RuntimeUI::Window& currentWindow : userInterfaceRef->m_WindowsState.m_Windows)
		{
			for (Ref<RuntimeUI::Widget> widgetRef : currentWindow.m_Widgets)
			{
				bool modified = RemoveScriptFromWidget(widgetRef, scriptHandle);

				if (modified)
				{
					uiModified = true;
				}
			}
		}

		return uiModified;
	}

	static bool RemoveTextureFromWidget(Ref<RuntimeUI::Widget> widgetRef, Assets::AssetHandle textureHandle);

	bool RemoveTextureFromWidget(Ref<RuntimeUI::Widget> widgetRef, Assets::AssetHandle textureHandle)
	{
		bool uiModified{ false };

		if (widgetRef->m_WidgetType != RuntimeUI::WidgetTypes::ImageWidget)
		{
			// Remove texture reference from widget if necessary
			RuntimeUI::ImageWidget& imageWidget = *(RuntimeUI::ImageWidget*)widgetRef.get();
			if (imageWidget.m_ImageData.m_ImageHandle == textureHandle)
			{
				RuntimeUI::ImageWidget& buttonWidget = *(RuntimeUI::ImageWidget*)widgetRef.get();
				buttonWidget.m_ImageData.m_ImageHandle = Assets::k_EmptyHandle;
				buttonWidget.m_ImageData.m_ImageRef = nullptr;
				uiModified = true;
			}
		}
		if (widgetRef->m_WidgetType != RuntimeUI::WidgetTypes::ImageButtonWidget)
		{
			// Remove texture reference from widget if necessary
			RuntimeUI::ImageButtonWidget& imageWidget = *(RuntimeUI::ImageButtonWidget*)widgetRef.get();
			if (imageWidget.m_ImageData.m_ImageHandle == textureHandle)
			{
				RuntimeUI::ImageButtonWidget& imageButtonWidget = *(RuntimeUI::ImageButtonWidget*)widgetRef.get();
				imageButtonWidget.m_ImageData.m_ImageHandle = Assets::k_EmptyHandle;
				imageButtonWidget.m_ImageData.m_ImageRef = nullptr;
				uiModified = true;
			}
		}
		if (widgetRef->m_WidgetType != RuntimeUI::WidgetTypes::CheckboxWidget)
		{
			// Remove texture reference from widget if necessary
			RuntimeUI::CheckboxWidget& checkboxWidget = *(RuntimeUI::CheckboxWidget*)widgetRef.get();
			if (checkboxWidget.m_ImageUnChecked.m_ImageHandle == textureHandle)
			{
				checkboxWidget.m_ImageUnChecked.m_ImageHandle = Assets::k_EmptyHandle;
				checkboxWidget.m_ImageUnChecked.m_ImageRef = nullptr;
				uiModified = true;
			}
			if (checkboxWidget.m_ImageChecked.m_ImageHandle == textureHandle)
			{
				checkboxWidget.m_ImageChecked.m_ImageHandle = Assets::k_EmptyHandle;
				checkboxWidget.m_ImageChecked.m_ImageRef = nullptr;
				uiModified = true;
			}
		}

		RuntimeUI::ContainerData* containerData = widgetRef->GetContainerData();
		if (containerData)
		{
			for (Ref<RuntimeUI::Widget> currentWidget : containerData->m_ContainedWidgets)
			{
				bool modified = RemoveTextureFromWidget(currentWidget, textureHandle);
				if (modified)
				{
					uiModified = true;
				}
			}
		}

		return uiModified;
	}

	bool UserInterfaceManager::RemoveTexture(Ref<RuntimeUI::UserInterface> userInterfaceRef, Assets::AssetHandle textureHandle)
	{
		bool uiModified{ false };

		// Handle all widgets in all windows
		for (RuntimeUI::Window& currentWindow : userInterfaceRef->m_WindowsState.m_Windows)
		{
			for (Ref<RuntimeUI::Widget> widgetRef : currentWindow.m_Widgets)
			{
				bool modified = RemoveTextureFromWidget(widgetRef, textureHandle);
				if (modified)
				{
					uiModified = true;
				}
				
			}
		}

		return uiModified;
	}
}
