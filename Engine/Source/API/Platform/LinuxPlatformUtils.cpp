#include "kgpch.h"

#include "Kargono/Utility/Time.h"
#include "Kargono/Utility/OSCommands.h"
#include "Kargono/Utility/FileDialogs.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Core/Engine.h"

#include <thread>
#include <mutex>
#include <condition_variable>

#if defined(KG_PLATFORM_LINUX) 

#include "API/Platform/LinuxBackendAPI.h"
#include "API/Platform/GlfwBackendAPI.h"

namespace Kargono::Utility
{
#if 0
		// Custom 'destroy' signal handler
	static void window_destroy(GtkWidget *widget, gpointer app)
	{
		// This function will be called when the window is closed.
		// To prevent the application from quitting automatically, we do nothing here.
		// The application won't exit until we explicitly call g_application_quit() elsewhere.
	}

	static void activate (GtkApplication* app, gpointer user_data)
	{
		GtkWidget *window;

		window = gtk_application_window_new (app);
		gtk_window_set_title (GTK_WINDOW (window), "A typical window");
		gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);

		// Connect the 'destroy' signal to a custom handler
    	g_signal_connect(window, "destroy", G_CALLBACK(window_destroy), app);
		
		gtk_window_present (GTK_WINDOW (window));
	}

	// Gtk specific context
	class DialogContext
	{
	public:
		DialogContext() = default;
		~DialogContext()
		{
			if (m_AppRunning)
			{
				// Close thread and close context
			}
		}
	public:
		void InitApp()
		{
			// Start running gtk app
			m_AppThread = std::thread(&DialogContext::RunApp, this);
			m_AppRunning = true;
		}

		void AddWindow()
		{
			GtkWidget *window;
			static int counter {0};
			counter++;
			std::string windowTitle {"A typical window"};
			windowTitle.append(std::to_string(counter));
			window = gtk_application_window_new (m_App);
			gtk_window_set_title (GTK_WINDOW (window), windowTitle.c_str());
			gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);

			// Connect the 'destroy' signal to a custom handler
			g_signal_connect(window, "destroy", G_CALLBACK(window_destroy), m_App);
			
			gtk_window_present (GTK_WINDOW (window));
		}

		void RunApp()
		{
			// Initialize app
			m_App = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
			g_signal_connect(m_App, "activate", G_CALLBACK(activate), NULL);

			// Run app
			g_application_run(G_APPLICATION(m_App), 0, NULL);
			g_object_unref(m_App);
		}
	public:
		GtkApplication* m_App {nullptr};
		std::thread m_AppThread;
		std::filesystem::path m_Result {};
		bool m_AppRunning { false };
	};

	static DialogContext s_DialogContext;
#endif

    // TODO: Replace function stubs with actual linux implementations
	std::filesystem::path FileDialogs::OpenFile(const char* filter, const char* initialDirectory)
	{
		return {};
	}
	std::filesystem::path FileDialogs::SaveFile(const char* filter, const char* initialDirectory)
	{
		return {};
	}

	std::filesystem::path FileDialogs::ChooseDirectory(const std::filesystem::path& initialPath)
	{
		#if 0
		// Start dialog context if not already started
		if (!Kargono::Utility::s_DialogContext.m_AppRunning)
		{
			s_DialogContext.InitApp();
		}

		// Open choose directory dialog
		s_DialogContext.AddWindow();

		// Wait on dialog to finish

		// Return the selected folder path, or an empty path if canceled
		return s_DialogContext.m_Result;
		#endif
		return {};
	}
}

#endif
