using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Kargono;
using ScriptEngine.Source.Kargono;

namespace Pong
{
	public class CustomCalls
	{
		static void PlayMusic()
		{
			Audio.PlayAudio("Audio\\Manoria-Cathedral.wav");
		}

		static void PlayMusicWithMask()
		{
			if (InputMode.IsKeySlotDown(0))
			{
				Audio.PlayAudio("Audio\\Manoria-Cathedral.wav");
			}
		}

		static void PlayMusihwfegwaejyfgwau()
		{
			Audio.PlayAudio("Audio\\Manoria-Cathedral.wav");
		}

		static void kahsfuihkhshae()
		{
			Audio.PlayAudio("Audio\\Manoria-Cathedral.wav");
		}

		static void ChangeScene()
		{
			Scene.TransitionScene("Scenes/new_scene.kgscene");
		}
	}
}
