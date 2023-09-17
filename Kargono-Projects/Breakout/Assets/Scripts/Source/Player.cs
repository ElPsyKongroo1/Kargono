using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Kargono;
using static System.Runtime.CompilerServices.RuntimeHelpers;

namespace Breakout
{
	public class Player : Entity
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
			Vector3 velocity = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.J)) { velocity.X = -1.0f; }
			else if (Input.IsKeyDown(KeyCode.L)) { velocity.X = 1.0f; }

			velocity *= speed * ts;
			Vector3 translation = Translation + velocity;
			Translation = translation;
		}

	}
}
