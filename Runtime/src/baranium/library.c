#include <baranium/compiler/compiler_context.h>
#include <baranium/backend/bfuncmgr.h>
#include <baranium/backend/bvarmgr.h>
#include <baranium/backend/varmath.h>
#include <baranium/callback.h>
#include <baranium/variable.h>
#include <baranium/library.h>
#include <baranium/logging.h>
#include <baranium/runtime.h>
#include <baranium/script.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

uint8_t BARANIUM_LIBRARY_HEADER_MAGIC[4] = {
    BARANIUM_LIBRARY_MAGIC_NUM0, BARANIUM_LIBRARY_MAGIC_NUM1, BARANIUM_LIBRARY_MAGIC_NUM2, BARANIUM_LIBRARY_MAGIC_NUM3
};

void baranium_library_dynadd_var_or_field(baranium_library_section* section)
{
    bvarmgr* varmgr = baranium_get_runtime()->varmgr;
    baranium_variable_type_t type = (baranium_variable_type_t)*( section->data );
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

void baranium_library_load_section(baranium_library* lib, uint64_t index, baranium_library_section* section)
{
    if (lib == NULL || section == NULL)
        return;
    if (section->type == BARANIUM_SCRIPT_SECTION_TYPE_INVALID)
        return;

    if (section->type == BARANIUM_SCRIPT_SECTION_TYPE_FIELDS || section->type == BARANIUM_SCRIPT_SECTION_TYPE_VARIABLES)
        baranium_library_dynadd_var_or_field(section);
    else
        baranium_function_manager_add(baranium_get_runtime()->function_manager, section->id, NULL, lib);

    memcpy(&lib->sections[index], section, sizeof(baranium_library_section));
}

void baranium_library_install_callbacks(baranium_library* lib);

baranium_library* baranium_library_load(const char* path)
{
    if (path == NULL)
        return NULL;

    FILE* file = fopen(path, "rb");
    if (file == NULL)
    {
        LOGERROR("File '%s' not found", path);
        return NULL;
    }

    baranium_library* library = malloc(sizeof(baranium_library));
    if (!library)
    {
        fclose(file);
        return NULL;
    }

    memset(library, 0, sizeof(baranium_library));
    library->file = file;
    library->sections = NULL;

    int lastSeperatorIdx = strlen(path)-1;
    for (int i = lastSeperatorIdx; i > 0; i--)
    {
        if (path[i] == '\\' || path[i] == '/')
        {
            lastSeperatorIdx = i;
            break;
        }
    }
    int namelen = strlen(path) - lastSeperatorIdx - 1;
    library->name = malloc(namelen+1);
    if (library->name == NULL)
    {
        free(library);
        fclose(file);
        return NULL;
    }
    memset((void*)library->name, 0, namelen+1);
    strncpy((char*)library->name, &path[lastSeperatorIdx+1], namelen);

    int pathlen = strlen(path);
    library->path = malloc(pathlen+1);
    if (library->path == NULL)
    {
        free(library);
        fclose(file);
        return NULL;
    }
    strcpy((char*)library->path, path);
    ((char*)library->path)[pathlen] = 0;

    fread(&library->libheader, 1, sizeof(baranium_library_header), file);

    if (memcmp(library->libheader.magic, BARANIUM_LIBRARY_HEADER_MAGIC, 4*sizeof(uint8_t)) != 0)
    {
        free(library);
        fclose(file);
        return NULL;
    }

    if (library->libheader.version != BARANIUM_VERSION_CURRENT)
        LOGWARNING("Library '%s' may be out of date, be sure to update your compiler and recompile the library", library->name);

    library->exports = realloc(library->exports, sizeof(baranium_library_export)*library->libheader.exports_count);
    for (uint64_t i = 0; i < library->libheader.exports_count; i++)
    {
        baranium_library_export* export = &library->exports[i];
        fread(&export->type, sizeof(baranium_script_section_type_t), 1, file);
        fread(&export->id, sizeof(index_t), 1, file);
        fread(&export->num_params, sizeof(int), 1, file);
        fread(&export->return_type, sizeof(baranium_variable_type_t), 1, file);
        fread(&export->symnamelen, sizeof(size_t), 1, file);
        if (export->symnamelen > 0)
        {
            export->symname = malloc(export->symnamelen+1);
            ((char*)export->symname)[export->symnamelen] = 0;
            fread((void*)export->symname, sizeof(char), export->symnamelen, file);
        }
    }

    baranium_library_section section;
    library->sections = realloc(library->sections, sizeof(baranium_library_section)*library->libheader.section_count);
    for (uint64_t i = 0; i < library->libheader.section_count; i++)
    {
        section = (baranium_library_section){.type=0,.id=0,.data_size=0,.data_location=0,.data=NULL};

        fread(&section.type, sizeof(uint8_t), 1, file);
        fread(&section.id, sizeof(index_t), 1, file);
        fread(&section.data_size, sizeof(uint64_t), 1, file);
        if (section.data_size == 0)
        {
            LOGDEBUG("found a section with a size of 0 (id[0x%x/%lld] type[0x%x/%lld])",section.id,section.id,section.type);
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

        library->sections[i] = section;

        // only works if there is a runtime loaded
        baranium_library_load_section(library, i, &section);
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

        // usually, we won't have both a compiler and a runtime running at the same time, therefore we can just add the library to "both"
        // this should only work for either the currently loaded runtime or the currently loaded compiler context
        baranium_runtime_load_dependency(dependency);
        baranium_compiler_context_add_library(baranium_get_compiler_context(), dependency);

        free(dependency);
    }

    // again, only works if a runtime is active
    baranium_library_install_callbacks(library);

    fclose(file);

    return library;
}

void baranium_library_dispose(baranium_library* lib)
{
    if (lib == NULL)
        return;

    if (lib->dynlib != NULL)
        baranium_dynlib_unload(lib->dynlib);

    if (lib->name != NULL)
        free((void*)lib->name);

    if (lib->path != NULL)
        free((void*)lib->path);

    if (lib->exports != NULL)
    {
        for (size_t i = 0; i < lib->libheader.exports_count; i++)
        {
            baranium_library_export export = lib->exports[i];
            if (export.symname == NULL)
                continue;

            free((void*)export.symname);
        }

        free(lib->exports);
    }

    if (lib->sections != NULL)
    {
        for (uint64_t i = 0; i < lib->libheader.section_count; i++)
        {
            baranium_library_section current = lib->sections[i];
            if (current.type == BARANIUM_SCRIPT_SECTION_TYPE_FUNCTIONS)
                baranium_function_manager_remove(baranium_get_runtime()->function_manager, current.id);

            free(current.data);
        }

        free(lib->sections);
    }

    free(lib);
}

baranium_library_section* baranium_library_get_section_by_id_and_type(baranium_library* library, index_t id, baranium_script_section_type_t type)
{
    if (library == NULL || type == BARANIUM_SCRIPT_SECTION_TYPE_INVALID || id == BARANIUM_INVALID_INDEX)
        return NULL;

    for (uint64_t i = 0; i < library->libheader.section_count; i++)
    {
        baranium_library_section current = library->sections[i];
        if (current.id == id && current.type == type)
            return library->sections + i;
    }
    return NULL;
}

index_t baranium_library_get_id_of(baranium_library* library, const char* name)
{
    if (library == NULL || name == NULL)
        return BARANIUM_INVALID_INDEX;

    index_t id = baranium_get_id_of_name(name);
    for (uint64_t i = 0; i < library->libheader.section_count; i++)
    {
        if (library->sections[i].id == id)
            return id;
    }

    return BARANIUM_INVALID_INDEX;
}

uint8_t baranium_library_has_symbol(baranium_library* lib, const char* name)
{
    if (lib == NULL || name == NULL)
        return 0;

    index_t id = baranium_get_id_of_name(name);
    for (uint64_t i = 0; i < lib->libheader.exports_count; i++)
    {
        if (lib->exports[i].id == id)
            return 1;
    }

    return 0;
}

baranium_field* baranium_library_get_field(baranium_library* lib, const char* name)
{
    index_t id = baranium_library_get_id_of(lib, name);
    return baranium_library_get_field_by_id(lib, id);
}

baranium_field* baranium_library_get_field_by_id(baranium_library* lib, index_t fieldID)
{
    if (lib == NULL || fieldID == BARANIUM_INVALID_INDEX)
        return NULL;

    baranium_field* result = NULL;

    bvarmgr_n* entry = bvarmgr_get(baranium_get_runtime()->varmgr, fieldID);
    if (entry != NULL)
        if (!entry->isVariable)
            return entry->field; // early out if we can somehow find it in the var manager already

    baranium_library_section* foundSection = baranium_library_get_section_by_id_and_type(lib, fieldID, BARANIUM_SCRIPT_SECTION_TYPE_FIELDS);
    if (foundSection == NULL)
        return NULL;

    result = malloc(sizeof(baranium_field));
    if (!result)
        return NULL;

    return result;
}

baranium_function* baranium_library_get_function(baranium_library* lib, const char* name)
{
    index_t id = baranium_library_get_id_of(lib, name);
    return baranium_library_get_function_by_id(lib, id);
}

baranium_function* baranium_library_get_function_by_id(baranium_library* lib, index_t functionID)
{
    if (lib == NULL || functionID == BARANIUM_INVALID_INDEX)
        return NULL;
    
    baranium_function* result = NULL;
    baranium_library_section* foundSection = baranium_library_get_section_by_id_and_type(lib, functionID, BARANIUM_SCRIPT_SECTION_TYPE_FUNCTIONS);
    if (foundSection == NULL)
        return NULL;

    result = malloc(sizeof(baranium_function));
    if (!result)
        return NULL;

    memset(result, 0, sizeof(baranium_function));
    result->data_size = foundSection->data_size-2;
    result->data = malloc(foundSection->data_size);
    memset(result->data, 0, result->data_size);
    fseek(lib->file, foundSection->data_location, SEEK_SET);
    fread(&result->parameter_count, sizeof(uint8_t), 1, lib->file);
    fread(&result->return_type, sizeof(uint8_t), 1, lib->file);
    fread(result->data, 1, result->data_size, lib->file);
    result->id = functionID;
    result->library = lib;

    return result;
}

void baranium_library_install_callbacks(baranium_library* lib)
{
    if (lib == NULL || lib->dynlib != NULL)
        return;

    if (baranium_get_runtime() == NULL)
        return;

    baranium_dynlib_data data = {
        .runtime = baranium_get_runtime(),
        .vargetsize = (void*)baranium_variable_get_size_of_type,
        .compvarpushtostack = (void*)baranium_compiled_variable_push_to_stack,
        .compvarconverttotype = (void*)baranium_compiled_variable_convert_to_type,
    };
    lib->dynlib = baranium_dynlib_load(stringf("%s%s", lib->path, BARANIUM_DYNLIB_EXTENSION), &data);

    for (size_t i = 0; i < lib->libheader.exports_count; i++)
    {
        baranium_library_export export = lib->exports[i];
        if (export.symname == NULL)
            continue;

        index_t id = export.id;
        baranium_callback_t callback = (baranium_callback_t)baranium_dynlib_symbol(lib->dynlib, export.symname);
        baranium_callback_add(id, callback, export.num_params);
    }
}
