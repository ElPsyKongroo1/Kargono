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
		static void OpenMainMenu()
		{
			UserInterface.LoadUserInterface("UserInterface/Main Menu.kgui");
			Scene.TransitionScene("Scenes/main_menu.kgscene");
			InputMode.LoadInputMode("Input/MainMenu.kginput");
			Audio.PlayStereoAudio("Audio/Manoria-Cathedral.wav");
		}

		static void OpenPongGameplay()
		{
			UserInterface.LoadUserInterface("UserInterface/RuntimeUI.kgui");
			UserInterface.SetDisplayWindow("pre_game_warning", true);
			Scene.TransitionScene("Scenes/main_gameplay.kgscene");
			InputMode.LoadInputMode("Input/Pre_Start.kginput");
			Audio.PlayAudio("Audio/menu_confirm.wav");
			Audio.PlayStereoAudio("Audio/mechanist-theme.wav");
		}

		static void CloseApplication()
		{
			Core.CloseApplication();
		}

		static void MoveRight()
		{
			UserInterface.MoveRight();
		}

		static void MoveLeft()
		{
			UserInterface.MoveLeft();
		}

        static void MoveUp()
        {
            UserInterface.MoveUp();
        }

        static void MoveDown()
        {
            UserInterface.MoveDown();
        }

        static void OnPress()
        {
	        UserInterface.OnPress();
        }

        static void PlayMenuSelect()
        {
	        Audio.PlayAudio("Audio/menu_select.wav");
        }

        static void StartGame()
        {
			InputMode.LoadInputMode("Input/Runtime.kginput");
			UserInterface.SetDisplayWindow("pre_game_warning", false);

			Player1 player1 = Entity.FindEntityByName("Player1").As<Player1>();
			player1.m_Transform.Translation = player1.InitialPosition;

			Player2 player2 = Entity.FindEntityByName("Player2").As<Player2>();
			player2.m_Transform.Translation = player2.InitialPosition;

			Ball ball = Entity.FindEntityByName("Ball").As<Ball>();
			ball.m_Transform.Translation = ball.InitialPosition;

			Random rnd = new Random();
			Int32 directionChoice = rnd.Next(2);
			Vector2 velocity = Vector2.Zero;

			if (directionChoice == 0)
			{
				velocity.X = -1.0f;
			}
			else
			{
				velocity.X = 1.0f;
			}

			ball.m_Rigidbody.LinearVelocity = velocity * ball.Speed;
			


		}
	}
}
