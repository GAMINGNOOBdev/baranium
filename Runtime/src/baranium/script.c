#pragma warning(disable: 4996)

#include <baranium/backend/bfuncmgr.h>
#include <baranium/backend/bvarmgr.h>
#include <baranium/variable.h>
#include <baranium/runtime.h>
#include <baranium/defines.h>
#include <baranium/logging.h>
#include <baranium/script.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

uint8_t BARANIUM_SCRIPT_HEADER_MAGIC[4] = {
    MAGIC_NUM_0, MAGIC_NUM_1, MAGIC_NUM_2, MAGIC_NUM_3
};

void baranium_script_dynadd_var_or_field(baranium_section* section)
{
    bvarmgr* varmgr = baranium_get_context()->varmgr;
    baranium_variable_type_t type = (baranium_variable_type_t)*( (uint8_t*)section->Data );
    index_t id = section->ID;
    size_t size = section->DataSize - 1;
    uint8_t isField = section->Type == SECTION_TYPE_FIELDS;
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

void baranium_script_append_section(baranium_script* script, baranium_section* section)
{
    if (script == NULL || section == NULL)
        return;
    if (section->Type == SECTION_TYPE_INVALID)
        return;

    if (section->Type == SECTION_TYPE_FIELDS || section->Type == SECTION_TYPE_VARIABLES)
        baranium_script_dynadd_var_or_field(section);
    else
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

void baranium_script_append_name_table_entry(baranium_script* script, baranium_script_name_table_entry* entry)
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

baranium_script* baranium_open_script(baranium_handle* handle)
{
    if (handle == NULL)
        return NULL;

    if (handle->file == NULL)
        return NULL;

    FILE* file = handle->file;

    baranium_script* script = malloc(sizeof(baranium_script));
    if (!script)
        return NULL;

    memset(script, 0, sizeof(baranium_script));
    script->Handle = handle;

    fread(&script->Header, 1, sizeof(baranium_script_header), file);

    if (memcmp(script->Header.MagicNumber, BARANIUM_SCRIPT_HEADER_MAGIC, 4*sizeof(uint8_t)) != 0)
        return NULL;

    if (script->Header.Version != VERSION_CURRENT)
        LOGWARNING("Warning, script may be out of date, be sure to update your compiler and recompile the script");

    baranium_section* section;
    for (uint64_t i = 0; i < script->Header.SectionCount; i++)
    {
        section = malloc(sizeof(baranium_section));
        if (!section)
            continue;

        memset(section, 0, sizeof(baranium_section));

        fread(&section->Type, sizeof(uint8_t), 1, file);
        fread(&section->ID, sizeof(index_t), 1, file);
        fread(&section->DataSize, sizeof(uint64_t), 1, file);
        if (section->Type != SECTION_TYPE_FUNCTIONS)
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
    baranium_script_name_table_entry* entry;
    for (uint64_t i = 0; i < script->NameTable.NameCount; i++)
    {
        entry = malloc(sizeof(baranium_script_name_table_entry));
        if (!entry)
            continue;

        memset(entry, 0, sizeof(baranium_script_name_table_entry));

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

void baranium_close_script(baranium_script* script)
{
    if (script == NULL)
        return;

    if (script->NameTable.EntriesStart != NULL)
    {
        baranium_script_name_table_entry* current = script->NameTable.EntriesStart;
        baranium_script_name_table_entry* next = NULL;
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
        baranium_section* current = script->SectionsStart;
        baranium_section* next = NULL;
        for (uint64_t i = 0; i < script->Header.SectionCount && current != NULL; i++)
        {
            if (current->Type == SECTION_TYPE_FUNCTIONS)
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

baranium_section* baranium_script_get_section_by_id_and_type(baranium_script* script, index_t id, baranium_section_type_t type)
{
    if (script == NULL || type == SECTION_TYPE_INVALID || id == INVALID_INDEX)
        return NULL;
    
    baranium_section* current = script->SectionsStart;
    for (uint64_t i = 0; i < script->Header.SectionCount && current != NULL; i++)
    {
        if (current->ID == id && current->Type == type)
            return current;

        current = current->next;
    }
    return NULL;
}

index_t baranium_script_get_id_of(baranium_script* script, const char* name)
{
    if (script == NULL || name == NULL)
        return INVALID_INDEX;
    
    baranium_script_name_table_entry* current = script->NameTable.EntriesStart;
    for (uint64_t i = 0; i < script->NameTable.NameCount && current != NULL; i++)
    {
        if (strcmp(name, current->Name) == 0)
            return current->ID;
        
        current = current->next;
    }

    return INVALID_INDEX;
}

char* baranium_script_get_name_of(baranium_script* script, index_t id)
{
    if (script == NULL || id == INVALID_INDEX)
        return NULL;
    
    baranium_script_name_table_entry* current = script->NameTable.EntriesStart;
    for (uint64_t i = 0; i < script->NameTable.NameCount && current != NULL; i++)
    {
        if (current->ID == id)
            return current->Name;

        current = current->next;
    }

    return NULL;
}

baranium_variable* baranium_script_get_variable(baranium_script* script, const char* name)
{
    index_t id = baranium_script_get_id_of(script, name);
    return baranium_script_get_variable_by_id(script, id);
}

baranium_variable* baranium_script_get_variable_by_id(baranium_script* script, index_t variableID)
{
    if (script == NULL || variableID == INVALID_INDEX)
        return NULL;

    baranium_variable* result = NULL;

    bvarmgr_n* entry = bvarmgr_get(baranium_get_context()->varmgr, variableID);
    if (entry != NULL)
    if (entry->isVariable)
        return entry->variable; // early out if we can somehow find it in the var manager already

    baranium_section* foundSection = baranium_script_get_section_by_id_and_type(script, variableID, SECTION_TYPE_VARIABLES);
    if (foundSection == NULL)
        return NULL;

    result = malloc(sizeof(baranium_variable));
    if (!result)
        return NULL;
    memset(result, 0, sizeof(baranium_variable));
    result->ID = variableID;
    result->Type = *foundSection->Data;
    result->Value = (void*)(((uint8_t*)foundSection->Data) + 1);

    return result;
}

baranium_field* baranium_script_get_field(baranium_script* script, const char* name)
{
    index_t id = baranium_script_get_id_of(script, name);
    return baranium_script_get_field_by_id(script, id);
}

baranium_field* baranium_script_get_field_by_id(baranium_script* script, index_t fieldID)
{
    if (script == NULL || fieldID == INVALID_INDEX)
        return NULL;

    baranium_field* result = NULL;

    bvarmgr_n* entry = bvarmgr_get(baranium_get_context()->varmgr, fieldID);
    if (entry != NULL)
    if (!entry->isVariable)
        return entry->field; // early out if we can somehow find it in the var manager already

    baranium_section* foundSection = baranium_script_get_section_by_id_and_type(script, fieldID, SECTION_TYPE_FIELDS);
    if (foundSection == NULL)
        return NULL;

    result = malloc(sizeof(baranium_field));
    if (!result)
        return NULL;


    return result;
}

baranium_function* baranium_script_get_function(baranium_script* script, const char* name)
{
    index_t id = baranium_script_get_id_of(script, name);
    return baranium_script_get_function_by_id(script, id);
}

baranium_function* baranium_script_get_function_by_id(baranium_script* script, index_t functionID)
{
    if (script == NULL || functionID == INVALID_INDEX)
        return NULL;
    
    baranium_function* result = NULL;
    baranium_section* foundSection = baranium_script_get_section_by_id_and_type(script, functionID, SECTION_TYPE_FUNCTIONS);
    if (foundSection == NULL)
        return NULL;

    result = malloc(sizeof(baranium_function));
    if (!result)
        return NULL;

    memset(result, 0, sizeof(baranium_function));
    result->DataSize = foundSection->DataSize-2;
    result->Data = malloc(foundSection->DataSize);
    memset(result->Data, 0, result->DataSize);
    fseek(script->Handle->file, foundSection->DataLocation, SEEK_SET);
    fread(&result->ParameterCount, sizeof(uint8_t), 1, script->Handle->file);
    fread(&result->ReturnType, sizeof(uint8_t), 1, script->Handle->file);
    fread(result->Data, 1, result->DataSize, script->Handle->file);
    result->ID = functionID;
    result->Script = script;

    return result;
}
