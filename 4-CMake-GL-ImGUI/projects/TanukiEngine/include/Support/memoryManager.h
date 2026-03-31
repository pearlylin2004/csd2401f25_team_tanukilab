/* Start Header ************************************************************************/
/*!
\file       memoryManager.cpp
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       January, 29th, 2026
\brief      This file implements a simple custom memory allocator that uses a free list
 * and memory pages to efficiently allocate and deallocate fixed-size objects.

Copyright (C) 2026 DigiPen Institute of Technology.
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
    // Construct a memory manager for objects of a fixed size.
    // Parameters:
    //   objectSize: size of each object in bytes
    //   objectsPerPage: number of objects to allocate per memory page
    MemoryManager(size_t objectSize, size_t objectsPerPage);

    // Destructor releases all allocated memory pages
    ~MemoryManager();

    // Allocate memory for one object from the pool
    // Returns pointer to the memory block
    void* Allocate();

    // Return memory block back to the pool
    // Parameter:
    //  ptr: pointer previously returned by Allocate()
    void  Deallocate(void* ptr);   // Return memory back to pool

private:
    // Represents a page of memory holding multiple objects
    struct MemoryPage
    {
        void* data;                // Raw memory block
        FreeNode* freeList;        // Free slots in this page
        MemoryPage* next;          // Next page
    };

    // Allocate a new memory page and add its blocks to the free list
    void AllocateNewPage();

    size_t objectSize;
    size_t objectsPerPage;

    MemoryPage* pageList;
};