#pragma warning(disable: 4996)

#include <baranium/runtime.h>
#include <baranium/logging.h>
#include <baranium/script.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

uint8_t BARANIUM_SCRIPT_HEADER_MAGIC[4] = {
    MAGIC_NUM_0, MAGIC_NUM_1, MAGIC_NUM_2, MAGIC_NUM_3
};

void baranium_script_append_section(BaraniumScript* script, BaraniumSection* section)
{
    if (script == NULL || section == NULL)
        return;
    
    if (script->SectionsStart == NULL)
    {
        script->SectionsStart = section;
        script->SectionsEnd = section;
        return;
    }

    script->SectionsEnd->next = section;
    section->prev = script->SectionsEnd;
    script->SectionsEnd = section;
}

void baranium_script_append_name_table_entry(BaraniumScript* script, BaraniumScriptNameTableEntry* entry)
{
    if (script == NULL || entry == NULL)
        return;
    
    if (script->NameTable.EntriesStart == NULL)
    {
        script->NameTable.EntriesStart = entry;
        script->NameTable.EntriesEnd = entry;
        return;
    }

    script->NameTable.EntriesEnd->next = entry;
    entry->prev = script->NameTable.EntriesEnd;
    script->NameTable.EntriesEnd = entry;
}

////////////////////////////////
///                          ///
/// public visible functions ///
///                          ///
////////////////////////////////

BaraniumScript* baranium_open_script(BaraniumHandle* handle)
{
    if (handle == NULL)
        return NULL;

    if (handle->file == NULL)
        return NULL;

    BaraniumScript* script = malloc(sizeof(BaraniumScript));
    memset(script, 0, sizeof(BaraniumScript));

    fread(&script->Header, 1, sizeof(BaraniumScriptHeader), handle->file);

    if (memcmp(script->Header.MagicNumber, BARANIUM_SCRIPT_HEADER_MAGIC, 4*sizeof(uint8_t)) != 0)
        return NULL;

    if (script->Header.Version != VERSION_CURRENT)
        LOGWARNING("Warning, script may be out of date, be sure to update your compiler and recompile the script");

    BaraniumSection* section;
    for (uint64_t i = 0; i < script->Header.SectionCount; i++)
    {
        section = malloc(sizeof(BaraniumSection));
        memset(section, 0, sizeof(BaraniumSection));

        fread(&section->Type, sizeof(uint8_t), 1, handle->file);
        fread(&section->ID, sizeof(index_t), 1, handle->file);
        fread(&section->DataSize, sizeof(uint64_t), 1, handle->file);
        section->Data = malloc(section->DataSize);
        memset(section->Data, 0, section->DataSize);
        fread(section->Data, sizeof(uint8_t), section->DataSize, handle->file);

        baranium_script_append_section(script, section);
    }

    fread(&script->NameTable.NameCount, sizeof(uint64_t), 1, handle->file);
    BaraniumScriptNameTableEntry* entry;
    for (uint64_t i = 0; i < script->NameTable.NameCount; i++)
    {
        entry = malloc(sizeof(BaraniumScriptNameTableEntry));
        memset(entry, 0, sizeof(BaraniumScriptNameTableEntry));

        fread(&entry->NameLength, sizeof(uint8_t), 1, handle->file);
        entry->Name = malloc(entry->NameLength);
        memset(entry->Name, 0, entry->NameLength);
        fread(entry->Name, sizeof(uint8_t), entry->NameLength, handle->file);
        fread(&entry->ID, sizeof(index_t), 1, handle->file);

        baranium_script_append_name_table_entry(script, entry);
    }

    return script;
}

void baranium_close_script(BaraniumScript* script)
{
    if (script == NULL)
        return;

    if (script->NameTable.EntriesStart != NULL)
    {
        BaraniumScriptNameTableEntry* current = script->NameTable.EntriesStart;
        BaraniumScriptNameTableEntry* next = NULL;
        for (uint64_t i = 0; i < script->NameTable.NameCount && current != NULL; i++)
        {
            free(current->Name);
            next = current->next;
            free(current);
            current = next;
        }

        script->NameTable.EntriesStart = script->NameTable.EntriesEnd = NULL;
        script->NameTable.NameCount = 0;
    }

    if (script->SectionsStart != NULL)
    {
        BaraniumSection* current = script->SectionsStart;
        BaraniumSection* next = NULL;
        for (uint64_t i = 0; i < script->Header.SectionCount && current != NULL; i++)
        {
            next = current->next;
            free(current->Data);
            free(current);
            current = next;
        }

        script->SectionsStart = script->SectionsEnd = NULL;
        script->Header.SectionCount = 0;
    }

    free(script);
}

BaraniumSection* baranium_script_get_section_by_id_and_type(BaraniumScript* script, index_t id, enum BaraniumSectionType type)
{
    if (script == NULL || type == BaraniumSectionType_Invalid || id == INVALID_INDEX)
        return NULL;
    
    BaraniumSection* current = script->SectionsStart;
    for (uint64_t i = 0; i < script->Header.SectionCount && current != NULL; i++)
    {
        if (current->ID == id && current->Type == type)
            return current;

        current = current->next;
    }
    return NULL;
}

index_t baranium_script_get_id_of(BaraniumScript* script, const char* name)
{
    if (script == NULL || name == NULL)
        return INVALID_INDEX;
    
    BaraniumScriptNameTableEntry* current = script->NameTable.EntriesStart;
    for (uint64_t i = 0; i < script->NameTable.NameCount && current != NULL; i++)
    {
        if (strcmp(name, current->Name) == 0)
            return current->ID;
        
        current = current->next;
    }

    return INVALID_INDEX;
}

char* baranium_script_get_name_of(BaraniumScript* script, index_t id)
{
    if (script == NULL || id == INVALID_INDEX)
        return NULL;
    
    BaraniumScriptNameTableEntry* current = script->NameTable.EntriesStart;
    for (uint64_t i = 0; i < script->NameTable.NameCount && current != NULL; i++)
    {
        if (current->ID == id)
            return current->Name;
        
        current = current->next;
    }

    return NULL;
}

BaraniumVariable* baranium_script_get_variable(BaraniumScript* script, const char* name)
{
    index_t id = baranium_script_get_id_of(script, name);
    return baranium_script_get_variable_by_id(script, id);
}

BaraniumVariable* baranium_script_get_variable_by_id(BaraniumScript* script, index_t variableID)
{
    if (script == NULL || variableID == INVALID_INDEX)
        return NULL;

    BaraniumVariable* result = NULL;

    BaraniumSection* foundSection = baranium_script_get_section_by_id_and_type(script, variableID, BaraniumSectionType_Variables);
    if (foundSection == NULL)
        return NULL;

    result = malloc(sizeof(BaraniumVariable));
    result->ID = variableID;
    result->Type = *foundSection->Data;
    result->Value = (void*)(((uint8_t*)foundSection->Data) + 1);

    return result;
}

BaraniumField* baranium_script_get_field(BaraniumScript* script, const char* name)
{
    index_t id = baranium_script_get_id_of(script, name);
    return baranium_script_get_field_by_id(script, id);
}

BaraniumField* baranium_script_get_field_by_id(BaraniumScript* script, index_t fieldID)
{
    if (script == NULL || fieldID == INVALID_INDEX)
        return NULL;

    BaraniumField* result = NULL;

    BaraniumSection* foundSection = baranium_script_get_section_by_id_and_type(script, fieldID, BaraniumSectionType_Fields);
    if (foundSection == NULL)
        return NULL;

    result = malloc(sizeof(BaraniumField));
    result->ID = fieldID;
    result->Type = *foundSection->Data;
    result->Value = (void*)(((uint8_t*)foundSection->Data) + 1);

    return result;
}

BaraniumFunction* baranium_script_get_function(BaraniumScript* script, const char* name)
{
    index_t id = baranium_script_get_id_of(script, name);
    return baranium_script_get_function_by_id(script, id);
}

BaraniumFunction* baranium_script_get_function_by_id(BaraniumScript* script, index_t functionID)
{
    if (script == NULL || functionID == INVALID_INDEX)
        return NULL;

    BaraniumFunction* result = NULL;

    BaraniumSection* foundSection = baranium_script_get_section_by_id_and_type(script, functionID, BaraniumSectionType_Functions);
    if (foundSection == NULL)
        return NULL;

    result = malloc(sizeof(BaraniumFunction));
    result->DataSize = foundSection->DataSize;
    result->Data = foundSection->Data;
    result->ID = functionID;

    return result;
}