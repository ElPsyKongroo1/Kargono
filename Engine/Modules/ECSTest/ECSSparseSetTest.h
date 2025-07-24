#pragma once

#include <algorithm>
#include <vector>
#include <cstdint>

namespace Kargono::ECS
{
    using DenseIndex = size_t;
    using SparseIndex = size_t;

    constexpr DenseIndex k_InvalidDenseIndex{ std::numeric_limits<DenseIndex>::max()};
    constexpr SparseIndex k_InvalidSparseIndex{ std::numeric_limits<SparseIndex>::max()};

    class SparseSet
    {
    public:
        //==============================
        // Constructors/Destructors
        //==============================
        SparseSet(SparseIndex maxSparseIndex, DenseIndex denseCapacity);
        ~SparseSet() = default;
    public:
        //==============================
        // Modify List
        //==============================
        void InsertElement(SparseIndex sparseIndex);
        void DeleteElement(SparseIndex x);
        void Clear();
    public:
        //==============================
        // Query List
        //==============================
        DenseIndex GetDenseIndex(SparseIndex sparseIndex);
    private:
        //==============================
        // Internal Fields
        //==============================
        // Data storage
        std::vector<DenseIndex> m_SparseList{};
        std::vector<SparseIndex> m_DenseList{};
        // Metadata
        DenseIndex m_DenseCount;
        DenseIndex m_DenseCapacity; 
        SparseIndex m_SparseMaxIndex;  
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