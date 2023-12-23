using Kargono;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptEngine.Source.Kargono
{
	public class Scene
	{
		public static void TransitionScene(string sceneFileLocation)
		{
			InternalCalls.Scene_TransitionScene(sceneFileLocation);
		}
	}
}
