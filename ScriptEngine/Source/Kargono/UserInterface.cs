using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Kargono
{
	public class UserInterface
	{
		public static void LoadUserInterface(string userInterfaceLocation)
		{
			InternalCalls.UserInterface_LoadUserInterface(userInterfaceLocation);
		}

		public static void MoveRight()
		{
			InternalCalls.UserInterface_MoveRight();
		}

		public static void MoveLeft()
		{
			InternalCalls.UserInterface_MoveLeft();
		}

		public static void MoveUp()
		{
			InternalCalls.UserInterface_MoveUp();
		}

		public static void MoveDown()
		{
			InternalCalls.UserInterface_MoveDown();
		}

		public static void OnPress()
		{
			InternalCalls.UserInterface_OnPress();
		}
		public static void SetWidgetText(string windowTag, string widgetTag, string newText)
		{
			InternalCalls.UserInterface_SetWidgetText(windowTag, widgetTag, newText);
		}

		public static void SetDisplayWindow(string windowTag, bool display)
		{
			InternalCalls.UserInterface_SetDisplayWindow(windowTag, display);
		}
	}

}
