#include <baranium/defines.h>
#include <baranium/version.h>
#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
#   pragma warning(disable: 4996)
#endif

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
#include <stdint.h>
#include <stdio.h>

uint8_t BARANIUM_SCRIPT_HEADER_MAGIC[4] = {
    BARANIUM_MAGIC_NUM0, BARANIUM_MAGIC_NUM1, BARANIUM_MAGIC_NUM2, BARANIUM_MAGIC_NUM3
};

void baranium_script_dynadd_var_or_field(baranium_script_section* section)
{
    bvarmgr* varmgr = baranium_get_runtime()->varmgr;
    baranium_variable_type_t type = (baranium_variable_type_t)*( (uint8_t*)section->data );
    index_t id = section->id;
    size_t size = section->data_size - 1;
    uint8_t isField = section->type == BARANIUM_SCRIPT_SECTION_TYPE_FIELDS;
    void* data = (void*)( ((uint64_t)section->data) + 1 );

    LOGDEBUG("Adding field/variable with id %ld", id);

    bvarmgr_alloc(varmgr, type, id, size, isField);
    bvarmgr_n* entry = bvarmgr_get(varmgr, id);
    if (!entry)
    {
        LOGERROR("Could not append %s with id %ld", isField ? "field" : "variable", id);
        return;
    }

    baranium_value_t val = {0};
    void* dataPtr = NULL;
    if (isField)
        val = entry->field->value;
    else
        val = entry->variable->value;

    if (type == BARANIUM_VARIABLE_TYPE_STRING)
        dataPtr = val.ptr;
    else
        dataPtr = &val.num64;

    if (!dataPtr)
    {
        LOGERROR("Could not allocate memory for %s with id %ld", isField ? "field" : "variable", id);
        bvarmgr_dealloc(varmgr, id);
        return;
    }

    memcpy(dataPtr, data, size);
}

void baranium_script_assign_section(baranium_script* script, baranium_script_section* section)
{
    if (script == NULL || section == NULL)
        return;
    if (section->type == BARANIUM_SCRIPT_SECTION_TYPE_INVALID)
        return;

    if (section->type == BARANIUM_SCRIPT_SECTION_TYPE_FIELDS || section->type == BARANIUM_SCRIPT_SECTION_TYPE_VARIABLES)
        baranium_script_dynadd_var_or_field(section);
    else
        baranium_function_manager_add(baranium_get_runtime()->function_manager, section->id, script, NULL);

    if (script->sections == NULL)
    {
        script->section_buffer_size += BARANIUM_SCRIPT_SECTION_LIST_BUFFER_SIZE;
        script->sections = realloc(script->sections, sizeof(baranium_script_section)*script->section_buffer_size);
    }

    memcpy(&script->sections[script->section_count], section, sizeof(baranium_script_section));
    script->section_count++;
}

void baranium_script_append_name_table_entry(baranium_script* script, baranium_script_name_table_entry* entry)
{
    if (script == NULL || entry == NULL)
        return;
    
    if (script->nametable.entries == NULL)
    {
        script->nametable.buffer_size += BARANIUM_SCRIPT_NAME_TABLE_BUFFER_SIZE;
        script->nametable.entries = realloc(script->nametable.entries, sizeof(baranium_script_name_table_entry)*script->nametable.buffer_size);
    }

    memcpy(&script->nametable.entries[script->nametable.name_count], entry, sizeof(baranium_script_name_table_entry));
    script->nametable.name_count++;
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
    script->handle = handle;
    script->section_buffer_size = 0;
    script->section_count = 0;
    script->sections = NULL;

    fread(&script->header, 1, sizeof(baranium_script_header), file);

    if (memcmp(script->header.magic, BARANIUM_SCRIPT_HEADER_MAGIC, 4*sizeof(uint8_t)) != 0)
    {
        free(script);
        LOGERROR("Invalid file detected, input was not a valid baranium binary");
        return NULL;
    }

    if (script->header.version != BARANIUM_VERSION_CURRENT)
        LOGWARNING("Warning, script may be out of date, be sure to update your compiler and recompile the script");

    baranium_script_section section;
    script->section_count = 0;
    script->section_buffer_size = script->header.section_count;
    script->sections = malloc(sizeof(baranium_script_section)*script->header.section_count);
    for (uint64_t i = 0; i < script->header.section_count; i++)
    {
        section = (baranium_script_section){.type=0,.id=0,.data_size=0,.data_location=0,.data=NULL};

        fread(&section.type, sizeof(uint8_t), 1, file);
        fread(&section.id, sizeof(index_t), 1, file);
        fread(&section.data_size, sizeof(uint64_t), 1, file);
        if (section.data_size == 0)
        {
            LOGDEBUG("found a section with a size of 0 (id[0x%x/%lld] type[0x%x/%lld])",section.id,section.id,section.type);
            script->section_count++;
            continue;
        }
        if (section.type != BARANIUM_SCRIPT_SECTION_TYPE_FUNCTIONS)
        {
            section.data = malloc(section.data_size);
            if (!section.data)
            {
                fseek(file, section.data_size, SEEK_CUR);
            }
            else
            {
                memset(section.data, 0, section.data_size);
                fread(section.data, sizeof(uint8_t), section.data_size, file);
            }
        }
        else
        {
            section.data_location = ftell(file);
            fseek(file, section.data_size, SEEK_CUR);
        }

        baranium_script_assign_section(script, &section);
    }
    script->section_count = script->header.section_count;

    if (script->header.version < BARANIUM_VERSION_THIRD_RELEASE)
    {
        fread(&script->nametable.name_count, sizeof(uint64_t), 1, file);
        script->nametable.buffer_size = 0;
        script->nametable.entries = NULL;
        baranium_script_name_table_entry entry;
        for (uint64_t i = 0; i < script->nametable.name_count; i++)
        {
            entry = (baranium_script_name_table_entry){.length=0,.name=NULL,.id=BARANIUM_INVALID_INDEX};

            fread(&entry.length, sizeof(uint8_t), 1, file);
            entry.name = malloc(entry.length);
            if (!entry.name)
                continue;

            memset(entry.name, 0, entry.length);
            fread(entry.name, sizeof(uint8_t), entry.length, file);
            fread(&entry.id, sizeof(index_t), 1, file);

            baranium_script_append_name_table_entry(script, &entry);
        }
        return script;
    }

    size_t dependency_count = 0;
    fread(&dependency_count, sizeof(size_t), 1, file);
    for (size_t i = 0; i < dependency_count; i++)
    {
        size_t dependencylen = BARANIUM_INVALID_INDEX;
        char* dependency = NULL;
        fread(&dependencylen, sizeof(size_t), 1, file);
        dependency = malloc(dependencylen+1);
        fread(dependency, sizeof(char), dependencylen, file);
        dependency[dependencylen] = 0;
        baranium_runtime_load_dependency(dependency);
        free(dependency);
    }

    return script;
}

void baranium_close_script(baranium_script* script)
{
    if (script == NULL)
        return;

    if (script->nametable.entries != NULL)
    {
        for (uint64_t i = 0; i < script->nametable.name_count; i++)
        {
            baranium_script_name_table_entry current = script->nametable.entries[i];
            free(current.name);
        }

        free(script->nametable.entries);

        script->nametable.buffer_size = 0;
        script->nametable.name_count = 0;
        script->nametable.entries = NULL;
    }

    if (script->sections != NULL)
    {
        for (uint64_t i = 0; i < script->header.section_count; i++)
        {
            baranium_script_section current = script->sections[i];
            if (current.type == BARANIUM_SCRIPT_SECTION_TYPE_FUNCTIONS)
                baranium_function_manager_remove(baranium_get_runtime()->function_manager, current.id);

            free(current.data);
        }

        free(script->sections);

        script->header.section_count = 0;
        script->section_buffer_size = 0;
        script->section_count = 0;
        script->sections = NULL;
    }

    free(script);
}

baranium_script_section* baranium_script_get_section_by_id_and_type(baranium_script* script, index_t id, baranium_script_section_type_t type)
{
    if (script == NULL || type == BARANIUM_SCRIPT_SECTION_TYPE_INVALID || id == BARANIUM_INVALID_INDEX)
        return NULL;
    
    for (uint64_t i = 0; i < script->header.section_count; i++)
    {
        baranium_script_section current = script->sections[i];
        if (current.id == id && current.type == type)
            return script->sections + i;
    }
    return NULL;
}

index_t baranium_script_get_id_of(baranium_script* script, const char* name)
{
    if (script == NULL || name == NULL)
        return BARANIUM_INVALID_INDEX;

    for (uint64_t i = 0; i < script->nametable.name_count; i++)
    {
        baranium_script_name_table_entry current = script->nametable.entries[i];
        if (strcmp(name, (const char*)current.name) == 0)
            return current.id;
    }

    index_t id = baranium_get_id_of_name(name);
    for (uint64_t i = 0; i < script->section_count; i++)
    {
        if (script->sections[i].id == id)
            return id;
    }

    return BARANIUM_INVALID_INDEX;
}

char* baranium_script_get_name_of(baranium_script* script, index_t id)
{
    if (script == NULL || id == BARANIUM_INVALID_INDEX)
        return NULL;
    
    for (uint64_t i = 0; i < script->nametable.name_count; i++)
    {
        baranium_script_name_table_entry current = script->nametable.entries[i];
        if (current.id == id)
            return (char*)current.name;
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
    if (script == NULL || variableID == BARANIUM_INVALID_INDEX)
        return NULL;

    baranium_variable* result = NULL;

    bvarmgr_n* entry = bvarmgr_get(baranium_get_runtime()->varmgr, variableID);
    if (entry != NULL)
        if (entry->isVariable)
            return entry->variable; // early out if we can somehow find it in the var manager already

    baranium_script_section* foundSection = baranium_script_get_section_by_id_and_type(script, variableID, BARANIUM_SCRIPT_SECTION_TYPE_VARIABLES);
    if (foundSection == NULL)
        return NULL;

    result = malloc(sizeof(baranium_variable));
    if (!result)
        return NULL;
    memset(result, 0, sizeof(baranium_variable));
    result->id = variableID;
    result->type = *foundSection->data;
    result->size = foundSection->data_size-1;
    if (result->type == BARANIUM_VARIABLE_TYPE_STRING)
        result->value.ptr = (void*)(((uint8_t*)foundSection->data) + 1);
    else
        memcpy(&result->value.num64, (void*)(((uint8_t*)foundSection->data) + 1), result->size);

    return result;
}

baranium_field* baranium_script_get_field(baranium_script* script, const char* name)
{
    index_t id = baranium_script_get_id_of(script, name);
    return baranium_script_get_field_by_id(script, id);
}

baranium_field* baranium_script_get_field_by_id(baranium_script* script, index_t fieldID)
{
    if (script == NULL || fieldID == BARANIUM_INVALID_INDEX)
        return NULL;

    baranium_field* result = NULL;

    bvarmgr_n* entry = bvarmgr_get(baranium_get_runtime()->varmgr, fieldID);
    if (entry != NULL)
        if (!entry->isVariable)
            return entry->field; // early out if we can somehow find it in the var manager already

    baranium_script_section* foundSection = baranium_script_get_section_by_id_and_type(script, fieldID, BARANIUM_SCRIPT_SECTION_TYPE_FIELDS);
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
    if (script == NULL || functionID == BARANIUM_INVALID_INDEX)
        return NULL;

    baranium_function* result = NULL;
    baranium_script_section* foundSection = baranium_script_get_section_by_id_and_type(script, functionID, BARANIUM_SCRIPT_SECTION_TYPE_FUNCTIONS);
    if (foundSection == NULL)
        return NULL;

    result = malloc(sizeof(baranium_function));
    if (!result)
        return NULL;

    memset(result, 0, sizeof(baranium_function));
    result->data_size = foundSection->data_size-2;
    result->data = malloc(foundSection->data_size);
    memset(result->data, 0, result->data_size);
    fseek(script->handle->file, foundSection->data_location, SEEK_SET);
    fread(&result->parameter_count, sizeof(uint8_t), 1, script->handle->file);
    fread(&result->return_type, sizeof(uint8_t), 1, script->handle->file);
    fread(result->data, 1, result->data_size, script->handle->file);
    result->id = functionID;
    result->script = script;

    return result;
}
