#include <Kargono.h>
#include "Kargono/Core/EntryPoint.h"
#include "EditorLayer.h"

namespace Kargono {

	class KargonoEditor : public Application
	{
	public:
		KargonoEditor()
			: Application("Kargono Editor")
		{
			//PushLayer(new ExampleLayer()); 
			PushLayer(new EditorLayer());
		}
		~KargonoEditor()
		{

		}
	};

	Application* CreateApplication()
	{
		return new KargonoEditor();
	}
}