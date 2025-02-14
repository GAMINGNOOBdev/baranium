#include <baranium/backend/varmath.h>
#include <baranium/variable.h>
#include <baranium/callback.h>
#include <baranium/runtime.h>
#include <baranium/bcpu.h>
#include <assert.h>
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

void print_baranium(baranium_callback_data_list_t* data)
{
    BARANIUM_CALLBACK_INIT(data, 1, 1);
    BARANIUM_CALLBACK_GET_VARIABLE_VALUE(char*, string, NULL, BARANIUM_VARIABLE_TYPE_STRING, 0);

    printf("%s", string);
}

void input_baranium(baranium_callback_data_list_t* data)
{
    BARANIUM_CALLBACK_INIT(data, 0, 0);

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

    baranium_compiled_variable input = {BARANIUM_VARIABLE_TYPE_STRING, {.ptr=buffer}, length};
    BARANIUM_CALLBACK_RETURN_VARIABLE(input);

    free(buffer);
}

void system_baranium(baranium_callback_data_list_t* data)
{
    BARANIUM_CALLBACK_INIT(data, 1, 1);
    BARANIUM_CALLBACK_GET_VARIABLE_VALUE(char*, command, NULL, BARANIUM_VARIABLE_TYPE_STRING, 0);

    system(command);
}

void exit_baranium(baranium_callback_data_list_t* data)
{
    BARANIUM_CALLBACK_INIT(data, 0, 0);

    baranium_get_context()->cpu->flags.FORCED_KILL = 1;
    baranium_get_context()->cpu->kill_triggered = 1;
}

void math_function_baranium(baranium_callback_data_list_t* data, float(*funcptr)(float))
{
    BARANIUM_CALLBACK_INIT(data, 1, 1);
    if (funcptr == NULL)
        return;

    BARANIUM_CALLBACK_GET_VARIABLE(number, BARANIUM_VARIABLE_TYPE_FLOAT, 0);

    number.value.numfloat = funcptr(number.value.numfloat);

    BARANIUM_CALLBACK_RETURN_VARIABLE(number);
}

void sin_baranium(baranium_callback_data_list_t* data) { math_function_baranium(data, sinf); }
void cos_baranium(baranium_callback_data_list_t* data) { math_function_baranium(data, cosf); }
void tan_baranium(baranium_callback_data_list_t* data) { math_function_baranium(data, tanf); }
void asin_baranium(baranium_callback_data_list_t* data) { math_function_baranium(data, asinf); }
void acos_baranium(baranium_callback_data_list_t* data) { math_function_baranium(data, acosf); }
void atan_baranium(baranium_callback_data_list_t* data) { math_function_baranium(data, atanf); }
void log_baranium(baranium_callback_data_list_t* data) { math_function_baranium(data, logf); }
void log10_baranium(baranium_callback_data_list_t* data) { math_function_baranium(data, log10f); }

void setup_callbacks(void)
{
    baranium_callback_add(baranium_get_id_of_name("print"), print_baranium, 1);
    baranium_callback_add(baranium_get_id_of_name("input"), input_baranium, 0);
    baranium_callback_add(baranium_get_id_of_name("system"), system_baranium, 1);
    baranium_callback_add(baranium_get_id_of_name("exit"), exit_baranium, 1);
    baranium_callback_add(baranium_get_id_of_name("sin"), sin_baranium, 1);
    baranium_callback_add(baranium_get_id_of_name("cos"), cos_baranium, 1);
    baranium_callback_add(baranium_get_id_of_name("tan"), tan_baranium, 1);
    baranium_callback_add(baranium_get_id_of_name("asin"), asin_baranium, 1);
    baranium_callback_add(baranium_get_id_of_name("acos"), acos_baranium, 1);
    baranium_callback_add(baranium_get_id_of_name("atan"), atan_baranium, 1);
    baranium_callback_add(baranium_get_id_of_name("log"), log_baranium, 1);
    baranium_callback_add(baranium_get_id_of_name("log10"), log10_baranium, 1);
}
