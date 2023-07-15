/// @brief Include guard to prevent multiple inclusion of this header file
#pragma once
/// @brief Include the Input.h header file from the Kargono namespace
#include "Kargono/Input.h"

/// @brief Declaration of the Kargono namespace
namespace Kargono
{
/// @brief Declaration of the WindowsInput class, which inherits from the Input class
	class WindowsInput : public Input
	{
/// @brief Access specifier for the following member functions and variables
	protected:
/// @brief Implementation of the IsKeyPressedImpl function
		virtual bool IsKeyPressedImpl(int keycode) override;

/// @brief Implementation of the IsMouseButtonPressedImpl function
		virtual bool IsMouseButtonPressedImpl(int button) override;
/// @brief Implementation of the GetMousePositionImpl function
		virtual std::pair<float, float> GetMousePositionImpl() override;
/// @brief Implementation of the GetMouseXImpl function
		virtual bool GetMouseXImpl() override;
/// @brief Implementation of the GetMouseYImpl function
		virtual bool GetMouseYImpl() override;


	};

}