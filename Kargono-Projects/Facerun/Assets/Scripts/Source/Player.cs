using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Kargono;
using static System.Runtime.CompilerServices.RuntimeHelpers;

namespace Sandbox
{
	public class Player : Entity
	{
		private TransformComponent m_Transform;
		private Rigidbody2DComponent m_Rigidbody;

		public float Speed;
		public float Time;

		void OnCreate()
		{
			Console.WriteLine($"Player.OnCreate - {ID}");

			m_Transform = GetComponent<TransformComponent>();
			m_Rigidbody = GetComponent<Rigidbody2DComponent>();

		}

		void OnUpdate(float ts)
		{
			Time += ts;
			//Console.WriteLine($"Player.OnUpdate: {ts}");

			float speed = Speed;
			Vector3 velocity = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.W)) { velocity.Y = 1.0f; }
			else if (Input.IsKeyDown(KeyCode.S)) { velocity.Y = -1.0f; }

			if (Input.IsKeyDown(KeyCode.A)) { velocity.X = -1.0f; }
			else if (Input.IsKeyDown(KeyCode.D)) { velocity.X = 1.0f; }

			Entity cameraEntity = FindEntityByName("Camera");
			if (cameraEntity != null)
			{
				Camera camera = cameraEntity.As<Camera>();

				if (Input.IsKeyDown(KeyCode.U) && camera.DistanceFromPlayer < 3.868f)
				{
					camera.DistanceFromPlayer += speed * ts;
				}
				else if (Input.IsKeyDown(KeyCode.O) && (camera.DistanceFromPlayer - speed * ts) > 0.0f)
				{
					camera.DistanceFromPlayer -= speed * ts;
				}
			}

			velocity *= speed * ts;

			m_Rigidbody.ApplyLinearImpulse(velocity.XY, true);

		}

	}
}
