#include <baranium/backend/varmath.h>
#include <baranium/variable.h>
#include <baranium/callback.h>
#include <baranium/logging.h>
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
size_t getdelim(char **buffer, size_t *buffersz, FILE *stream, char delim)
{
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (buffer == NULL || stream == NULL || buffersz == NULL)
        return -1;

    bufptr = *buffer;
    size = *buffersz;

    c = fgetc(stream);
    if (c == EOF)
        return -1;

    if (bufptr == NULL)
    {
        bufptr = malloc(128);
        if (bufptr == NULL)
            return -1;

        size = 128;
    }
    p = bufptr;
    while(c != EOF)
    {
        if ((p - bufptr) > (size - 1))
        {
            size = size + 128;
            bufptr = realloc(bufptr, size);
            if (bufptr == NULL)
                return -1;

        }
        *p++ = c;
        if (c == delim)
            break;
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

BARANIUM_CALLBACK_DEFINE(fopen)
{
    BARANIUM_CALLBACK_INIT(2, 2);
    BARANIUM_CALLBACK_GET_VARIABLE_VALUE(char*, filename, NULL, BARANIUM_VARIABLE_TYPE_STRING, 0);
    BARANIUM_CALLBACK_GET_VARIABLE_VALUE(char*, filemode, NULL, BARANIUM_VARIABLE_TYPE_STRING, 1);

    FILE* file = NULL;
    if (filename != NULL && filemode != NULL)
        file = fopen(filename, filemode);

    baranium_compiled_variable result = {BARANIUM_VARIABLE_TYPE_OBJECT, {.ptr = file}, baranium_variable_get_size_of_type(BARANIUM_VARIABLE_TYPE_OBJECT)};
    BARANIUM_CALLBACK_RETURN_VARIABLE(result);
}

BARANIUM_CALLBACK_DEFINE(fclose)
{
    BARANIUM_CALLBACK_INIT(1, 1);
    BARANIUM_CALLBACK_GET_VARIABLE_VALUE(FILE*, file, NULL, BARANIUM_VARIABLE_TYPE_OBJECT, 0);

    if (file)
        fclose(file);
}

BARANIUM_CALLBACK_DEFINE(fseek)
{
    BARANIUM_CALLBACK_INIT(3, 3);
    BARANIUM_CALLBACK_GET_VARIABLE_VALUE(FILE*, file, NULL, BARANIUM_VARIABLE_TYPE_OBJECT, 0);
    BARANIUM_CALLBACK_GET_VARIABLE_VALUE(int, off, 0, BARANIUM_VARIABLE_TYPE_INT32, 1);
    BARANIUM_CALLBACK_GET_VARIABLE_VALUE(int, whence, 0, BARANIUM_VARIABLE_TYPE_INT32, 2);

    if (file != NULL)
        fseek(file, off, whence);
}


BARANIUM_CALLBACK_DEFINE(print)
{
    BARANIUM_CALLBACK_INIT(1, 1);
    BARANIUM_CALLBACK_GET_VARIABLE_VALUE(char*, string, NULL, BARANIUM_VARIABLE_TYPE_STRING, 0);

    printf("%s", string);
}

BARANIUM_CALLBACK_DEFINE(log_info)
{
    BARANIUM_CALLBACK_INIT(1, 1);
    BARANIUM_CALLBACK_GET_VARIABLE_VALUE(char*, string, NULL, BARANIUM_VARIABLE_TYPE_STRING, 0);

    LOGINFO(string);
}

BARANIUM_CALLBACK_DEFINE(log_debug)
{
    BARANIUM_CALLBACK_INIT(1, 1);
    BARANIUM_CALLBACK_GET_VARIABLE_VALUE(char*, string, NULL, BARANIUM_VARIABLE_TYPE_STRING, 0);

    LOGDEBUG(string);
}

BARANIUM_CALLBACK_DEFINE(log_error)
{
    BARANIUM_CALLBACK_INIT(1, 1);
    BARANIUM_CALLBACK_GET_VARIABLE_VALUE(char*, string, NULL, BARANIUM_VARIABLE_TYPE_STRING, 0);

    LOGERROR(string);
}

BARANIUM_CALLBACK_DEFINE(log_warning)
{
    BARANIUM_CALLBACK_INIT(1, 1);
    BARANIUM_CALLBACK_GET_VARIABLE_VALUE(char*, string, NULL, BARANIUM_VARIABLE_TYPE_STRING, 0);

    LOGWARNING(string);
}

BARANIUM_CALLBACK_DEFINE(input)
{
    BARANIUM_CALLBACK_INIT(0, 0);

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

BARANIUM_CALLBACK_DEFINE(system)
{
    BARANIUM_CALLBACK_INIT(1, 1);
    BARANIUM_CALLBACK_GET_VARIABLE_VALUE(char*, command, NULL, BARANIUM_VARIABLE_TYPE_STRING, 0);

    system(command);
}

BARANIUM_CALLBACK_DEFINE(exit)
{
    BARANIUM_CALLBACK_INIT(0, 0);

    baranium_get_runtime()->cpu->flags.FORCED_KILL = 1;
    baranium_get_runtime()->cpu->kill_triggered = 1;
}

void baranium_callback_math_function(baranium_callback_data_list_t* data, float(*funcptr)(float))
{
    BARANIUM_CALLBACK_INIT(1, 1);
    if (funcptr == NULL)
        return;

    BARANIUM_CALLBACK_GET_VARIABLE(number, BARANIUM_VARIABLE_TYPE_FLOAT, 0);

    number.value.numfloat = funcptr(number.value.numfloat);

    BARANIUM_CALLBACK_RETURN_VARIABLE(number);
}

BARANIUM_CALLBACK_DEFINE(sin) { baranium_callback_math_function(data, sinf); }
BARANIUM_CALLBACK_DEFINE(cos) { baranium_callback_math_function(data, cosf); }
BARANIUM_CALLBACK_DEFINE(tan) { baranium_callback_math_function(data, tanf); }
BARANIUM_CALLBACK_DEFINE(asin) { baranium_callback_math_function(data, asinf); }
BARANIUM_CALLBACK_DEFINE(acos) { baranium_callback_math_function(data, acosf); }
BARANIUM_CALLBACK_DEFINE(atan) { baranium_callback_math_function(data, atanf); }
BARANIUM_CALLBACK_DEFINE(log) { baranium_callback_math_function(data, logf); }
BARANIUM_CALLBACK_DEFINE(log10) { baranium_callback_math_function(data, log10f); }

void setup_callbacks(void)
{
    BARANIUM_CALLBACK_ADD(fopen, 2);
    BARANIUM_CALLBACK_ADD(fseek, 3);
    BARANIUM_CALLBACK_ADD(fclose, 1);
    BARANIUM_CALLBACK_ADD(log_info, 1);
    BARANIUM_CALLBACK_ADD(log_debug, 1);
    BARANIUM_CALLBACK_ADD(log_error, 1);
    BARANIUM_CALLBACK_ADD(log_warning, 1);
    BARANIUM_CALLBACK_ADD(print, 1);
    BARANIUM_CALLBACK_ADD(input, 0);
    BARANIUM_CALLBACK_ADD(system, 1);
    BARANIUM_CALLBACK_ADD(exit, 1);
    BARANIUM_CALLBACK_ADD(sin, 1);
    BARANIUM_CALLBACK_ADD(cos, 1);
    BARANIUM_CALLBACK_ADD(tan, 1);
    BARANIUM_CALLBACK_ADD(asin, 1);
    BARANIUM_CALLBACK_ADD(acos, 1);
    BARANIUM_CALLBACK_ADD(atan, 1);
    BARANIUM_CALLBACK_ADD(log, 1);
    BARANIUM_CALLBACK_ADD(log10, 1);
}
