/* Start Header ************************************************************************/
/*!
\file       memoryManager.h
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

#include "memoryManager.h"
#include <new>
#include <cassert>

MemoryManager::MemoryManager(size_t objSize, size_t objsPerPage)
    : objectSize(objSize),
    objectsPerPage(objsPerPage),
    pageList(nullptr)
{
    //allocate first page at load time
    AllocateNewPage();
}

MemoryManager::~MemoryManager()
{
    MemoryPage* page = pageList;

    while (page)
    {
        MemoryPage* next = page->next;
        operator delete(page->data);
        delete page;
        page = next;
    }
}

void MemoryManager::AllocateNewPage()
{
    MemoryPage* page = new MemoryPage();
    page->data = operator new(objectSize * objectsPerPage);
    page->freeList = nullptr;
    page->next = pageList;
    pageList = page;

    char* start = static_cast<char*>(page->data);

    // Build free list
    for (size_t i = 0; i < objectsPerPage; ++i)
    {
        FreeNode* node = reinterpret_cast<FreeNode*>(start + i * objectSize);
        node->next = page->freeList;
        page->freeList = node;
    }
}

void* MemoryManager::Allocate()
{
    MemoryPage* page = pageList;

    // Find page with free memory
    while (page && page->freeList == nullptr)
        page = page->next;

    // No free memory → create new page
    if (!page)
    {
        AllocateNewPage();
        page = pageList;
    }

    // Pop from free list
    FreeNode* node = page->freeList;
    page->freeList = node->next;

    return node;
}

void MemoryManager::Deallocate(void* ptr)
{
    assert(ptr != nullptr);

    //push back into free list of first page
    FreeNode* node = static_cast<FreeNode*>(ptr);
    node->next = pageList->freeList;
    pageList->freeList = node;
}
