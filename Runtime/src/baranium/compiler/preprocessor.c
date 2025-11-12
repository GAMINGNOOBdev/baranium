#include <baranium/compiler/language/language.h>
#include <baranium/compiler/preprocessor.h>
#include <baranium/compiler/source_token.h>
#include <baranium/compiler/source.h>
#include <baranium/string_util.h>
#include <baranium/file_util.h>
#include <baranium/logging.h>
#include <baranium/runtime.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BARANIUM_PREPROCESSOR_DEFINE_LIST_BUFFER_SIZE 0x20

typedef struct baranium_preprocessor_define_list
{
    size_t count;
    size_t buffer_size;
    
    index_t* hashes;
    baranium_source_token_list* replacements;
} baranium_preprocessor_define_list;

static void baranium_preprocessor_define_list_init(baranium_preprocessor_define_list* list)
{
    if (list == NULL)
        return;

    memset(list, 0, sizeof(baranium_preprocessor_define_list));
}

static void baranium_preprocessor_define_list_dispose(baranium_preprocessor_define_list* list)
{
    if (list == NULL || list->buffer_size == 0 || list->hashes == NULL || list->replacements == NULL)
        return;

    for (size_t i = 0; i < list->count; i++)
        baranium_source_token_list_dispose(&list->replacements[i]);

    free(list->hashes);
    free(list->replacements);

    memset(list, 0, sizeof(baranium_preprocessor_define_list));
}

static int baranium_preprocessor_define_list_get_index(baranium_preprocessor_define_list* list, const char* entry)
{
    if (list == NULL || list->hashes == NULL || list->replacements == NULL || entry == NULL)
        return -1;

    index_t hash = baranium_get_id_of_name(entry);
    for (size_t i = 0; i < list->count; i++)
    {
        if (hash == list->hashes[i])
            return i;
    }

    return -1;
}

static void baranium_preprocessor_define_list_set(baranium_preprocessor_define_list* list, int index, const char* replacement)
{
    if (list == NULL || index < 0 || index >= list->count || replacement == NULL)
        return;

    baranium_source_token_list replacementTokens;
    baranium_source_token_list_init(&replacementTokens);
    baranium_source_parse_single_line(&replacementTokens, replacement);

    list->replacements[index] = replacementTokens;
}

static void baranium_preprocessor_define_list_add(baranium_preprocessor_define_list* list, const char* define, const char* replacement)
{
    if (list == NULL || define == NULL || replacement == NULL)
        return;

    index_t hash = baranium_get_id_of_name(define);
    int index = baranium_preprocessor_define_list_get_index(list, define);
    if (index != -1)
    {
        baranium_preprocessor_define_list_set(list, index, replacement);
        return;
    }

    if (list->count + 1 >= list->buffer_size)
    {
        list->buffer_size += BARANIUM_PREPROCESSOR_DEFINE_LIST_BUFFER_SIZE;
        list->replacements = realloc(list->replacements, sizeof(baranium_source_token_list)*list->buffer_size);
        list->hashes = realloc(list->hashes, sizeof(index_t)*list->buffer_size);
    }

    baranium_source_token_list replacementTokens;
    baranium_source_token_list_init(&replacementTokens);
    baranium_source_parse_single_line(&replacementTokens, replacement);

    list->replacements[list->count] = replacementTokens;
    list->hashes[list->count] = hash;
    list->count++;
}

static baranium_string_list baranium_include_paths;
static baranium_preprocessor_define_list baranium_define_list;

void baranium_preprocessor_init(void)
{
    baranium_preprocessor_define_list_init(&baranium_define_list);
    baranium_include_paths = baranium_string_list_init();
}

void baranium_preprocessor_dispose(void)
{
    baranium_preprocessor_define_list_dispose(&baranium_define_list);
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
            include_file = strtrimleading(strdup(not_safe_for_work_path));
            include_path = (char*)baranium_preprocessor_search_include_path(include_file);
            if (include_path == NULL)
            {
                LOGERROR("Including file '%s' failed: Check path variable", include_file);
                free(include_file);
                include_file = NULL;
                continue;
            }

            FILE* file = fopen(include_path, "rb");

            if (file == NULL)
            {
                LOGERROR("Including file '%s' failed: File might be missing/corrupt (or path may not even point to a file!)", include_file);
                free(include_file);
                include_file = NULL;
                continue;
            }

            baranium_source_token_list src;
            baranium_source_token_list_init(&src);
            baranium_source_open_from_file(&src, file);
            baranium_source_token_list_push_list(source, &src);
            fclose(file);
            baranium_source_token_list_dispose(&src);
            free(include_file);
            include_file = NULL;
        }
        baranium_string_list_dispose(&paths);
        if (include_file)
            free(include_file);
        free(path_string);
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
    uint8_t modified = 0;
    if (path[len-1] == '\\' || path[len-1] == '/')
    {
        modified = path[len-1];
        ((char*)path)[len-1] = 0;
    }

    baranium_string_list_add(&baranium_include_paths, path);

    if (modified)
        ((char*)path)[len-1] = modified;
}

void baranium_preprocessor_pop_last_include(void)
{
    baranium_string_list_remove_last(&baranium_include_paths);
}

void baranium_preprocessor_add_define(const char* define, const char* replacement)
{
    if (define == NULL || strlen(define) < 1)
        return;

    if (baranium_preprocessor_define_list_get_index(&baranium_define_list, define) != -1)
        return;

    if (baranium_is_keyword(define) != -1 || strisnum(define) ||
        isdigit(define[0]) || baranium_is_special_char(define[0]) != -1)
        return;

    LOGDEBUG("define{'%s'} replacement{'%s'}", define, replacement);

    baranium_preprocessor_define_list_add(&baranium_define_list, define, replacement);
}

void baranium_preprocessor_assist_in_line(baranium_source_token_list* line_tokens)
{
    if (line_tokens == NULL || line_tokens->count == 0)
        return;

    for (size_t i = 0; i < line_tokens->count; i++)
    {
        baranium_source_token token = line_tokens->data[i];
        int index = baranium_preprocessor_define_list_get_index(&baranium_define_list, token.contents);
        if (index == -1)
            continue;

        baranium_source_token_list replacementTokens = baranium_define_list.replacements[index];
        index = i;
        baranium_source_token_list_remove(line_tokens, index);
        if (index != 0)
            index--;

        for (size_t i = 0; i < replacementTokens.count; i++)
            replacementTokens.data[i].line_number = token.line_number;

        if (index != 0)
            baranium_source_token_list_insert_after(line_tokens, index, &replacementTokens);
        else
            baranium_source_token_list_insert_start(line_tokens, &replacementTokens);
    }
}

const char* baranium_preprocessor_search_include_path(const char* file)
{
    baranium_string_list directory_contents;

    for (size_t i = 0; i < baranium_include_paths.count; i++)
    {
        const char* include_path = baranium_include_paths.strings[i];
        directory_contents = baranium_file_util_get_directory_contents(include_path, BARANIUM_FILE_UTIL_FILTER_MASK_ALL_FILES);
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
