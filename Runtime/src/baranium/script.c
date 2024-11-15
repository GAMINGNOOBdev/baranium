#pragma warning(disable: 4996)

#include <baranium/backend/bfuncmgr.h>
#include <baranium/backend/bvarmgr.h>
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

void baranium_script_dynadd_var_or_field(BaraniumSection* section)
{
    bvarmgr* varmgr = baranium_get_context()->varmgr;
    enum BaraniumVariableType type = (enum BaraniumVariableType)*( (uint8_t*)section->Data );
    index_t id = section->ID;
    size_t size = section->DataSize - 1;
    uint8_t isField = section->Type == BaraniumSectionType_Fields;
    void* data = (void*)( ((uint64_t)section->Data) + 1 );

    LOGDEBUG(stringf("Adding field/variable with id %ld", id));

    bvarmgr_alloc(varmgr, type, id, size, isField);
    bvarmgr_n* entry = bvarmgr_get(varmgr, id);
    if (!entry)
    {
        LOGERROR(stringf("Could not append %s with id %ld", isField ? "field" : "variable", id));
        return;
    }

    void* dataPtr = NULL;
    if (isField)
        dataPtr = entry->field->Value;
    else
        dataPtr = entry->variable->Value;

    if (!dataPtr)
    {
        LOGERROR(stringf("Could not allocate memory for %s with id %ld", isField ? "field" : "variable", id));
        bvarmgr_dealloc(varmgr, id);
        return;
    }

    memcpy(dataPtr, data, size);
}

void baranium_script_append_section(BaraniumScript* script, BaraniumSection* section)
{
    if (script == NULL || section == NULL)
        return;

    if (section->Type == BaraniumSectionType_Fields || section->Type == BaraniumSectionType_Variables)
    {
        baranium_script_dynadd_var_or_field(section);
        return;
    }

    baranium_function_manager_add(baranium_get_context()->functionManager, section->ID, script);

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

    FILE* file = handle->file;

    BaraniumScript* script = malloc(sizeof(BaraniumScript));
    if (!script)
        return NULL;

    memset(script, 0, sizeof(BaraniumScript));
    script->Handle = handle;

    fread(&script->Header, 1, sizeof(BaraniumScriptHeader), file);

    if (memcmp(script->Header.MagicNumber, BARANIUM_SCRIPT_HEADER_MAGIC, 4*sizeof(uint8_t)) != 0)
        return NULL;

    if (script->Header.Version != VERSION_CURRENT)
        LOGWARNING("Warning, script may be out of date, be sure to update your compiler and recompile the script");

    BaraniumSection* section;
    for (uint64_t i = 0; i < script->Header.SectionCount; i++)
    {
        section = malloc(sizeof(BaraniumSection));
        if (!section)
            continue;

        memset(section, 0, sizeof(BaraniumSection));

        fread(&section->Type, sizeof(uint8_t), 1, file);
        fread(&section->ID, sizeof(index_t), 1, file);
        fread(&section->DataSize, sizeof(uint64_t), 1, file);
        if (section->Type != BaraniumSectionType_Functions)
        {
            section->Data = malloc(section->DataSize);
            if (!section->Data)
            {
                fseek(file, section->DataSize, SEEK_CUR);
                free(section);
                continue;
            }
            else
            {
                memset(section->Data, 0, section->DataSize);
                fread(section->Data, sizeof(uint8_t), section->DataSize, file);
            }
        }
        else
        {
            section->DataLocation = ftell(file);
            fseek(file, section->DataSize, SEEK_CUR);
        }

        baranium_script_append_section(script, section);
    }

    fread(&script->NameTable.NameCount, sizeof(uint64_t), 1, file);
    BaraniumScriptNameTableEntry* entry;
    for (uint64_t i = 0; i < script->NameTable.NameCount; i++)
    {
        entry = malloc(sizeof(BaraniumScriptNameTableEntry));
        if (!entry)
            continue;

        memset(entry, 0, sizeof(BaraniumScriptNameTableEntry));

        fread(&entry->NameLength, sizeof(uint8_t), 1, file);
        entry->Name = malloc(entry->NameLength);
        if (!entry->Name)
        {
            free(entry);
            continue;
        }

        memset(entry->Name, 0, entry->NameLength);
        fread(entry->Name, sizeof(uint8_t), entry->NameLength, file);
        fread(&entry->ID, sizeof(index_t), 1, file);

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
            if (current->Type == BaraniumSectionType_Functions)
                baranium_function_manager_remove(baranium_get_context()->functionManager, current->ID);

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
    if (!result)
        return NULL;
    memset(result, 0, sizeof(BaraniumVariable));
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
    if (!result)
        return NULL;

    memset(result, 0, sizeof(BaraniumField));
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
    if (!result)
        return NULL;

    memset(result, 0, sizeof(BaraniumFunction));
    result->DataSize = foundSection->DataSize;
    result->Data = malloc(foundSection->DataSize);
    memset(result->Data, 0, result->DataSize);
    fseek(script->Handle->file, foundSection->DataLocation, SEEK_SET);
    fread(result->Data, 1, result->DataSize, script->Handle->file);
    result->ID = functionID;
    result->Script = script;

    return result;
}