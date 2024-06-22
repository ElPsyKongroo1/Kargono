using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Kargono;
using static System.Runtime.CompilerServices.RuntimeHelpers;

namespace Pong
{
	enum NetworkSignals : UInt16
	{
		None = 0, EndGame = 1
	}


	public class Player1 : Entity
	{
		public TransformComponent m_Transform;
		public Rigidbody2DComponent m_Rigidbody;

		public float Speed = 2.0f;
		private float SpeedUpFactor = 2.0f;
		public Vector3 InitialPosition = Vector3.Zero;

		void OnCreate()
		{
			m_Transform = GetComponent<TransformComponent>();
			InitialPosition = m_Transform.Translation;
			m_Rigidbody = GetComponent<Rigidbody2DComponent>();
		}

		void MoveUp(float ts)
		{
			bool upperLimit = m_Transform.Translation.Y >= 11.15;
			bool lowerLimit = m_Transform.Translation.Y <= -11.15;

			float speed = Input.IsKeyDown(KeyCode.LeftShift) ? Speed * SpeedUpFactor : Speed;
			Vector3 velocity = Vector3.Zero;

			if (!upperLimit) { velocity.Y = 1.0f; }

			velocity *= speed * ts;
			Translation += velocity;
		}

		void MoveDown(float ts)
		{
			bool upperLimit = m_Transform.Translation.Y >= 11.15;
			bool lowerLimit = m_Transform.Translation.Y <= -11.15;

			float speed = Input.IsKeyDown(KeyCode.LeftShift) ? Speed * SpeedUpFactor : Speed;
			Vector3 velocity = Vector3.Zero;

			if (!lowerLimit) { velocity.Y = -1.0f; }

			velocity *= speed * ts;
			Translation += velocity;
		}

		void OnUpdate(float ts)
		{
			UInt16 userSlot = Network.GetSessionSlot();
			if (userSlot == 0)
			{
				Network.SendAllEntityLocation(ID, m_Transform.Translation);
			}
		}

	}

	public class Player2 : Entity
	{
		public TransformComponent m_Transform;
		public Rigidbody2DComponent m_Rigidbody;

		public float Speed = 2.0f;
		private float SpeedUpFactor = 2.0f;
		public Vector3 InitialPosition = Vector3.Zero;

		void OnCreate()
		{
			m_Transform = GetComponent<TransformComponent>();
			InitialPosition = m_Transform.Translation;
			m_Rigidbody = GetComponent<Rigidbody2DComponent>();
		}

		void MoveUp(float ts)
		{
			bool upperLimit = m_Transform.Translation.Y >= 11.15;
			bool lowerLimit = m_Transform.Translation.Y <= -11.15;

			float speed = Input.IsKeyDown(KeyCode.RightShift) ? Speed * SpeedUpFactor : Speed;
			Vector3 velocity = Vector3.Zero;

			if (!upperLimit) { velocity.Y = 1.0f; }

			velocity *= speed * ts;
			Translation += velocity;
		}

		void MoveDown(float ts)
		{
			bool upperLimit = m_Transform.Translation.Y >= 11.15;
			bool lowerLimit = m_Transform.Translation.Y <= -11.15;

			float speed = Input.IsKeyDown(KeyCode.RightShift) ? Speed * SpeedUpFactor : Speed;
			Vector3 velocity = Vector3.Zero;

			if (!lowerLimit) { velocity.Y = -1.0f; }

			velocity *= speed * ts;
			Translation += velocity;
		}

		void OnUpdate(float ts)
		{
			UInt16 userSlot = Network.GetSessionSlot();
			if (userSlot == 1)
			{
				Network.SendAllEntityLocation(ID, m_Transform.Translation);
			}
		}

	}

	public class Ball : Entity
	{
		public AudioComponent m_Audio;
		public TransformComponent m_Transform;
		public Rigidbody2DComponent m_Rigidbody;

		public float Speed = 2.0f;
		public bool ChangeVelocity = false;
		public UInt32 PlayerOneScore = 0;
		public UInt32 PlayerTwoScore = 0;
		public Vector3 InitialPosition = Vector3.Zero;

		void OnCreate()
		{
			m_Audio = GetComponent<AudioComponent>();
			m_Transform = GetComponent<TransformComponent>();
			InitialPosition = m_Transform.Translation;
			m_Rigidbody = GetComponent<Rigidbody2DComponent>();
		}

		void OnUpdate(float ts)
		{
		}

		bool OfflineCollision(ulong otherEntity)
		{
			Entity otherEntityInstance = CreateEntityWithID(otherEntity);

			bool collisionHandled = false;
			switch (otherEntityInstance.GetComponent<TagComponent>().Tag)
			{
				case "Left Wall":
					{
						PlayerTwoScore++;
						UserInterface.SetWidgetText("base_window", "score_player_2", PlayerTwoScore.ToString());
						m_Rigidbody.LinearVelocity *= 0;
						m_Audio.PlayAudio("lose_sound");
						UserInterface.SetDisplayWindow("pre_game_warning", true);
						InputMode.LoadInputMode("Input/Pre_Start.kginput");
						collisionHandled = true;
						break;
					}
				case "Right Wall":
					{
						PlayerOneScore++;
						UserInterface.SetWidgetText("base_window", "score_player_1", PlayerOneScore.ToString());
						m_Rigidbody.LinearVelocity *= 0;
						m_Audio.PlayAudio("lose_sound");
						UserInterface.SetDisplayWindow("pre_game_warning", true);
						InputMode.LoadInputMode("Input/Pre_Start.kginput");
						collisionHandled = true;
						break;
					}
				case "Top Wall":
				case "Bottom Wall":
					{
						Vector2 horizontalDirection;
						Vector2 currentVelocity = m_Rigidbody.LinearVelocity;
						if (currentVelocity.X >= 0)
						{
							horizontalDirection = new Vector2(1.0f, 0.0f);
						}
						else
						{
							horizontalDirection = new Vector2(-1.0f, 0.0f);
						}
						currentVelocity = (currentVelocity.Normalize() + (horizontalDirection * 0.1f)).Normalize() * Speed;
						m_Rigidbody.LinearVelocity = currentVelocity;
						m_Audio.PlayAudio();
						collisionHandled = true;
						break;
					}
				case "Player1":
					{
						if (Input.IsKeyDown(KeyCode.W))
						{
							float deflectionFactor = Input.IsKeyDown(KeyCode.LeftShift) ? 0.55f : 0.33f;
							Vector2 up = new Vector2(0.0f, 1.0f);
							Vector2 currentVelocity = m_Rigidbody.LinearVelocity;

							currentVelocity = (currentVelocity.Normalize() + (up * deflectionFactor)).Normalize() * Speed;
							m_Rigidbody.LinearVelocity = currentVelocity;
						}
						if (Input.IsKeyDown(KeyCode.A))
						{
							float deflectionFactor = Input.IsKeyDown(KeyCode.LeftShift) ? 0.55f : 0.33f;
							Vector2 up = new Vector2(0.0f, -1.0f);
							Vector2 currentVelocity = m_Rigidbody.LinearVelocity;

							currentVelocity = (currentVelocity.Normalize() + (up * deflectionFactor)).Normalize() * Speed;
							m_Rigidbody.LinearVelocity = currentVelocity;
						}
						m_Audio.PlayAudio();
						collisionHandled = true;
						break;
					}
				case "Player2":
					{
						if (Input.IsKeyDown(KeyCode.O))
						{
							float deflectionFactor = Input.IsKeyDown(KeyCode.LeftShift) ? 0.55f : 0.33f;
							Vector2 up = new Vector2(0.0f, 1.0f);
							Vector2 currentVelocity = m_Rigidbody.LinearVelocity;

							currentVelocity = (currentVelocity.Normalize() + (up * deflectionFactor)).Normalize() * Speed;
							m_Rigidbody.LinearVelocity = currentVelocity;
						}
						if (Input.IsKeyDown(KeyCode.Semicolon))
						{
							float deflectionFactor = Input.IsKeyDown(KeyCode.LeftShift) ? 0.55f : 0.33f;
							Vector2 up = new Vector2(0.0f, -1.0f);
							Vector2 currentVelocity = m_Rigidbody.LinearVelocity;

							currentVelocity = (currentVelocity.Normalize() + (up * deflectionFactor)).Normalize() * Speed;
							m_Rigidbody.LinearVelocity = currentVelocity;
						}
						m_Audio.PlayAudio();
						collisionHandled = true;
						break;
					}
			}

			return collisionHandled;
		}

		bool OnlineCollision(UInt16 userSlot , ulong otherEntity)
		{
			Entity otherEntityInstance = CreateEntityWithID(otherEntity);

			bool collisionHandled = false;
			switch (otherEntityInstance.GetComponent<TagComponent>().Tag)
			{
				case "Left Wall":
				{
					if (userSlot == 0)
					{
						PlayerTwoScore++;
						UserInterface.SetWidgetText("base_window", "score_player_2", PlayerTwoScore.ToString());
						m_Rigidbody.LinearVelocity *= 0;
						m_Audio.PlayAudio("lose_sound");
						UserInterface.SetDisplayWindow("pre_game_warning", true);
						InputMode.LoadInputMode("Input/Online_Pre_Start.kginput");
						Network.SignalAll((UInt16)NetworkSignals.EndGame);
						Network.EnableReadyCheck();
						collisionHandled = true;
					}
					else if (userSlot == 1)
					{
						m_Rigidbody.LinearVelocity *= 0;
					}

					break;
				}
				case "Right Wall":
				{
					if (userSlot == 1)
					{
						PlayerOneScore++;
						UserInterface.SetWidgetText("base_window", "score_player_1", PlayerOneScore.ToString());
						m_Rigidbody.LinearVelocity *= 0;
						m_Audio.PlayAudio("lose_sound");
						UserInterface.SetDisplayWindow("pre_game_warning", true);
						InputMode.LoadInputMode("Input/Online_Pre_Start.kginput");
						Network.SignalAll((UInt16)NetworkSignals.EndGame);
						Network.EnableReadyCheck();
						collisionHandled = true;
					}
					else if (userSlot == 0)
					{
						m_Rigidbody.LinearVelocity *= 0;
					}

					break;
				}
				case "Top Wall":
				case "Bottom Wall":
				{
					Vector2 horizontalDirection;
					Vector2 currentVelocity = m_Rigidbody.LinearVelocity;
					if (currentVelocity.X >= 0)
					{
						horizontalDirection = new Vector2(1.0f, 0.0f);
					}
					else
					{
						horizontalDirection = new Vector2(-1.0f, 0.0f);
					}
					currentVelocity = (currentVelocity.Normalize() + (horizontalDirection * 0.1f)).Normalize() * Speed;
					m_Rigidbody.LinearVelocity = currentVelocity;
					m_Audio.PlayAudio();
					collisionHandled = true;
					break;
				}
				case "Player1":
				{
					if (userSlot == 0)
					{
						if (Input.IsKeyDown(KeyCode.W))
						{
							float deflectionFactor = Input.IsKeyDown(KeyCode.LeftShift) ? 0.55f : 0.33f;
							Vector2 up = new Vector2(0.0f, 1.0f);
							Vector2 currentVelocity = m_Rigidbody.LinearVelocity;

							currentVelocity = (currentVelocity.Normalize() + (up * deflectionFactor)).Normalize() * Speed;
							m_Rigidbody.LinearVelocity = currentVelocity;
						}
						if (Input.IsKeyDown(KeyCode.A))
						{
							float deflectionFactor = Input.IsKeyDown(KeyCode.LeftShift) ? 0.55f : 0.33f;
							Vector2 up = new Vector2(0.0f, -1.0f);
							Vector2 currentVelocity = m_Rigidbody.LinearVelocity;

							currentVelocity = (currentVelocity.Normalize() + (up * deflectionFactor)).Normalize() * Speed;
							m_Rigidbody.LinearVelocity = currentVelocity;
						}

					}
					m_Audio.PlayAudio();
					collisionHandled = true;
					break;
				}
				case "Player2":
				{
					if (userSlot == 1)
					{
						if (Input.IsKeyDown(KeyCode.O))
						{
							float deflectionFactor = Input.IsKeyDown(KeyCode.LeftShift) ? 0.55f : 0.33f;
							Vector2 up = new Vector2(0.0f, 1.0f);
							Vector2 currentVelocity = m_Rigidbody.LinearVelocity;

							currentVelocity = (currentVelocity.Normalize() + (up * deflectionFactor)).Normalize() * Speed;
							m_Rigidbody.LinearVelocity = currentVelocity;
						}
						if (Input.IsKeyDown(KeyCode.Semicolon))
						{
							float deflectionFactor = Input.IsKeyDown(KeyCode.LeftShift) ? 0.55f : 0.33f;
							Vector2 up = new Vector2(0.0f, -1.0f);
							Vector2 currentVelocity = m_Rigidbody.LinearVelocity;

							currentVelocity = (currentVelocity.Normalize() + (up * deflectionFactor)).Normalize() * Speed;
							m_Rigidbody.LinearVelocity = currentVelocity;
						}
					}
					m_Audio.PlayAudio();
					collisionHandled = true;
					break;
				}
			}

			return collisionHandled;
		}


		bool OnPhysicsCollision(ulong otherEntity)
		{
			UInt16 userSlot = Network.GetSessionSlot();
			if (userSlot == UInt16.MaxValue)
			{
				return OfflineCollision(otherEntity);
			}
			else
			{
				return OnlineCollision(userSlot , otherEntity);
			}
		}

		bool OnPhysicsCollisionEnd(ulong otherEntity)
		{
			UInt16 userSlot = Network.GetSessionSlot();
			if (userSlot == UInt16.MaxValue)
			{
				return false;
			}

			Entity otherEntityInstance = CreateEntityWithID(otherEntity);
			bool collisionHandled = false;
			switch (otherEntityInstance.GetComponent<TagComponent>().Tag)
			{
				case "Player1":
					{
						if (userSlot == 0)
						{
							Vector3 updateVector = m_Transform.Translation;
							updateVector.X += 0.2f;
							Network.SendAllEntityPhysics(ID, updateVector, m_Rigidbody.LinearVelocity);
						}
						collisionHandled = true;
						break;
					}
				case "Player2":
					{
						if (userSlot == 1)
						{
							Vector3 updateVector = m_Transform.Translation;
							updateVector.X += -0.2f;
							Network.SendAllEntityPhysics(ID, updateVector, m_Rigidbody.LinearVelocity);
						}
						collisionHandled = true;
						break;
					}
			}

			return collisionHandled;
		}
	}
}
