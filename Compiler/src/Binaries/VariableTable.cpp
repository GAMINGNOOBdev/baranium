#include "VariableTable.h"
#include <algorithm>

namespace Binaries
{

    VariableTable::VariableTable()
        : mEntries(), mEntryCount(0)
    {
    }

    void VariableTable::Clear()
    {
        mEntries.clear();
        mEntryCount = 0;
    }

    index_t VariableTable::Lookup(std::string name)
    {
        auto iterator = std::find_if(mEntries.begin(), mEntries.end(), [name](VariableTableEntry& entry){
            return entry.Name == name;
        });

        if (iterator == mEntries.end())
            return -1;
        
        return (*iterator).ID;
    }

    std::string VariableTable::Lookup(index_t id)
    {
        auto iterator = std::find_if(mEntries.begin(), mEntries.end(), [id](VariableTableEntry& entry){
            return entry.ID == id;
        });

        if (iterator == mEntries.end())
            return "";

        return (*iterator).Name;
    }

    void VariableTable::Add(Language::Variable& var)
    {
        auto iterator = std::find_if(mEntries.begin(), mEntries.end(), [var](VariableTableEntry& entry){
            return entry.Name == var.mName;
        });
        if (iterator != mEntries.end())
            return;

        mEntries.push_back( VariableTableEntry{std::string(var.mName), var.ID} );
        mEntryCount++;
    }

    void VariableTable::Remove(Language::Variable& var)
    {
        auto iterator = std::find_if(mEntries.begin(), mEntries.end(), [var](VariableTableEntry& entry){
            return entry.Name == var.mName;
        });

        if (iterator != mEntries.end())
            mEntries.erase(iterator);
    }

}