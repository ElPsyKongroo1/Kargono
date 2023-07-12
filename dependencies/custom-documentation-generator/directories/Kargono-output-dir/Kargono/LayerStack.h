
#pragma once


#include "Kargono/Core.h"

#include "Kargono/Layer.h"


namespace Kargono 
{

	class KG_API LayerStack 
	{

	public:
/// @brief Default constructor for the LayerStack class
		LayerStack();
/// @brief Destructor for the LayerStack class
		~LayerStack();
		
/// @brief Pushes a layer onto the layer stack
/// @param layer The layer to be pushed
		void PushLayer(Layer* layer);
/// @brief Pushes an overlay onto the layer stack
/// @param overlay The overlay to be pushed
		void PushOverlay(Layer* overlay);
/// @brief Pops a layer from the layer stack
/// @param layer The layer to be popped
		void PopLayer(Layer* layer);
/// @brief Pops an overlay from the layer stack
/// @param overlay The overlay to be popped
		void PopOverlay(Layer* overlay);

/// @brief Returns an iterator pointing to the beginning of the layer stack
/// @return Iterator pointing to the beginning of the layer stack
		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
/// @brief Returns an iterator pointing to the end of the layer stack
/// @return Iterator pointing to the end of the layer stack
		std::vector<Layer*>::iterator end() { return m_Layers.end(); }

	private:

		std::vector<Layer*> m_Layers;

		std::vector<Layer*>::iterator m_LayerInsert;
	};
}