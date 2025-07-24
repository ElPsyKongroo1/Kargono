#include "kgpch.h"

#include "Modules/ECSTest/ECSSparseSetTest.h"

namespace Kargono::ECS
{
	SparseSet::SparseSet(SparseIndex maxSparseIndex, DenseIndex denseCapacity)
    {
        // Store options
        m_SparseMaxIndex = maxSparseIndex;
        m_DenseCapacity = denseCapacity;

        // Allocate memory
        m_SparseList.resize(m_SparseMaxIndex + 1);
        m_DenseList.resize(m_DenseCapacity);
        m_DenseCount = 0;
    }
    DenseIndex SparseSet::GetDenseIndex(SparseIndex sparseIndex)
    {
        // Ensure sparseIndex maps somewhere in sparse list
        if (sparseIndex > m_SparseMaxIndex)
        {
            return k_InvalidDenseIndex;
        }
        DenseIndex denseIndex{ m_SparseList[sparseIndex] };

        // Ensure dense index maps somewhere in the dense list
        if (denseIndex >= m_DenseCount)
        {
            return k_InvalidDenseIndex;
        }

        // Ensure dense value matches the sparse index
        if (m_DenseList[denseIndex] != sparseIndex)
        {
            return k_InvalidDenseIndex;
        }

        return denseIndex;
    }
    void SparseSet::InsertElement(SparseIndex sparseIndex)
    {
        // Ensure sparse/dense capacities aren't full
        if (sparseIndex > m_SparseMaxIndex || m_DenseCount >= m_DenseCapacity)
        {
            return;
        }

        // Ensure index does not already exist
        if (GetDenseIndex(sparseIndex) != k_InvalidDenseIndex)
        {
            return;
        }

        // Create new element at end of dense list and map sparse list to it
        m_DenseList[m_DenseCount] = sparseIndex;
        m_SparseList[sparseIndex] = m_DenseCount;
        m_DenseCount++;
    }
    void SparseSet::DeleteElement(SparseIndex selectedSparse)
    {
        // Ensure this sparseIndex maps to a denseIndex
        if (GetDenseIndex(selectedSparse) == k_InvalidDenseIndex)
        {
            return;
        }

        // Get corresponding dense index and final element from dense list
        DenseIndex selectedDenseIndex{ m_SparseList[selectedSparse] };
        SparseIndex lastDenseElement = m_DenseList[m_DenseCount - 1];

        // Replace deleted element w/ ending element and shorten dense list
        m_DenseList[selectedDenseIndex] = lastDenseElement;
        m_SparseList[lastDenseElement] = selectedDenseIndex;
        m_DenseCount--;
    }

    void SparseSet::Clear()
    {
        // Dense list effectively reset to 0
        m_DenseCount = 0;
    }
}