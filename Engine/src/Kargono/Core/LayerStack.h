#pragma once

#include "Kargono/Core/Application.h"

#include <vector>

namespace Kargono 
{
	class LayerStack 
	{
	public:
		LayerStack() = default;
		~LayerStack();
		
		void PushLayer(Application* layer);
		void PushOverlay(Application* overlay);
		void PopLayer(Application* layer);
		void PopOverlay(Application* overlay);

		std::vector<Application*>& GetLayers() { return m_Layers; }

		std::vector<Application*>::iterator begin() { return m_Layers.begin(); }
		std::vector<Application*>::iterator end() { return m_Layers.end(); }
		std::vector<Application*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
		std::vector<Application*>::reverse_iterator rend() { return m_Layers.rend(); }

		std::vector<Application*>::const_iterator begin() const { return m_Layers.begin(); }
		std::vector<Application*>::const_iterator end()	const { return m_Layers.end(); }
		std::vector<Application*>::const_reverse_iterator rbegin() const { return m_Layers.rbegin(); }
		std::vector<Application*>::const_reverse_iterator rend() const { return m_Layers.rend(); }
	private:
		std::vector<Application*> m_Layers;
		uint32_t m_LayerInsertIndex = 0;
	};
}
