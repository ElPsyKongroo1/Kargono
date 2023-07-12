#include "Kargono/kgpch.h"
#include "Kargono/LayerStack.h"

namespace Kargono 
{
/// @brief Constructor for LayerStack class
	LayerStack::LayerStack() 
	{
		m_LayerInsert = m_Layers.begin();
	}

/// @brief Destructor for LayerStack class
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
/// @param layer A pointer to the layer object
	void LayerStack::PushLayer(Layer* layer)
	{
		m_LayerInsert = m_Layers.emplace(m_LayerInsert, layer);
	}

/// @brief Pushes an overlay onto the layer stack
/// @param overlay A pointer to the overlay object
	void LayerStack::PushOverlay(Layer* overlay)
	{
		m_Layers.emplace_back(overlay);
	}

/// @brief Pops a layer from the layer stack
/// @param layer A pointer to the layer object
	void LayerStack::PopLayer(Layer* layer)
	{
		auto location = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (location != m_Layers.end())
		{
			m_Layers.erase(location);
			m_LayerInsert--;
		}
	}

/// @brief Pops an overlay from the layer stack
/// @param overlay A pointer to the overlay object
	void LayerStack::PopOverlay(Layer* overlay)
	{
		auto location = std::find(m_Layers.begin(), m_Layers.end(), overlay);
		if (location != m_Layers.end())
		{
			m_Layers.erase(location);
		}
	}
}