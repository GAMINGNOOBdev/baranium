#include <baranium/file_util.h>
#include <baranium/logging.h>
#include <operations.h>
#include <commands.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <toml.h>
#include <util.h>

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
#   define OS_DELIMITER '\\'
#else
#   define OS_DELIMITER '/'
#endif

void cmd_build_clone_library(toml_property* library, int* status)
{
    const char* target = library->name;
    const char* source = library->value.stringValue;
    LOGINFO("Building library %s", target);
    const char* command = stringf("git clone %s .build/%s", source, target);
    system(command);

    if (!baranium_file_util_directory_exists(stringf(".build/%s", target)))
    {
        *status = 1;
        LOGERROR("Unable to find library '%s' from source '%s'", target, source);
    }

    system(stringf("cd .build/%s && %s build && %s install", target, executablename, executablename));
}

void cmd_build_create_defines_file(toml_section* section)
{
    FILE* defines_file = fopen(".build/defines.bar", "wb+");
    if (!defines_file)
        return;

    fprintf(defines_file, "# AUTOMATICALLY GENERATED FILE\n");
    fprintf(defines_file, "# ANY MANUAL MODIFICATIONS WILL BE DISCARDED\n\n");

    FOREACH_PROPERTY(define, section, {
        char* value = (char*)get_property_value_string(define);
        fprintf(defines_file, "+define %s %s", define->name, value);
        free(value);
    });

    fclose(defines_file);
}

const char* cmd_build_files = NULL;
void cmd_build_compile_file_callback(const char* path)
{
    if (!strstr(path, ".bar") && !strstr(path, ".bgs"))
        return;

    const char* tmp = cmd_build_files;
    if (cmd_build_files != NULL)
        cmd_build_files = strdup(stringf("%s src/%s", cmd_build_files, path));
    else
        cmd_build_files = strdup(stringf("src/%s", path));

    if (tmp != NULL)
        free((void*)tmp);
}

char* cmd_build_get_build_command(toml_section* cfg, const char* execdir, const char* outputdir, const char* includedir)
{
    char* result = NULL;
    toml_property* property = toml_section_get(cfg, "build.library");
    int is_library = (property ? (property->value.boolValue ? 1 : 0) : 0);

    char* libraries = strdup("");
    toml_section* libraries_section = toml_section_get_section(cfg, "libraries");
    FOREACH_PROPERTY(lib, libraries_section, {
        char* tmp;
        tmp = libraries;
        libraries = strdup(stringf("%s-l %s ", libraries, lib->name));
        free(tmp);
    });

    cmd_build_files = strdup(".build/defines.bar");
    baranium_file_util_iterate_directory("src", BARANIUM_FILE_UTIL_FILTER_MASK_ALL_FILES, cmd_build_compile_file_callback);
    if (cmd_build_files == NULL)
    {
        LOGERROR("No baranium source files found in 'src'");
        return NULL;
    }

    const char* project_name = toml_section_get(cfg, "build.project_name")->value.stringValue;
    if (strlen(execdir) != 0)
        result = strdup(stringf("%s%cbarc -d -o %s%c%s -i %s %s%s%s", execdir, OS_DELIMITER, outputdir, OS_DELIMITER, project_name, includedir, is_library ? "-e " : "", libraries ? libraries : "", cmd_build_files));
    else
        result = strdup(stringf("barc -d -o %s%c%s -i %s %s%s%s", outputdir, OS_DELIMITER, project_name, includedir, is_library ? "-e " : "", libraries ? libraries : "", cmd_build_files));

    free((void*)cmd_build_files);
    free(libraries);
    return result;
}

cmd_args_t cmd_build(cmd_args_t* userparam)
{
    cmd_args_t args = *userparam;
    int status = 0;
    toml_section cfg = TOML_SECTION_EMPTY;
    if (!open_project_file(&cfg))
    {
        LOGERROR("Cannot find project file in current directory");
        return EMPTY_CMD_ARGS;
    }

    if (!toml_section_has_property(&cfg, "build.project_name"))
    {
        toml_section_dispose(&cfg);
        LOGERROR("Invalid project file, missing 'project_name' property");
        return EMPTY_CMD_ARGS;
    }

    if (!baranium_file_util_directory_exists(".build"))
        baranium_file_util_create_directory(".build");

    if (!baranium_file_util_directory_exists(".build/library-bin"))
        baranium_file_util_create_directory(".build/library-bin");

    toml_property* property = NULL;
    toml_section* section = NULL;

    property = toml_section_get(&cfg, "build.prebuild_commands");
    if (property)
    {
        for (int i = 0; i < property->arrayLength; i++)
            if (property->value.arrayValue[i].type == TOML_PROPERTY_TYPE_STRING)
                system(property->value.arrayValue[i].value.stringValue);
    }

    section = toml_section_get_section(&cfg, "libraries");
    FOREACH_PROPERTY(library, section, {
        cmd_build_clone_library(library, &status);
    })

    if (status != 0)
        goto build_end;

    const char* output_directory = "bin";
    property = toml_section_get(&cfg, "build.output");
    if (property)
        output_directory = property->value.stringValue;

    if (!baranium_file_util_directory_exists(output_directory))
        baranium_file_util_create_directory(output_directory);

    cmd_build_create_defines_file(toml_section_get_section(&cfg, "defines"));

    const char* includedir = "src";
    property = toml_section_get(&cfg, "build.include");
    if (property)
        includedir = property->value.stringValue;

    char* execname_dirname = (char*)executablename+strlen(executablename)-1;
    for (; *execname_dirname != OS_DELIMITER; execname_dirname--);
    execname_dirname[0] = 0;
    const char* build_command = cmd_build_get_build_command(&cfg, executablename, output_directory, includedir);
    execname_dirname[0] = OS_DELIMITER;
    if (!build_command)
        goto build_end;
    system(build_command);
    free((void*)build_command);

build_end:
    toml_section_dispose(&cfg);
    return args;
}
