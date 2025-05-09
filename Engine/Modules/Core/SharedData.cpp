#include "kgpch.h"

#include "SharedData.h"

namespace Kargono
{
	SharedData::SharedData(uint8_t* data, size_t size, size_t alignment) : m_Data(data), m_DataSize(size), m_Alignment(alignment) {}
}

