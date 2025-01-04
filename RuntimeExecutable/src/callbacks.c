#include <baranium/backend/varmath.h>
#include <baranium/variable.h>
#include <baranium/callback.h>
#include <baranium/runtime.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_APPLE
#   include <stdlib.h>
#else
#   include <malloc.h>
#endif

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
size_t getdelim(char **buffer, size_t *buffersz, FILE *stream, char delim) {
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (buffer == NULL) {
        return -1;
    }
    if (stream == NULL) {
        return -1;
    }
    if (buffersz == NULL) {
        return -1;
    }
    bufptr = *buffer;
    size = *buffersz;

    c = fgetc(stream);
    if (c == EOF) {
        return -1;
    }
    if (bufptr == NULL) {
        bufptr = malloc(128);
        if (bufptr == NULL) {
            return -1;
        }
        size = 128;
    }
    p = bufptr;
    while(c != EOF) {
        if ((p - bufptr) > (size - 1)) {
            size = size + 128;
            bufptr = realloc(bufptr, size);
            if (bufptr == NULL) {
                return -1;
            }
        }
        *p++ = c;
        if (c == delim) {
            break;
        }
        c = fgetc(stream);
    }

    *p++ = '\0';
    *buffer = bufptr;
    *buffersz = size;

    return p - bufptr - 1;
}

size_t getline(char **buffer, size_t *buffersz, FILE *stream)
{
    return getdelim(buffer, buffersz, stream, '\n');
}
#endif

void print_baranium(void** dataptr, baranium_variable_type_t* datatypes, int numData)
{
    if (dataptr == NULL || datatypes == NULL || numData == 0 || numData > 1)
        return;

    if (*datatypes == VARIABLE_TYPE_VOID || *datatypes == VARIABLE_TYPE_INVALID)
        return;

    size_t dataSize = baranium_variable_get_size_of_type(*datatypes);
    if (dataSize == (size_t)-1)
        dataSize = strlen(*dataptr);

    void* data = malloc(dataSize+1);
    if (data == NULL)
        return;

    memset(data, 0, dataSize + 1);
    memcpy(data, *dataptr, dataSize);
    baranium_compiled_variable var = {*datatypes, data, dataSize};
    baranium_compiled_variable_convert_to_type(&var, VARIABLE_TYPE_STRING);
    size_t length = strlen((const char*)var.value);
    if (length != var.size)
    {
        free(var.value);
        return;
    }

    printf("%s", (const char*)var.value);

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
    if (buffer == NULL)
    {
        free(line);
        return;
    }
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
    if (data == NULL)
        return;

    memset(data, 0, dataSize + 1);
    memcpy(data, *dataptr, dataSize);
    baranium_compiled_variable var = {*datatypes, data, dataSize};
    baranium_compiled_variable_convert_to_type(&var, VARIABLE_TYPE_STRING);
    size_t length = strlen((const char*)var.value);
    if (length != var.size)
    {
        free(var.value);
        return;
    }

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

void math_function_baranium(void** dataptr, baranium_variable_type_t* datatypes, int numData, float(*funcptr)(float))
{
    if (dataptr == NULL || datatypes == NULL || numData == 0 || numData > 1 || funcptr == NULL)
        return;

    if (*datatypes == VARIABLE_TYPE_VOID || *datatypes == VARIABLE_TYPE_INVALID)
        return;

    size_t dataSize = baranium_variable_get_size_of_type(*datatypes);
    if (dataSize == (size_t)-1)
        return;

    void* data = malloc(dataSize);
    if (data == NULL)
        return;

    memcpy(data, *dataptr, dataSize);

    baranium_compiled_variable var = {*datatypes, data, dataSize};
    baranium_compiled_variable_convert_to_type(&var, VARIABLE_TYPE_FLOAT);

    *((float*)var.value) = funcptr(*((float*)var.value));

    baranium_compiled_variable_push_to_stack(baranium_get_context()->cpu, &var);

    free(var.value);
}

void sin_baranium(void** dataptr, baranium_variable_type_t* datatypes, int numData) { math_function_baranium(dataptr, datatypes, numData, sinf); }
void cos_baranium(void** dataptr, baranium_variable_type_t* datatypes, int numData) { math_function_baranium(dataptr, datatypes, numData, cosf); }
void tan_baranium(void** dataptr, baranium_variable_type_t* datatypes, int numData) { math_function_baranium(dataptr, datatypes, numData, tanf); }
void asin_baranium(void** dataptr, baranium_variable_type_t* datatypes, int numData) { math_function_baranium(dataptr, datatypes, numData, asinf); }
void acos_baranium(void** dataptr, baranium_variable_type_t* datatypes, int numData) { math_function_baranium(dataptr, datatypes, numData, acosf); }
void atan_baranium(void** dataptr, baranium_variable_type_t* datatypes, int numData) { math_function_baranium(dataptr, datatypes, numData, atanf); }
void log_baranium(void** dataptr, baranium_variable_type_t* datatypes, int numData) { math_function_baranium(dataptr, datatypes, numData, logf); }
void log10_baranium(void** dataptr, baranium_variable_type_t* datatypes, int numData) { math_function_baranium(dataptr, datatypes, numData, log10f); }

void setup_callbacks(void)
{
    baranium_callback_add(baranium_get_context(), baranium_get_id_of_name("print"), print_baranium, 1);
    baranium_callback_add(baranium_get_context(), baranium_get_id_of_name("input"), input_baranium, 0);
    baranium_callback_add(baranium_get_context(), baranium_get_id_of_name("system"), system_baranium, 1);
    baranium_callback_add(baranium_get_context(), baranium_get_id_of_name("exit"), exit_baranium, 1);
    baranium_callback_add(baranium_get_context(), baranium_get_id_of_name("sin"), sin_baranium, 1);
    baranium_callback_add(baranium_get_context(), baranium_get_id_of_name("cos"), cos_baranium, 1);
    baranium_callback_add(baranium_get_context(), baranium_get_id_of_name("tan"), tan_baranium, 1);
    baranium_callback_add(baranium_get_context(), baranium_get_id_of_name("asin"), asin_baranium, 1);
    baranium_callback_add(baranium_get_context(), baranium_get_id_of_name("acos"), acos_baranium, 1);
    baranium_callback_add(baranium_get_context(), baranium_get_id_of_name("atan"), atan_baranium, 1);
    baranium_callback_add(baranium_get_context(), baranium_get_id_of_name("log"), log_baranium, 1);
    baranium_callback_add(baranium_get_context(), baranium_get_id_of_name("log10"), log10_baranium, 1);
}
