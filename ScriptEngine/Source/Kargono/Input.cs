using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Kargono
{
	public class Input
	{
		public static bool IsKeyDown(KeyCode keycode)
		{
			return InternalCalls.Input_IsKeyDown(keycode);
		}
	}

	public class InputMode
	{
		public static bool IsKeySlotDown(UInt16 keySlot)
		{
			return InternalCalls.InputMode_IsKeySlotDown(keySlot);
		}

		public static void LoadInputMode(string inputModeLocation)
		{
			InternalCalls.InputMode_LoadInputMode(inputModeLocation);
		}
	}
}
