#include "NameLookupTable.h"
#include <algorithm>

namespace Binaries
{

    NameLookupTable::NameLookupTable()
        : mEntries(), mEntryCount(0)
    {
    }

    index_t NameLookupTable::Lookup(std::string name)
    {
        auto iterator = std::find_if(mEntries.begin(), mEntries.end(), [name](NameLookupTableEntry& entry){
            return entry.Name == name;
        });

        if (iterator == mEntries.end())
            return -1;
        
        return (*iterator).ID;
    }

    std::string NameLookupTable::Lookup(index_t id)
    {
        auto iterator = std::find_if(mEntries.begin(), mEntries.end(), [id](NameLookupTableEntry& entry){
            return entry.ID == id;
        });

        if (iterator == mEntries.end())
            return "";

        return (*iterator).Name;
    }

    void NameLookupTable::AddEntry(std::string name, index_t ID)
    {
        if (!Lookup(ID).empty())
            return;

        mEntries.push_back( NameLookupTableEntry{std::string(name), ID} );
        mEntryCount++;
    }

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
