#include "toml.h"
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

int open_project_file_from(toml_section* file, const char* path)
{
    FILE* toml_file = fopen(path, "r");
    if (toml_file == NULL)
        return 0;

    *file = toml_file_open(toml_file);
    fclose(toml_file);
    return 1;
}

int open_project_file(toml_section* file)
{
    char* cwd = get_current_working_directory();
    FILE* toml_file = fopen(stringf("%s/barproject.toml", cwd), "r");
    if (toml_file == NULL)
        return 0;

    *file = toml_file_open(toml_file);
    fclose(toml_file);
    return 1;
}

int save_project_file(toml_section* file)
{
    char* cwd = get_current_working_directory();
    FILE* toml_file = fopen(stringf("%s/barproject.toml", cwd), "wb+");
    if (toml_file == NULL)
        return 0;

    toml_section_save(file, toml_file, NULL);
    fclose(toml_file);
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

const char* get_property_value_string(toml_property* property)
{
    if (property == NULL)
        return strdup("default");

    char result[0x1000] = {0};
    toml_property_as_str(property, result, 0x1000);
    return strdup(result);
}
