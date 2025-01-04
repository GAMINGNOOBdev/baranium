#ifndef __BINARIES__NAMELOOKUPTABLE_H_
#define __BINARIES__NAMELOOKUPTABLE_H_ 1

#include "../BgeFile.hpp"
#include <stdint.h>
#include <string>
#include <vector>

typedef int64_t index_t;

namespace Binaries
{

    /**
     * @brief An entry of a `NameLookupTable`
     */
    struct NameLookupTableEntry
    {
        // normally name length + name data but since it's c++ rn, just use std::string
        std::string Name;
        index_t ID;
    };

    /**
     * @brief A table containing names and their respective internal ID
     */
    struct NameLookupTable
    {
        /**
         * @brief Construct a new `NameLookupTable`
         */
        NameLookupTable();

        /**
         * @brief Look for a specific entry
         * 
         * @param name Name of the entry
         * @returns The id of the entry
         */
        index_t Lookup(std::string name);

        /**
         * @brief Look for a specific entry
         * 
         * @param id ID of the entry
         * @returns The name of the entry
         */
        std::string Lookup(index_t id);

        /**
         * @brief Add a name entry
         * 
         * @param name Name
         * @param ID ID
         */
        void AddEntry(std::string name, index_t ID);

        /**
         * @brief Write all entries to a file
         * 
         * @param file The file that will be written to
         */
        void Write(BgeFile& file);

    private:
        // The number of names that are saved
        uint64_t mEntryCount;

        // Name entries, can be directly written to a file using fwrite
        std::vector<NameLookupTableEntry> mEntries;
    };

}

#endif
