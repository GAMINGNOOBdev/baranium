#ifndef __MEMORYMANAGER_H_
#define __MEMORYMANAGER_H_ 1

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <memory.h>
#include <vector>

class MemoryManager
{
public:

    /**
     * @brief allocate memory
     * 
     * @param size memory size
     * 
     * @returns a pointer to the memory location
     */
    static void* allocate(size_t size);

    /**
     * @brief deallocate a memory pointer
     * 
     * @note only deallocated if allocated by the memory manager
     * 
     * @param ptr memory pointer
     */
    static void deallocate(void* ptr);

    /**
     * @brief deallocate all allocated memory
     */
    static void dealloc_all();

private:
    static std::vector<void*> addresses;
};

#endif