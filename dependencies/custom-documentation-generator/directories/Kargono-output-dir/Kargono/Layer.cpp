#include "Kargono/kgpch.h"
#include "Kargono/Layer.h"

/// @namespace Kargono
namespace Kargono 
{
/// @brief Constructor for the Layer class
/// @param debugName The debug name of the layer
	Layer::Layer(const std::string& debugName) : m_DebugName(debugName)
	{}

/// @brief Destructor for the Layer class
	Layer::~Layer() 
	{}
}