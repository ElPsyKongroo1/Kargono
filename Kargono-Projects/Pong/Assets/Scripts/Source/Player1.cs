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

		void OnCreate()
		{
			m_Transform = GetComponent<TransformComponent>();
			m_Rigidbody = GetComponent<Rigidbody2DComponent>();
		}

		void OnUpdate(float ts)
		{
			bool upperLimit = m_Transform.Translation.Y >= 8.25;
			bool lowerLimit = m_Transform.Translation.Y <= -8.25;

			float speed = Speed;
			Vector3 velocity = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.A) && !lowerLimit) { velocity.Y = -1.0f; }
			else if (Input.IsKeyDown(KeyCode.Q) && !upperLimit) { velocity.Y = 1.0f; }

			velocity *= speed * ts;
			Vector3 translation = Translation + velocity;
			Translation = translation;
		}

	}

	public class Player2 : Entity
	{
		private TransformComponent m_Transform;
		private Rigidbody2DComponent m_Rigidbody;

		public float Speed = 2.0f;

		void OnCreate()
		{
			m_Transform = GetComponent<TransformComponent>();
			m_Rigidbody = GetComponent<Rigidbody2DComponent>();
		}

		void OnUpdate(float ts)
		{

			bool upperLimit = m_Transform.Translation.Y >= 8.25;
			bool lowerLimit = m_Transform.Translation.Y <= -8.25;

			float speed = Speed;
			Vector3 velocity = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.L) && !lowerLimit) { velocity.Y = -1.0f; }
			else if (Input.IsKeyDown(KeyCode.O) && !upperLimit) { velocity.Y = 1.0f; }

			velocity *= speed * ts;
			Vector3 translation = Translation + velocity;
			Translation = translation;
		}

	}

	public class Ball : Entity
	{
		private TransformComponent m_Transform;
		private Rigidbody2DComponent m_Rigidbody;

		public float Speed = 2.0f;

		void OnCreate()
		{
			m_Transform = GetComponent<TransformComponent>();
			m_Rigidbody = GetComponent<Rigidbody2DComponent>();
		}

		void OnUpdate(float ts)
		{
			float speed = Speed;
			Vector2 velocity = Vector2.Zero;
			bool changeVelocity = false;
			if (Input.IsKeyDown(KeyCode.Up)) { 
				velocity.Y = 1.0f;
				changeVelocity = true;
			}
			else if (Input.IsKeyDown(KeyCode.Down))
			{
				velocity.Y = -1.0f;
				changeVelocity = true;
			}

			if (Input.IsKeyDown(KeyCode.Right))
			{
				velocity.X = 1.0f;
				changeVelocity = true;
			}
			else if (Input.IsKeyDown(KeyCode.Left))
			{
				velocity.X = -1.0f;
				changeVelocity = true;
			}

			if (changeVelocity)
			{
				m_Rigidbody.LinearVelocity = velocity * speed;
			}

			
		}

	}
}
