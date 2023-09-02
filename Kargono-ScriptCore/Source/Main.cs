using System;
using System.Runtime.CompilerServices;

namespace Kargono
{
	public struct Vector3
	{
		public float X, Y, Z;
		public static Vector3 Zero => new Vector3(0.0f);

		public Vector3(float scalar)
		{
			X = scalar;
			Y = scalar;
			Z = scalar;
		}

		public Vector3(float x, float y, float z)
		{
			X = x;
			Y = y;
			Z = z;
		}

		public static Vector3 operator +(Vector3 a, Vector3 b)
		{
			return new Vector3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
		}
		public static Vector3 operator*(Vector3 vector, float scalar)
		{
			return new Vector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
		}
	}

	public static class InternalCalls
	{
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void NativeLog(string text, int parameter);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void NativeLog_Vector(ref Vector3 parameter, out Vector3 result);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float NativeLog_VectorDot(ref Vector3 parameter);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Entity_GetTranslation(ulong entityID, out Vector3 parameter);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Entity_SetTranslation(ulong entityID, ref Vector3 parameter);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyDown(KeyCode keycode);
	}

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
				InternalCalls.Entity_GetTranslation(ID, out Vector3 translation);
				return translation;
			}

			set
			{
				InternalCalls.Entity_SetTranslation(ID, ref value);
			}
		}

	}
}
