#include "NameLookupTable.h"

namespace Binaries
{

    /**
     * @brief Construct a new `NameLookupTable`
     */
    NameLookupTable::NameLookupTable()
        : mEntries(), mEntryCount(0)
    {
    }

    /**
     * @brief Add a name entry
     * 
     * @param name Name
     * @param ID ID
     */
    void NameLookupTable::AddEntry(std::string name, index_t ID)
    {
        mEntries.push_back( NameLookupTableEntry{std::string(name), ID} );
        mEntryCount++;
    }

    /**
     * @brief Write all entries to a file
     * 
     * @param file The file that will be written to
     */
    void NameLookupTable::Write(BgeFile& file)
    {
        file.Write<uint64_t>(mEntryCount);
        for (auto entry : mEntries)
        {
            file.Write<uint8_t>(entry.Name.length()+1);
            file.Write(entry.Name.data(), sizeof(char), entry.Name.length());
            file.Write<uint8_t>(0);
            file.Write<index_t>(entry.ID);
        }
    }

}