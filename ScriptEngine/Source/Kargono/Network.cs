using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Kargono
{
	public class Network
	{
		public static void RequestJoinSession()
		{
			InternalCalls.Network_RequestJoinSession();
		}

		public static void RequestUserCount()
		{
			InternalCalls.Network_RequestUserCount();
		}

		public static UInt16 GetSessionSlot()
		{
			return InternalCalls.Network_GetSessionSlot();
		}

		public static void LeaveCurrentSession()
		{
			InternalCalls.Network_LeaveCurrentSession();
		}

		public static void EnableReadyCheck()
		{
			InternalCalls.Network_EnableReadyCheck();
		}

		public static void SessionReadyCheck()
		{
			InternalCalls.Network_SessionReadyCheck();
		}

		public static void SendAllEntityLocation(ulong id, Vector3 translation)
		{
			InternalCalls.Network_SendAllEntityLocation(id, ref translation);
		}

		public static void SendAllEntityPhysics(ulong id, Vector3 translation, Vector2 linearVelocity)
		{
			InternalCalls.Network_SendAllEntityPhysics(id, ref translation, ref linearVelocity);
		}

		public static void SignalAll(UInt16 signal)
		{
			InternalCalls.Network_SignalAll(signal);
		}

	}

}
