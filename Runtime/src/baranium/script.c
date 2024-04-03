#pragma warning(disable: 4996)

#include <baranium/runtime.h>
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
        fprintf(stderr, "Warning, script may be out of date, be sure to update your compiler and recompile the script");

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

index_t baranium_script_get_location_of(BaraniumScript* script, const char* name)
{
    if (script == NULL || name == NULL)
        return -1;
    
    BaraniumScriptNameTableEntry* current = script->NameTable.EntriesStart;
    for (uint64_t i = 0; i < script->NameTable.NameCount && current != NULL; i++)
    {
        if (strcmp(name, current->Name) == 0)
            return current->ID;
        
        current = current->next;
    }

    return -1;
}