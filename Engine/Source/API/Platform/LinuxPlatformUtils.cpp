#include "kgpch.h"

#include "Kargono/Utility/Time.h"
#include "Kargono/Utility/OSCommands.h"
#include "Kargono/Utility/FileDialogs.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Core/Engine.h"


#if defined(KG_PLATFORM_LINUX) 

#include "API/Platform/LinuxBackendAPI.h"
#include "API/Platform/GlfwBackendAPI.h"

namespace Kargono::Utility
{
    // TODO: Replace function stubs with actual linux implementations
	std::filesystem::path FileDialogs::OpenFile(const char* filter, const char* initialDirectory)
	{
		return {};
	}
	std::filesystem::path FileDialogs::SaveFile(const char* filter, const char* initialDirectory)
	{
		return {};
	}
#if 0
	static void on_open_response (GtkDialog *dialog, int response)
	{
  		if (response == GTK_RESPONSE_ACCEPT)
    	{
      		//GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
      		//g_autoptr(GFile) file = gtk_file_chooser_get_file (chooser);
      		//open_file(file);
        }

  	    gtk_window_destroy (GTK_WINDOW (dialog));
	}
#endif

	std::filesystem::path FileDialogs::ChooseDirectory(const std::filesystem::path& initialPath)
	{
#if 0
		// Initialize GTK
        GtkWidget *dialog;
        GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
        dialog = gtk_file_chooser_dialog_new ("Open Folder", (GtkWindow*)glfwGetWaylandWindow((GLFWwindow*)EngineService::GetActiveWindow().GetNativeWindow()), 
										action, ("_Cancel"),
                                        GTK_RESPONSE_CANCEL, ("_Open"), GTK_RESPONSE_ACCEPT, NULL);

  		gtk_window_present (GTK_WINDOW (dialog));
  		g_signal_connect (dialog, "response", G_CALLBACK(on_open_response), NULL);
#endif
		// Return the selected folder path, or an empty path if canceled
		return {};
	}
}

#endif
