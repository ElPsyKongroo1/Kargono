#pragma once

#include "Kargono/Core.h"
#include "Kargono/Layer.h"

namespace Kargono 
{
/// @brief The LayerStack class represents a stack of layers in the game engine
	class KG_API LayerStack 
	{

	public:
/// @brief Default constructor for the LayerStack class
		LayerStack();
/// @brief Destructor for the LayerStack class
		~LayerStack();
		
/// @brief Pushes a layer onto the layer stack
/// @param layer A pointer to the layer to push onto the stack
		void PushLayer(Layer* layer);
/// @brief Pushes an overlay onto the layer stack
/// @param overlay A pointer to the overlay to push onto the stack
		void PushOverlay(Layer* overlay);
/// @brief Pops a layer from the layer stack
/// @param layer A pointer to the layer to pop from the stack
		void PopLayer(Layer* layer);
/// @brief Pops an overlay from the layer stack
/// @param overlay A pointer to the overlay to pop from the stack
		void PopOverlay(Layer* overlay);

/// @brief Returns an iterator pointing to the beginning of the layer stack
/// @return An iterator pointing to the beginning of the layer stack
		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
/// @brief Returns an iterator pointing to the end of the layer stack
/// @return An iterator pointing to the end of the layer stack
		std::vector<Layer*>::iterator end() { return m_Layers.end(); }

	private:
/// @brief A vector holding pointers to layers in the layer stack
		std::vector<Layer*> m_Layers;
/// @brief The index where the next layer will be inserted into the layer stack
		unsigned int m_LayerInsertIndex = 0;
	};
}