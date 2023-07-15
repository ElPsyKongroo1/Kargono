#include "Kargono/kgpch.h"
#include "Kargono/LayerStack.h"

namespace Kargono 
{
/// @brief Constructor for the LayerStack class
	LayerStack::LayerStack() 
	{
	}

/// @brief Destructor for the LayerStack class
	LayerStack::~LayerStack()
	{
		for (Layer* layer : m_Layers)
		{
			if (layer) 
			{
				delete layer;
				layer = nullptr;
			}
		}
	}

/// @brief Pushes a layer onto the layer stack
///
/// @param layer The layer to be pushed onto the stack
	void LayerStack::PushLayer(Layer* layer)
	{
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
	}

/// @brief Pushes an overlay onto the layer stack
///
/// @param overlay The overlay to be pushed onto the stack
	void LayerStack::PushOverlay(Layer* overlay)
	{
		m_Layers.emplace_back(overlay);
	}

/// @brief Pops a layer from the layer stack
///
/// @param layer The layer to be popped from the stack
	void LayerStack::PopLayer(Layer* layer)
	{
		auto location = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (location != m_Layers.end())
		{
			m_Layers.erase(location);
			m_LayerInsertIndex--;
		}
	}

/// @brief Pops an overlay from the layer stack
///
/// @param overlay The overlay to be popped from the stack
	void LayerStack::PopOverlay(Layer* overlay)
	{
		auto location = std::find(m_Layers.begin(), m_Layers.end(), overlay);
		if (location != m_Layers.end())
		{
			m_Layers.erase(location);
		}
	}
}