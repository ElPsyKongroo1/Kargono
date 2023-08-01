#include "Kargono/kgpch.h"
#include "Kargono/Core/LayerStack.h"

namespace Kargono 
{

	LayerStack::~LayerStack()
	{
		for (Layer* layer : m_Layers)
		{
			if (layer) 
			{
				layer->OnDetach();
				delete layer;
				layer = nullptr;
			}
		}
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		m_Layers.emplace_back(overlay);
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		auto location = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (location != m_Layers.end())
		{
			m_Layers.erase(location);
			m_LayerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(Layer* overlay)
	{
		auto location = std::find(m_Layers.begin(), m_Layers.end(), overlay);
		if (location != m_Layers.end())
		{
			m_Layers.erase(location);
		}
	}
}