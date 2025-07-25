#pragma once

#include <algorithm>
#include <vector>
#include <cstdint>
#include <string>
#include <sstream>
#include <limits>

namespace Kargono::ECS
{
    template< std::unsigned_integral t_SparseIndex = size_t, std::unsigned_integral t_DenseIndex = size_t>
    class SparseSet
    {
    public:
        //==============================
        // Constants
        //==============================
        static constexpr t_DenseIndex k_InvalidDenseIndex{ std::numeric_limits<t_DenseIndex>::max() };
        static constexpr t_SparseIndex k_InvalidSparseIndex{ std::numeric_limits<t_SparseIndex>::max() };
    public:
        //==============================
        // Constructors/Destructors
        //==============================
        SparseSet(t_SparseIndex maxSparseIndex, t_DenseIndex denseCapacity)
        {
            // Store options
            m_SparseMaxIndex = maxSparseIndex;
            m_DenseCapacity = denseCapacity;

            // Allocate memory
            m_SparseList.resize(m_SparseMaxIndex + 1);
            m_DenseList.resize(m_DenseCapacity);
            m_DenseCount = 0;
        }
        ~SparseSet() = default;
    public:
        //==============================
        // Modify List
        //==============================
        t_DenseIndex InsertElement(t_SparseIndex sparseIndex)
        {
            // Ensure sparse/dense capacities aren't full
            if (sparseIndex > m_SparseMaxIndex || m_DenseCount >= m_DenseCapacity)
            {
                return k_InvalidDenseIndex;
            }

            // Ensure index does not already exist
            if (GetDenseIndex(sparseIndex) != k_InvalidDenseIndex)
            {
                return k_InvalidDenseIndex;
            }

            // Create new element at end of dense list and map sparse list to it
            m_DenseList[m_DenseCount] = sparseIndex;
            m_SparseList[sparseIndex] = m_DenseCount;

            return m_DenseCount++;
        }
        
        t_DenseIndex DeleteElement(t_SparseIndex selectedSparse)
        {
            // Ensure this sparseIndex maps to a denseIndex
            if (GetDenseIndex(selectedSparse) == k_InvalidDenseIndex)
            {
                return k_InvalidDenseIndex;
            }

            // Get corresponding dense index and final element from dense list
            t_DenseIndex selectedDenseIndex{ m_SparseList[selectedSparse] };
            t_SparseIndex lastDenseElement = m_DenseList[m_DenseCount - 1];

            // Replace deleted element w/ ending element and shorten dense list
            m_DenseList[selectedDenseIndex] = lastDenseElement;
            m_SparseList[lastDenseElement] = selectedDenseIndex;
            m_DenseCount--;

            return selectedDenseIndex;
        }
        void Clear()
        {
            // Dense list effectively reset to 0
            m_DenseCount = 0;
        }
    public:
        //==============================
        // Query List
        //==============================
        t_DenseIndex GetDenseIndex(t_SparseIndex sparseIndex)
        {
            // Ensure sparseIndex maps somewhere in sparse list
            if (sparseIndex > m_SparseMaxIndex)
            {
                return k_InvalidDenseIndex;
            }
            t_DenseIndex denseIndex{ m_SparseList[sparseIndex] };

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

        bool HasSparseIndex(t_SparseIndex index) const
        {
            return GetDenseIndex(index) != k_InvalidDenseIndex;
        }

    public:
        //==============================
        // Getters/Setters
        //==============================
        t_DenseIndex GetDenseCount() const
        {
            return m_DenseCount;
        }

        t_DenseIndex GetDenseCapacity() const
        {
            return m_DenseCapacity;
        }

        t_SparseIndex GetSparseMax() const
        {
            return m_SparseMaxIndex;
        }

    public:
        //==============================
        // Debugging Function(s)
        //==============================
        std::string Print()
        {
            std::stringstream ss;

            ss << "Metadata:\n";
            ss << "\tDense Count: " << m_DenseCount << '\n';
            ss << "\tDense Capacity: " << m_DenseCapacity << '\n';
            ss << "\tSparse Max Index: " << m_SparseMaxIndex << '\n';

            ss << "Data:\n";
            ss << "\tDense List (Sparse List Indices):";
            for (t_DenseIndex i = 0; i < m_DenseCount; i++)
            {
                ss << m_DenseList[i] << ' ';
            }
            ss << '\n';

            ss << "\tSparse List (Dense List Indices):";
            for (t_SparseIndex i = 0; i < m_SparseMaxIndex; i++)
            {
                ss << m_SparseList[i] << ' ';
            }
            ss << '\n';

            return ss.str();
        }
    private:
        //==============================
        // Internal Fields
        //==============================
        // Data storage
        std::vector<t_DenseIndex> m_SparseList{};
        std::vector<t_SparseIndex> m_DenseList{};
        // Metadata
        t_DenseIndex m_DenseCount;
        t_DenseIndex m_DenseCapacity; 
        t_SparseIndex m_SparseMaxIndex;  
    };
}

#if 0 // Interaction between sparse sets functions
        // Finds Intersection of this set with s
        // and returns pointer to result.
        SparseSet* Intersection(SparseSet& s)
        {
            // Capacity and max value of result set
            int iCap = std::min(m_ElementCount, s.m_ElementCount);
            int iMaxVal = std::max(s.m_SparseMaxVal, m_SparseMaxVal);

            // Create result set
            SparseSet* result = new SparseSet(iMaxVal, iCap);

            // Find the smaller of two sets
            // If this set is smaller
            if (m_ElementCount < s.m_ElementCount)
            {
                // Search every element of this set in 's'.
                // If found, add it to result
                for (int i = 0; i < m_ElementCount; i++)
                    if (s.Search(m_DenseList[i]) != -1)
                        result->Insert(m_DenseList[i]);
            }
            else
            {
                // Search every element of 's' in this set.
                // If found, add it to result
                for (int i = 0; i < s.m_ElementCount; i++)
                    if (Search(s.m_DenseList[i]) != -1)
                        result->Insert(s.m_DenseList[i]);
            }

            return result;
        }

        // A function to find union of two sets
            // Time Complexity-O(n1+n2)
        SparseSet* SetUnion(SparseSet& s)
        {
            // Find capacity and maximum value for result
            // set.
            int uCap = s.m_ElementCount + m_ElementCount;
            int uMaxVal = std::max(s.m_SparseMaxVal, m_SparseMaxVal);

            // Create result set
            SparseSet* result = new SparseSet(uMaxVal, uCap);

            // Traverse the first set and insert all
            // elements of it in result.
            for (int i = 0; i < m_ElementCount; i++)
                result->Insert(m_DenseList[i]);

            // Traverse the second set and insert all
            // elements of it in result (Note that sparse
            // set doesn't insert an entry if it is already
            // present)
            for (int i = 0; i < s.m_ElementCount; i++)
                result->Insert(s.m_DenseList[i]);

            int stackVal = 8;

            int* ptr = &stackVal;
            int* ptr2 = new int();

            float val1{ 1.0f };

            double* ptrVal1{ (double*)&val1 };
            sizeof(float);
            sizeof(double);


            return result;
        }
#endif