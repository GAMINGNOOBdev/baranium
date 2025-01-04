#ifndef __BINARIES__VARIABLETABLE_H_
#define __BINARIES__VARIABLETABLE_H_ 1

#include "../Language/Variable.h"
#include "../BgeFile.hpp"
#include <stdint.h>
#include <string>
#include <vector>

typedef int64_t index_t;

namespace Binaries
{

    /**
     * @brief An entry of a `VariableTable`
     */
    struct VariableTableEntry
    {
        // normally name length + name data but since it's c++ rn, just use std::string
        std::string Name;
        index_t ID;
    };

    /**
     * @brief A table containing names and their respective internal ID
     * 
     * @note This is only useful while compilation to check for temporary variables
     */
    struct VariableTable
    {
        /**
         * @brief Construct a new `VariableTable`
         */
        VariableTable();

        /**
         * @brief Clear the variable table
         */
        void Clear();

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
         * @brief Add a variable entry
         * 
         * @param var The variable for which the entry will be created
         */
        void Add(Language::Variable& var);

        /**
         * @brief Remove a variable entry
         * 
         * @param var The variable that will be removed
         */
        void Remove(Language::Variable& var);

        /**
         * @brief Get all variable entries
         */
        const std::vector<VariableTableEntry> GetAllEntries();

        void operator=(VariableTable& other);

    private:
        // Name entries, can be directly written to a file using fwrite
        std::vector<VariableTableEntry> mEntries;
    };

}

#endif
