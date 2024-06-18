#pragma once

namespace Kargono::Utility
{
	//==============================
	// General Operations Class
	//==============================
	class Operations
	{
	public:
		//==============================
		// Boolean Operations
		//==============================
		static void ToggleBoolean(bool& boolToToggle)
		{
			boolToToggle = !boolToToggle;
		}
	};
}
