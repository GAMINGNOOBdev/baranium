#include "MemoryManager.h"
#include <algorithm>

std::vector<void*> MemoryManager::addresses;

void* MemoryManager::allocate(size_t size)
{
    void* addr = malloc(size);

    if (addr != nullptr)
        addresses.push_back(addr);

    return addr;
}

void MemoryManager::deallocate(void* ptr)
{
    if (ptr == nullptr) return;

    auto iterator = std::find(addresses.begin(), addresses.end(), ptr);
    if (iterator == addresses.end())
        return;

    free(ptr);
    addresses.erase(iterator);
}

void MemoryManager::dealloc_all()
{
    for (auto& ptr : addresses)
        free(ptr);

    addresses.clear();
}