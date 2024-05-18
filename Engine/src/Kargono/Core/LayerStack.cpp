#include "kgpch.h"

#include "Kargono/Core/LayerStack.h"

namespace Kargono 
{

	LayerStack::~LayerStack()
	{
		
	}

	void LayerStack::PushLayer(Application* layer)
	{
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
	}

	void LayerStack::PushOverlay(Application* overlay)
	{
		m_Layers.emplace_back(overlay);
	}

	void LayerStack::PopLayer(Application* layer)
	{
		auto location = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (location != m_Layers.end())
		{
			m_Layers.erase(location);
			m_LayerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(Application* overlay)
	{
		auto location = std::find(m_Layers.begin(), m_Layers.end(), overlay);
		if (location != m_Layers.end())
		{
			m_Layers.erase(location);
		}
	}
}
