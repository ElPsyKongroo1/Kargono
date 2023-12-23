using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Kargono
{
	public abstract class Component
	{
		public Entity Entity { get; internal set; }
	}

	public class TransformComponent : Component
	{
		public Vector3 Translation
		{
			get
			{
				InternalCalls.TransformComponent_GetTranslation(Entity.ID, out Vector3 translation);
				return translation;
			}
			set { InternalCalls.TransformComponent_SetTranslation(Entity.ID, ref value); }
		}
	}

	public class TagComponent : Component
	{
		public String Tag
		{
			get => InternalCalls.TagComponent_GetTag(Entity.ID);
		}
	}

	public class AudioComponent : Component
	{
		public void PlayAudio()
		{
			InternalCalls.AudioComponent_PlayAudio(Entity.ID);
		}

		public void PlayAudio(String audioTag)
		{
			InternalCalls.AudioComponent_PlayAudioByName(Entity.ID, audioTag);
		}
	}

	public class Rigidbody2DComponent : Component
	{

		public enum BodyType { Static = 0, Dynamic, Kinematic }


		public BodyType Type
		{
			get => InternalCalls.Rigidbody2DComponent_GetType(Entity.ID);
			set => InternalCalls.Rigidbody2DComponent_SetType(Entity.ID, value);
		}
		public Vector2 LinearVelocity
		{
			get
			{
				InternalCalls.Rigidbody2DComponent_GetLinearVelocity(Entity.ID, out Vector2 velocity);
				return velocity;
			}
			set
			{
				InternalCalls.Rigidbody2DComponent_SetLinearVelocity(Entity.ID, ref value);
			}
		}

		public void ApplyLinearImpulse(Vector2 impulse, Vector2 point, bool wake)
		{
			InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(Entity.ID, ref impulse, ref point, wake);
		}

		public void ApplyLinearImpulse(Vector2 impulse, bool wake)
		{
			InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseToCenter(Entity.ID, ref impulse, wake);
		}
	}
}
