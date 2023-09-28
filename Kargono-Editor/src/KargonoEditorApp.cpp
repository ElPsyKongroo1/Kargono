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
		spec.WorkingDirectory = std::filesystem::current_path();
		spec.Width = 1600;
		spec.Height = 900;

		return new KargonoEditor(spec);
	}
}
