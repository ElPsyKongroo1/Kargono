using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Kargono
{
	public class GameState
	{
		public static T GetField<T>(string fieldName) where T : struct // Ensure T is a value type
		{
			IntPtr dataPtr = InternalCalls.GameState_GetField(fieldName);

			// Determine the type of T and use the appropriate Marshal method to dereference the pointer
			if (typeof(T) == typeof(int))
			{
				return (T)(object)Marshal.ReadInt32(dataPtr); // Cast to object first, then to T
			}
			else if (typeof(T) == typeof(short))
			{
				return (T)(object)Marshal.ReadInt16(dataPtr);
			}
			else if (typeof(T) == typeof(long))
			{
				return (T)(object)Marshal.ReadInt64(dataPtr);
			}
			else if (typeof(T) == typeof(byte))
			{
				return (T)(object)Marshal.ReadByte(dataPtr);
			}
			else if (typeof(T) == typeof(uint))
			{
				return (T)(object)(uint)Marshal.ReadInt32(dataPtr);
			}
			else if (typeof(T) == typeof(ushort))
			{
				return (T)(object)(ushort)Marshal.ReadInt16(dataPtr);
			}
			else if (typeof(T) == typeof(ulong))
			{
				return (T)(object)(ulong)Marshal.ReadInt64(dataPtr);
			}
			else
			{
				throw new InvalidOperationException("Unsupported type");
			}
		}

		public static void SetField<T>(string fieldName, T value) where T : struct
		{
			// Calculate the size of the value type T
			int size = Marshal.SizeOf<T>();

			// Allocate unmanaged memory for the value
			IntPtr ptr = Marshal.AllocHGlobal(size);

			try
			{
				// Copy the value to the allocated memory
				Marshal.StructureToPtr(value, ptr, false);

				// Pass the field name and the memory pointer to the internal function
				InternalCalls.GameState_SetField(fieldName, ptr);
			}
			finally
			{
				// Free the allocated memory
				Marshal.FreeHGlobal(ptr);
			}
		}
	}

}
