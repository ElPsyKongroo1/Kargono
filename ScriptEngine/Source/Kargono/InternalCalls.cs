using System;
using System.Runtime.CompilerServices;


namespace Kargono
{
	public static class InternalCalls
	{
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_HasComponent(ulong entityID, Type componentType);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_FindEntityByName(string name);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static object GetScriptInstance(ulong entityID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetTranslation(ulong entityID, out Vector3 parameter);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetTranslation(ulong entityID, ref Vector3 parameter);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyDown(KeyCode keycode);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool InputMode_IsKeySlotDown(ushort keySlot);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_ApplyLinearImpulse(ulong entityID, ref Vector2 impulse, ref Vector2 point, bool wake);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(ulong entityID, ref Vector2 impulse, bool wake);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_GetLinearVelocity(ulong entityID, out Vector2 linearVelocity );
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_SetLinearVelocity(ulong entityID, ref Vector2 linearVelocity);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static Rigidbody2DComponent.BodyType Rigidbody2DComponent_GetType(ulong entityID);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_SetType(ulong entityID, Rigidbody2DComponent.BodyType type);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static String TagComponent_GetTag(ulong entityID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Audio_PlayAudio(string audioFileLocation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Audio_PlayStereoAudio(string audioFileLocation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Scene_TransitionScene(string sceneFileLocation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void AudioComponent_PlayAudio(ulong entityID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void AudioComponent_PlayAudioByName(ulong entityID, string audioTag);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void UserInterface_LoadUserInterface(string userInterfaceLocation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void UserInterface_MoveRight();

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void UserInterface_MoveLeft();

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void UserInterface_MoveUp();

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void UserInterface_MoveDown();

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void UserInterface_OnPress();

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void UserInterface_SetWidgetText(string windowTag, string widgetTag, string newText);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void UserInterface_SetDisplayWindow(string windowTag, bool display);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void InputMode_LoadInputMode(string inputModeLocation);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Core_CloseApplication();
	}
}
