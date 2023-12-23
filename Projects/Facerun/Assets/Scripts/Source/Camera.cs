using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Kargono;
using static System.Runtime.CompilerServices.RuntimeHelpers;

namespace Sandbox
{
	public class Camera : Entity
	{
		public Entity OtherEntity;

		public float DistanceFromPlayer = 5.0f;

		private float distanceFromPlayer = 0.0f;

		private Entity m_Player;

		void OnCreate()
		{
			m_Player = FindEntityByName("Player");
		}

		private float Lerp(float a, float b, float t)
		{
			return a * (1.0f - t) + (b * t);
		}

		void OnUpdate(float ts)
		{
			if (m_Player == null)
			{
				return;
			}

			Vector2 playerVelocity = m_Player.GetComponent<Rigidbody2DComponent>().LinearVelocity;
			float target = DistanceFromPlayer + playerVelocity.Length();
			distanceFromPlayer = Lerp(distanceFromPlayer, target, 3.0f * ts);
			Translation = new Vector3(m_Player.Translation.X, m_Player.Translation.Y + 1.25f, distanceFromPlayer);
		}

	}
}
