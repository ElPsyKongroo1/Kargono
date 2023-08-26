#include <Kargono.h>
#include "Kargono/Core/EntryPoint.h"
#include "EditorLayer.h"

namespace Kargono {

	class KargonoEditor : public Application
	{
	public:
		KargonoEditor(ApplicationCommandLineArgs args)
			: Application("Kargono Editor", args)
		{
			//PushLayer(new ExampleLayer()); 
			PushLayer(new EditorLayer());
		}
		~KargonoEditor()
		{

		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		return new KargonoEditor(args);
	}
}
