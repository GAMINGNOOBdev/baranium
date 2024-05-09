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