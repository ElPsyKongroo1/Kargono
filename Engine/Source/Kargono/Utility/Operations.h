#pragma once

namespace Kargono::Utility
{
	class Operations
	{
	public:
		static void ToggleBoolean(bool& boolToToggle)
		{
			boolToToggle = !boolToToggle;
		}
	};
}
