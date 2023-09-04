using System;
using System.Runtime.CompilerServices;

namespace Kargono
{
	public class Entity
	{
		protected Entity() { ID = 0;}

		internal Entity(ulong id)
		{
			ID = id;
		}

		public readonly ulong ID;

		public Vector3 Translation
		{
			get
			{
				return GetComponent<TransformComponent>().Translation;
			}

			set
			{
				InternalCalls.Entity_SetTranslation(ID, ref value);
			}
		}

		public bool HasComponent<T>() where T : Component, new()
		{
			Type componentType = typeof(T);
			return InternalCalls.Entity_HasComponent();
		}

		public TransformComponent GetComponent<T>() where T : Component, new()
		{
			if (!HasComponent<T>())
				return null;
			
		}

	}
}
