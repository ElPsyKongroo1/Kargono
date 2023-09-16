#include <Kargono.h>
#include "Kargono/Core/EntryPoint.h"
#include "EditorLayer.h"

namespace Kargono {

	class KargonoEditor : public Application
	{
	public:
		KargonoEditor(const ApplicationSpecification& spec)
			: Application(spec)
		{
			PushLayer(new EditorLayer());
		}
		~KargonoEditor() = default;
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification spec;
		spec.Name = "Kargono Editor";
		spec.CommandLineArgs = args;

		return new KargonoEditor(spec);
	}
}
