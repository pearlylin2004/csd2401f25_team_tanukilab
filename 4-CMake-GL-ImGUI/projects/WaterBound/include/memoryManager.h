/* Start Header ************************************************************************/
/*!
\file       memoryManager.cpp
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       January, 29th, 2025
\brief

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include <cstddef> //size_t

//node for free list to link free blocks
struct FreeNode{
    FreeNode* next;
};

class MemoryManager {
public:
    MemoryManager(size_t objectSize, size_t objectsPerPage);
    ~MemoryManager();

    void* Allocate();              // Allocate memory for ONE object
    void  Deallocate(void* ptr);   // Return memory back to pool

private:
    struct MemoryPage
    {
        void* data;                // Raw memory block
        FreeNode* freeList;        // Free slots in this page
        MemoryPage* next;          // Next page
    };

    void AllocateNewPage();

    size_t objectSize;
    size_t objectsPerPage;

    MemoryPage* pageList;
};