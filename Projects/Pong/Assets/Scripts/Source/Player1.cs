using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Kargono;
using static System.Runtime.CompilerServices.RuntimeHelpers;

namespace Pong
{
	public class Player1 : Entity
	{
		private TransformComponent m_Transform;
		private Rigidbody2DComponent m_Rigidbody;

		public float Speed = 2.0f;
		private float SpeedUpFactor = 2.0f;

		void OnCreate()
		{
			m_Transform = GetComponent<TransformComponent>();
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
		}

	}

	public class Player2 : Entity
	{
		private TransformComponent m_Transform;
		private Rigidbody2DComponent m_Rigidbody;

		public float Speed = 2.0f;
		private float SpeedUpFactor = 2.0f;

		void OnCreate()
		{
			m_Transform = GetComponent<TransformComponent>();
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
		}

	}

	public class Ball : Entity
	{
		private AudioComponent m_Audio;
		private TransformComponent m_Transform;
		private Rigidbody2DComponent m_Rigidbody;

		public float Speed = 2.0f;
		public bool ChangeVelocity = false;

		void OnCreate()
		{
			m_Audio = GetComponent<AudioComponent>();
			m_Transform = GetComponent<TransformComponent>();
			m_Rigidbody = GetComponent<Rigidbody2DComponent>();
		}

		void MoveUp()
		{
			Vector2 velocity = Vector2.Zero;
			velocity.Y = 1.0f;
			m_Rigidbody.LinearVelocity = velocity * Speed;
			
		}

		void MoveDown()
		{
			Vector2 velocity = Vector2.Zero;
			velocity.Y = -1.0f;
			m_Rigidbody.LinearVelocity = velocity * Speed;
			
		}

		void MoveLeft()
		{
			Vector2 velocity = Vector2.Zero;
			velocity.X = -1.0f;
			m_Rigidbody.LinearVelocity = velocity * Speed;
		}

		void MoveRight()
		{
			Vector2 velocity = Vector2.Zero;
			velocity.X = 1.0f;
			m_Rigidbody.LinearVelocity = velocity * Speed;
		}

		void OnUpdate(float ts)
		{
		}

		bool OnPhysicsCollision(ulong otherEntity)
		{
			Entity otherEntityInstance = CreateEntityWithID(otherEntity);

			bool collisionHandled = false;
			switch (otherEntityInstance.GetComponent<TagComponent>().Tag)
			{
				case "Left Wall":
				case "Right Wall":
				{
					m_Rigidbody.LinearVelocity *= 0;
					m_Audio.PlayAudio("lose_sound");
					collisionHandled =  true;
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

			//if (collisionHandled)
			//{
			//	m_Audio.PlayAudio();
			//	return true;
			//}

			return false;
		}



	}
}
