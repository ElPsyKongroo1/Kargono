#pragma once
#include "Kargono.h"





namespace Kargono
{
	class InputEditorPanel
	{
	public:
		InputEditorPanel();

		// This panel allows the editor to create different input modes that can be swapped out
		void OnEditorUIRender();
	private:
		// This function displays the events section for keyboard input which currently only includes the OnUpdate()
		//		function.
		void InputEditor_Keyboard_OnUpdate();

		void InputEditor_Keyboard_OnKeyPressed();
		// This function displays the keyboard input polling interface.
		void InputEditor_Keyboard_Polling();


	};
}
