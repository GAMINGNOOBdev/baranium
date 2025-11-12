#include <baranium/compiler/binaries/compiler.h>
#include <baranium/compiler/compiler_context.h>
#include <baranium/compiler/token_parser.h>
#include <baranium/compiler/source_token.h>
#include <baranium/compiler/preprocessor.h>
#include <baranium/compiler/source.h>
#include <baranium/string_util.h>
#include <baranium/file_util.h>
#include <baranium/logging.h>
#include <baranium/runtime.h>
#include <baranium/library.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

static baranium_compiler_context* current_active_compiler_context = NULL;

baranium_compiler_context* baranium_compiler_context_init(void)
{
    baranium_compiler_context* ctx = malloc(sizeof(baranium_compiler_context));
    if (!ctx)
        return NULL;

    memset(ctx, 0, sizeof(baranium_compiler_context));
    baranium_source_token_list_init(&ctx->combined_source);
    baranium_token_parser_init(&ctx->token_parser);
    ctx->nametable = baranium_string_map_init();
    baranium_preprocessor_init();

    // has to be added so that the compiler will actually create the variables and such
    baranium_preprocessor_add_define("int", "int32");
    baranium_preprocessor_add_define("uint", "uint32");

    ctx->library_dir_contents = baranium_string_list_init();

    if (current_active_compiler_context == NULL)
        baranium_set_compiler_context(ctx);

    return ctx;
}

void baranium_set_compiler_context(baranium_compiler_context* ctx)
{
    current_active_compiler_context = ctx;
}

baranium_compiler_context* baranium_get_compiler_context(void)
{
    return current_active_compiler_context;
}

void baranium_compiler_context_dispose(baranium_compiler_context* ctx)
{
    if (ctx == NULL)
        return;

    for (size_t i = 0; i < ctx->library_count; i++)
        baranium_library_dispose(ctx->libraries[i]);

    if (ctx->libraries != NULL)
        free(ctx->libraries);

    if (ctx->library_dir_path != NULL)
    {
        free((void*)ctx->library_dir_path);
        baranium_string_list_dispose(&ctx->library_dir_contents);
    }

    baranium_source_token_list_dispose(&ctx->combined_source);
    baranium_token_parser_dispose(&ctx->token_parser);
    baranium_string_map_dispose(&ctx->nametable);
    baranium_preprocessor_dispose();

    free(ctx);
}

void baranium_compiler_context_set_library_directory(baranium_compiler_context* ctx, const char* str)
{
    if (ctx == NULL || str == NULL)
        return;

    if (ctx->library_dir_path != NULL)
    {
        free((void*)ctx->library_dir_path);
        baranium_string_list_dispose(&ctx->library_dir_contents);
    }

    size_t len = strlen(str);
    if (str[len-1] == '/' || str[len-1] == '\\')
        len--;

    ctx->library_dir_path = malloc(len+1);
    strncpy(ctx->library_dir_path, str, len);
    ctx->library_dir_path[len] = 0;
    ctx->library_dir_contents = baranium_file_util_get_directory_contents(ctx->library_dir_path, BARANIUM_FILE_UTIL_FILTER_MASK_ALL_FILES);
}

void baranium_compiler_context_add_source(baranium_compiler_context* ctx, FILE* sourcefile, const char* filename)
{
    if (ctx == NULL || sourcefile == NULL)
        return;

    baranium_preprocessor_add_define("__FILE__", stringf("\"%s\"", filename));
    baranium_source_token_list source;
    baranium_source_token_list_init(&source);
    baranium_source_open_from_file(&source, sourcefile);
    baranium_source_token_list_push_list(&ctx->combined_source, &source);
    baranium_source_token_list_dispose(&source);
}

void baranium_compiler_context_clear_sources(baranium_compiler_context* ctx)
{
    if (ctx == NULL)
        return;

    baranium_token_parser_dispose(&ctx->token_parser);
    baranium_source_token_list_dispose(&ctx->combined_source);
    baranium_string_map_dispose(&ctx->nametable);
}

baranium_library* baranium_compiler_context_lookup(baranium_compiler_context* ctx, const char* symbolname)
{
    if (ctx == NULL || symbolname == NULL)
        return NULL;

    for (size_t i = 0; i < ctx->library_count; i++)
    {
        if (baranium_library_has_symbol(ctx->libraries[i], symbolname))
            return ctx->libraries[i];
    }

    return NULL;
}

void baranium_compiler_context_compile(baranium_compiler_context* ctx, const char* output, uint8_t library)
{
    if (ctx == NULL || output == NULL)
        return;

    if (ctx->error_occurred)
        return;

    const char* outputType = library ? "library" : "executable";
    baranium_token_parser_dispose(&ctx->token_parser);
    baranium_token_parser_parse(&ctx->token_parser, &ctx->combined_source);

    baranium_compiler compiler;
    baranium_compiler_init(&compiler);

    FILE* file = fopen(output, "wb+");
    if (file == NULL)
    {
        LOGERROR("Error: cannot create or open file '%s'\n", output);
        return;
    }
    baranium_compiler_write(&compiler, &ctx->token_parser.tokens, file, library);
    baranium_compiler_dispose(&compiler);
    fclose(file);

    if (ctx->error_occurred)
    {
        remove(output);
        LOGERROR("Error while compiling %s '%s'", outputType, output);
        LOGWARNING("Aborting compilation...");
        return;
    }

    LOGINFO("Successfully compiled %s as '%s'", outputType, output);
}

void baranium_compiler_context_add_library(baranium_compiler_context* ctx, const char* name)
{
    if (ctx == NULL)
        return;

    baranium_library* lib = NULL;

    for (size_t i = 0; i < ctx->library_dir_contents.count; i++)
    {
        const char* path = ctx->library_dir_contents.strings[i];
        LOGDEBUG("libdir contents index %d: '%s'", i, path);
        char* filenameptr = (char*)path;
        for (int i = strlen(path)-1; i > 0; i--)
        {
            if (path[i] == '\\' || path[i] == '/')
            {
                filenameptr = (char*)&path[i+1];
                break;
            }
        }
        if (strcmp(filenameptr, name) == 0 || strcmp(stringf("%s.blib", filenameptr), name) == 0)
        {
            LOGDEBUG("loading library from '%s'", stringf("%s/%s", ctx->library_dir_path, path));
            lib = baranium_library_load(stringf("%s/%s", ctx->library_dir_path, path));
            break;
        }
    }

    if (lib == NULL)
    {
        LOGERROR("Could not find library named '%s'", name);
        ctx->error_occurred = 1;
        return;
    }

    if (ctx->library_count + 1 >= ctx->library_buffer_size)
    {
        ctx->library_buffer_size += BARANIUM_COMPILER_CONTEXT_LIBRARY_BUFFER_SIZE;
        ctx->libraries = realloc(ctx->libraries, sizeof(baranium_library*)*ctx->library_buffer_size);
    }

    ctx->libraries[ctx->library_count] = lib;
    ctx->library_count++;
}
