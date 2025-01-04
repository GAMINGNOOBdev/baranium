#include "baranium/variable.h"
#include <baranium/backend/varmath.h>
#include <baranium/callback.h>
#include <baranium/runtime.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

void print_baranium(void** dataptr, baranium_variable_type_t* datatypes, int numData)
{
    if (dataptr == NULL || datatypes == NULL || numData == 0 || numData > 1)
        return;

    if (*datatypes == VARIABLE_TYPE_VOID || *datatypes == VARIABLE_TYPE_INVALID)
        return;

    size_t dataSize = baranium_variable_get_size_of_type(*datatypes);
    if (dataSize == (size_t)-1)
        dataSize = strlen(*dataptr);

    void* data = malloc(dataSize);
    memcpy(data, *dataptr, dataSize);
    baranium_compiled_variable var = {*datatypes, data, dataSize};
    baranium_compiled_variable_convert_to_type(&var, VARIABLE_TYPE_STRING);
    if (var.value != data)
        free(data);

    printf("%s\n", (const char*)var.value);

    free(var.value);
}

void input_baranium(void** dataptr, baranium_variable_type_t* datatypes, int numData)
{
    if (numData > 0 || dataptr != NULL || datatypes != NULL)
        return;

    size_t bufferSize = 0;
    char* line = NULL;
    while (getline(&line, &bufferSize, stdin) == -1);
    size_t length = strlen(line) - 1;

    char* buffer = malloc(length+1);
    memset(buffer, 0, length+1);
    memcpy(buffer, line, length);
    free(line);

    baranium_compiled_variable var = {VARIABLE_TYPE_STRING, buffer, length};
    baranium_compiled_variable_push_to_stack(baranium_get_context()->cpu, &var);

    free(buffer);
}

void system_baranium(void** dataptr, baranium_variable_type_t* datatypes, int numData)
{
    if (dataptr == NULL || datatypes == NULL || numData == 0 || numData > 1)
        return;

    if (*datatypes == VARIABLE_TYPE_VOID || *datatypes == VARIABLE_TYPE_INVALID)
        return;

    size_t dataSize = baranium_variable_get_size_of_type(*datatypes);
    if (dataSize == (size_t)-1)
        dataSize = strlen(*dataptr);

    void* data = malloc(dataSize);
    memcpy(data, *dataptr, dataSize);
    baranium_compiled_variable var = {*datatypes, data, dataSize};
    baranium_compiled_variable_convert_to_type(&var, VARIABLE_TYPE_STRING);
    if (var.value != data)
        free(var.value);

    system((const char*)var.value);

    free(data);
}

void exit_baranium(void** dataptr, baranium_variable_type_t* datatypes, int numData)
{
    if (dataptr != NULL || datatypes != NULL || numData != 0)
        return;

    baranium_get_context()->cpu->flags.FORCED_KILL = 1;
    baranium_get_context()->cpu->killTriggered = 1;
}

void setup_callbacks(void)
{
    baranium_callback_add(baranium_get_context(), baranium_get_id_of_name("print"), print_baranium, 1);
    baranium_callback_add(baranium_get_context(), baranium_get_id_of_name("input"), input_baranium, 0);
    baranium_callback_add(baranium_get_context(), baranium_get_id_of_name("system"), system_baranium, 1);
    baranium_callback_add(baranium_get_context(), baranium_get_id_of_name("exit"), exit_baranium, 1);
}
