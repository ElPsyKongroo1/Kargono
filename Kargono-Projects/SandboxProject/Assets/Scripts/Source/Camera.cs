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

		private Entity m_Player;

		void OnCreate()
		{
			m_Player = FindEntityByName("Player");
		}

		void OnUpdate(float ts)
		{
			if (m_Player != null)
			{
				Translation = new Vector3(m_Player.Translation.X, m_Player.Translation.Y + 1.25f, DistanceFromPlayer);
			}

			float speed = 1.0f;
			Vector3 velocity = Vector3.Zero;

			velocity *= speed;
			

			Vector3 translation = Translation;
			translation += velocity * ts;
			Translation = translation;
		}

	}
}
