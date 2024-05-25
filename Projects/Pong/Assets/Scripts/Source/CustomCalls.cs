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
		static void UpdateOnlineCount(UInt32 count)
		{
			string onlineCount = "Online: " + count;
			UserInterface.SetWidgetText("main_window", "online_count", onlineCount);
			UserInterface.SetWidgetSelectable("main_window", "online_multiplayer", true);
			UserInterface.SetWidgetTextColor("main_window", "online_multiplayer", new Vector4(1.0f));
			UserInterface.SetWidgetBackgroundColor("main_window", "online_multiplayer", new Vector4(103.0f / 255.0f, 17.0f / 255.0f, 175.0f / 255.0f, 54.0f / 255.0f));
		}

		static void SendReadyCheck()
		{
			Network.SessionReadyCheck();
		}

		static void StartOnlineGame()
		{
			UInt16 userSlot = Network.GetSessionSlot();
			if (userSlot == 0)
			{
				InputMode.LoadInputMode("Input/Online_Player1_Runtime.kginput");
			}
			else if (userSlot == 1)
			{
				InputMode.LoadInputMode("Input/Online_Player2_Runtime.kginput");
			}

			UserInterface.SetDisplayWindow("pre_game_warning", false);
			UserInterface.SetDisplayWindow("online_lobby", false);
			UserInterface.SetDisplayWindow("base_window", true);

			Player1 player1 = Entity.FindEntityByName("Player1").As<Player1>();
			player1.m_Transform.Translation = player1.InitialPosition;

			Player2 player2 = Entity.FindEntityByName("Player2").As<Player2>();
			player2.m_Transform.Translation = player2.InitialPosition;

			Ball ball = Entity.FindEntityByName("Ball").As<Ball>();
			ball.m_Transform.Translation = ball.InitialPosition;

			Random rnd = new Random();
			Vector2 velocity = Vector2.Zero;

			//InputMode.GetField<UInt16>("BallDirection");

			UInt16 ballDirection = GameState.GetField<UInt16>("BallDirection");

			if (ballDirection == 0)
			{
				velocity.X = -1.0f;
				GameState.SetField<UInt16>("BallDirection", 1);
			}
			else
			{
				velocity.X = 1.0f;
				GameState.SetField<UInt16>("BallDirection", 0);
			}

			ball.m_Rigidbody.LinearVelocity = velocity * ball.Speed;
		}

		static void OnSessionReadyCheckConfirm()
		{
			StartOnlineGame();
		}

		static void ApproveJoinSession(UInt16 userSlot)
		{
			GameState.SetField<UInt16>("BallDirection", 0);
			UserInterface.LoadUserInterface("UserInterface/RuntimeUI.kgui");
			UserInterface.SetDisplayWindow("online_lobby", true);
			UserInterface.SetDisplayWindow("base_window", false);
			Scene.TransitionScene("Scenes/main_gameplay.kgscene");
			InputMode.LoadInputMode("Input/Online_Lobby_Input.kginput");
			Audio.PlayAudio("Audio/menu_confirm.wav");
			Audio.PlayStereoAudio("Audio/greenA.wav");

			string selectedWidget = "player_slot_" + userSlot;
			UserInterface.SetWidgetText("online_lobby", selectedWidget, "Connected!");
		}

		static void OnUpdateSessionUserSlot(UInt16 userSlot)
		{
			string selectedWidget = "player_slot_" + userSlot;
			UserInterface.SetWidgetText("online_lobby", selectedWidget, "Connected!");
		}

		static void UserLeftSession(UInt16 userSlot)
		{
			string selectedWidget = "player_slot_" + userSlot;
			string newText = "No Player " + (userSlot + 1);

			UserInterface.SetWidgetText("online_lobby", selectedWidget, newText);

			UserInterface.SetWidgetText("online_lobby", "main_text", "Waiting for Players...");

		}

		static void OnStartSession()
        {
	        UInt16 userSlot = Network.GetSessionSlot();
	        UserInterface.SetDisplayWindow("pre_game_warning", true);
	        UserInterface.SetDisplayWindow("base_window", false);
	        UserInterface.SetDisplayWindow("online_lobby", false);
			InputMode.LoadInputMode("Input/Online_Pre_Start.kginput");
	        Audio.PlayAudio("Audio/menu_confirm.wav");
	        if (userSlot == 0)
	        {
		        UserInterface.SetWidgetText("pre_game_warning", "controls_2", "");
			}
	        else if (userSlot == 1)
	        {
		        UserInterface.SetWidgetText("pre_game_warning", "controls_1", "");
			}

	        Network.EnableReadyCheck();

        }

        static void OnCurrentSessionInit()
		{
			UserInterface.SetWidgetText("online_lobby", "main_text", "Starting Session...");
		}

		static void OnConnectionTerminated()
		{
			UserInterface.SetWidgetText("main_window", "online_count", "Offline");
			UserInterface.SetWidgetSelectable("main_window", "online_multiplayer", false);
			UserInterface.SetWidgetTextColor("main_window", "online_multiplayer", new Vector4(1.0f, 1.0f, 1.0f, 222.0f / 255.0f));
			UserInterface.SetWidgetBackgroundColor("main_window", "online_multiplayer", new Vector4(30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 37.0f / 255.0f));
			UserInterface.SetSelectedWidget("main_window", "local_multiplayer");
		}

		static void OnReceiveSignal(UInt16 signalNum)
		{
			NetworkSignals signal = (NetworkSignals)signalNum;
			UInt32 userSlot = Network.GetSessionSlot();
			switch (signal)
			{
				case NetworkSignals.EndGame:
				{
					Ball ball = Entity.FindEntityByName("Ball").As<Ball>();
					if (userSlot == 0)
					{
						ball.PlayerOneScore++;
						UserInterface.SetWidgetText("base_window", "score_player_1", ball.PlayerOneScore.ToString());
					}
					else if (userSlot == 1)
					{
						ball.PlayerTwoScore++;
						UserInterface.SetWidgetText("base_window", "score_player_2", ball.PlayerTwoScore.ToString());
					}

					ball.m_Rigidbody.LinearVelocity *= 0;
					ball.m_Audio.PlayAudio("lose_sound");
					UserInterface.SetDisplayWindow("pre_game_warning", true);
					InputMode.LoadInputMode("Input/Online_Pre_Start.kginput");
					Network.EnableReadyCheck();

					break;
				}
			}
		}

		static void OpenMainMenu()
		{
			UserInterface.LoadUserInterface("UserInterface/Main Menu.kgui");
			Scene.TransitionScene("Scenes/main_menu.kgscene");
			InputMode.LoadInputMode("Input/MainMenu.kginput");
			Audio.PlayStereoAudio("Audio/blueA.wav");
			Network.RequestUserCount();
		}

		static void LeaveOnlineGameplay()
		{
			Network.LeaveCurrentSession();
			OpenMainMenu();
		}

		static void OpenPongGameplay()
		{
			UserInterface.LoadUserInterface("UserInterface/RuntimeUI.kgui");
			UserInterface.SetDisplayWindow("pre_game_warning", true);
			UserInterface.SetDisplayWindow("base_window", false);
			Scene.TransitionScene("Scenes/main_gameplay.kgscene");
			InputMode.LoadInputMode("Input/Pre_Start.kginput");
			Audio.PlayAudio("Audio/menu_confirm.wav");
			Audio.PlayStereoAudio("Audio/greenA.wav");
		}

		static void OpenOnlineGameplay()
		{
			Network.RequestJoinSession();
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
			UserInterface.SetDisplayWindow("online_lobby", false);
			UserInterface.SetDisplayWindow("base_window", true);

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
