using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Kargono
{
	public class Audio
	{
		public static void PlayAudio(string audioFileLocation)
		{
			InternalCalls.Audio_PlayAudio(audioFileLocation);
		}

		public static void PlayStereoAudio(string audioFileLocation)
		{
			InternalCalls.Audio_PlayStereoAudio(audioFileLocation);
		}
	}

}
