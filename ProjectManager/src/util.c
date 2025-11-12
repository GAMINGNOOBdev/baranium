#include <baranium/logging.h>
#include <string.h>
#include <stdlib.h>
#include <util.h>

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
#   include <Windows.h>
#   undef max
#   undef min
#   define OS_DELIMITER '\\'
#else
#   include <sys/stat.h>
#   include <unistd.h>
#   define OS_DELIMITER '/'
#endif

char* get_executable_working_directory(void)
{
    static char result[0x1000];

    #if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
        DWORD status = GetModuleFileNameA(NULL, &result[0], 0x1000);
        if (status == ERROR)
            return "";
    #else
        if (readlink("/proc/self/exe", result, 0x1000) == -1)
            return "";
    #endif

    return result;
}

char* get_current_working_directory(void)
{
    static char cwd[0x1000];
    static int cwd_fetched = 0;

    if (cwd_fetched)
        return cwd;

    #if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
    DWORD status = GetCurrentDirectoryA(0x1000, cwd);
    if (status == 0)
        return "";
    #else
    if (getcwd(cwd, 0x1000) == NULL)
        return "";
    #endif

    cwd_fetched = 1;
    return cwd;
}

int open_project_file_from(config_file_t* file, const char* path)
{
    FILE* config_file = fopen(path, "r");
    if (config_file == NULL)
        return 0;

    config_file_open(file, config_file);
    fclose(config_file);
    return 1;
}

int open_project_file(config_file_t* file)
{
    char* cwd = get_current_working_directory();
    FILE* config_file = fopen(stringf("%s/barproject.toml", cwd), "r");
    if (config_file == NULL)
        return 0;

    config_file_open(file, config_file);
    fclose(config_file);
    return 1;
}

int save_project_file(config_file_t* file)
{
    char* cwd = get_current_working_directory();
    FILE* config_file = fopen(stringf("%s/barproject.toml", cwd), "wb+");
    if (config_file == NULL)
        return 0;

    config_file_save(file, config_file);
    fclose(config_file);
    return 1;
}

int copy(const char* input, const char* output)
{
    if (!output || !input)
        return 0;

    FILE* inputfile = fopen(input, "r");
    if (!inputfile)
        return -1;

    FILE* outputfile = fopen(output, "wb+");
    if (!outputfile)
    {
        fclose(inputfile);
        return 1;
    }
    uint8_t buffer[BUFSIZ];
    size_t read, write;

    while (!feof(inputfile))
    {
        read = fread(buffer, sizeof(uint8_t), BUFSIZ, inputfile);
        if (!read)
            continue;

        write = fwrite(buffer, sizeof(uint8_t), read, outputfile);
        if (write != read)
            break;
    }

    fclose(inputfile);
    fclose(outputfile);
    return 0;
}

const char* get_property_value_string(config_property_t* property)
{
    if (property == NULL)
        return strdup("default");

    switch (property->type)
    {
    case CONFIG_PROPERTY_VALUE_TYPE_INT:
        return strdup(stringf("%d", property->value.intValue));

    case CONFIG_PROPERTY_VALUE_TYPE_FLOAT:
        return strdup(stringf("%f", property->value.floatValue));

    case CONFIG_PROPERTY_VALUE_TYPE_BOOL:
        return strdup(property->value.boolValue ? "true" : "false");

    case CONFIG_PROPERTY_VALUE_TYPE_ARRAY:
    {
        char result[0x100];
        result[0] = '[';
        result[1] = ' ';
        result[2] = '\0';
        for (int i = 0; i < property->arrayLength; i++)
        {
            const char* value = get_property_value_string(&property->value.arrayValue[i]);
            strcat(result, value);
            free((void*)value);
            if (i != property->arrayLength-1)
                strcat(result, ", ");
        }
        strcat(result, " ]");
        return strdup(result);
    }

    case CONFIG_PROPERTY_VALUE_TYPE_UNKNOWN:
    case CONFIG_PROPERTY_VALUE_TYPE_STRING:
    {
        if (property->value.stringValue != NULL && strlen(property->value.stringValue) > 0)
            return strdup(stringf("'%s'", property->value.stringValue));
    }
    }

    return strdup("");
}
