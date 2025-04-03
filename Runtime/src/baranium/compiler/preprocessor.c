#include <baranium/compiler/language/language.h>
#include <baranium/compiler/preprocessor.h>
#include <baranium/compiler/source_token.h>
#include <baranium/compiler/source.h>
#include <baranium/string_util.h>
#include <baranium/file_util.h>
#include <baranium/logging.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static baranium_string_map baranium_defines_map;
static baranium_string_list baranium_include_paths;

void baranium_preprocessor_init(void)
{
    baranium_string_map_init(&baranium_defines_map);
    baranium_string_list_init(&baranium_include_paths);
}

void baranium_preprocessor_dispose(void)
{
    baranium_string_map_dispose(&baranium_defines_map);
    baranium_string_list_dispose(&baranium_include_paths);
}

void baranium_preprocessor_parse(const char* operation, baranium_source_token_list* source)
{
    if (operation == NULL || source == NULL)
        return;

    if (strlen(operation) < 1)
        return;

    char* clean_buffer = strtrimleading(operation);

    if (strcmp(clean_buffer,"include") == 0)
    {
        printf("haha, funni. including nothing, huh? well that's certainly strange but i know something stranger and it's the fact that there even is a message like this that catches that specific case of not having a single file that you want to include in a include statement. you really deserve a gold medal for finding this secret little easter egg my friend. have a good day.\n");
        return;
    }

    char* command = strsubstr(clean_buffer, 0, stridx(clean_buffer, ' '));
    // there are currently no preprocessor commands that are just standalone
    if (command == NULL || strlen(command) < 1)
        return;

    // make sure this is actually an include
    if (strcmp(command, "include") == 0)
    {
        char* path_string = strsubstr(clean_buffer, 7, -1);
        path_string = strtrimleading(path_string);
        baranium_string_list paths;
        strsplit(&paths, path_string, ',');
        char* include_file;
        char* include_path;
        for (size_t i = 0; i < paths.count; i++)
        {
            const char* not_safe_for_work_path = paths.strings[i];
            include_file = strtrimleading(not_safe_for_work_path);
            include_path = (char*)baranium_preprocessor_search_include_path(include_file);
            if (include_path == NULL)
            {
                LOGERROR(stringf("Including file '%s' failed: Check path variable", include_file));
                continue;
            }

            FILE* file = fopen(include_path, "rb");

            if (file == NULL)
            {
                LOGERROR(stringf("Including file '%s' failed: File might be missing/corrupt (or path may not even point to a file!)", include_file));
                continue;
            }

            baranium_source_token_list src;
            baranium_source_token_list_init(&src);
            baranium_source_open_from_file(&src, file);
            baranium_source_token_list_push_list(source, &src);
            fclose(file);
            baranium_source_token_list_dispose(&src, 1);
        }
        free(path_string);
        baranium_string_list_dispose(&paths);
    }
    else if (strcmp(command, "define") == 0)
    {
        char* name_and_replacement = strtrimleading(strsubstr(clean_buffer, 6, -1));
        if (name_and_replacement == NULL || strlen(name_and_replacement) < 1)
            return;

        int nameSplitIndex = stridx(name_and_replacement, ' ');
        if (nameSplitIndex == -1)
        {
            baranium_preprocessor_add_define(name_and_replacement, "");
            return;
        }

        char* define = strtrimleading(strsubstr(name_and_replacement, 0,nameSplitIndex));
        char* define_value = strtrimleading(strsubstr(name_and_replacement, nameSplitIndex, -1));
        baranium_preprocessor_add_define(define, define_value);
        free(define);
        free(define_value);
        free(name_and_replacement);
    }

    free(command);
}

void baranium_preprocessor_add_include_path(const char* path)
{
    size_t len = strlen(path);
    if (path[len-1] == '\\' || path[len-1] == '/')
        ((char*)path)[len-1] = 0;

    baranium_string_list_add(&baranium_include_paths, path);
}

void baranium_preprocessor_pop_last_include(void)
{
    baranium_string_list_remove_last(&baranium_include_paths);
}

void baranium_preprocessor_add_define(const char* define, const char* replacement)
{
    if (define == NULL || strlen(define) < 1)
        return;

    if (baranium_string_map_get_index(&baranium_defines_map, define) != -1)
        return;

    if (baranium_is_keyword(define) != -1 || strisnum(define) ||
        isdigit(define[0]) || baranium_is_special_char(define[0]) != -1)
        return;

    LOGDEBUG(stringf("define{'%s'} replacement{'%s'}", define, replacement));

    baranium_string_map_add(&baranium_defines_map, define, replacement);
}

void baranium_preprocessor_assist_in_line(baranium_source_token_list* line_tokens)
{
    if (line_tokens == NULL || line_tokens->count == 0)
        return;

    baranium_source_token_list improved_tokens;
    baranium_source_token_list_init(&improved_tokens);

    uint8_t anything_changed = 0;
    for (size_t i = 0; i < line_tokens->count; i++)
    {
        baranium_source_token token = line_tokens->data[i];
        int index = baranium_string_map_get_index(&baranium_defines_map, token.contents);
        if (index == -1)
        {
            baranium_source_token_list_add(&improved_tokens, &token);
            continue;
        }

        anything_changed = 1;
        const char* replacementText = baranium_defines_map.strings[index];
        if (strlen(replacementText) < 1)
            continue;

        baranium_source_token_list replacementTokens;
        baranium_source_token_list_init(&replacementTokens);
        baranium_source_parse_single_line(&replacementTokens, replacementText);
        if (replacementTokens.count == 0)
            continue;

        if (replacementTokens.count == 1)
        {
            replacementTokens.data[0].line_number = token.line_number;
            baranium_source_token_list_add(&improved_tokens, replacementTokens.data);
            free(replacementTokens.data);
            continue;
        }

        for (size_t i = 0; i < replacementTokens.count; i++)
            replacementTokens.data[i].line_number = token.line_number;

        baranium_source_token_list_push_list(&improved_tokens, &replacementTokens);
        free(replacementTokens.data);

    }
    if (!anything_changed)
    {
        free(improved_tokens.data);
        return;
    }

    baranium_source_token_list_clear(line_tokens);
    baranium_source_token_list_push_list(line_tokens, &improved_tokens);
    free(improved_tokens.data);
}

const char* baranium_preprocessor_search_include_path(const char* file)
{
    baranium_string_list directory_contents;

    for (size_t i = 0; i < baranium_include_paths.count; i++)
    {
        const char* include_path = baranium_include_paths.strings[i];
        baranium_string_list_init(&directory_contents);
        baranium_file_util_get_directory_contents(&directory_contents, include_path, BARANIUM_FILTER_MASK_ALL_FILES);
        for (size_t i = 0; i < directory_contents.count; i++)
        {
            const char* filename = directory_contents.strings[i];
            if (strcmp(filename, file) == 0 || strcmp(filename, stringf("%s.bgs", file)) == 0 || strcmp(filename, stringf("%s.ib", file)) == 0)
            {
                const char* result = stringf("%s/%s", include_path, filename);
                baranium_string_list_dispose(&directory_contents);
                return result;
            }
        }
        baranium_string_list_dispose(&directory_contents);
    }

    return NULL;
}
