#include "baranium/compiler/compiler_context.h"
#include "baranium/compiler/preprocessor.h"
#include <baranium/compiler/language/language.h>
#include <baranium/compiler/source_token.h>
#include <baranium/compiler/source.h>
#include <baranium/string_util.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

void baranium_source_read_line(baranium_source_token_list* _out, const char* line, int lineNumber);
void baranium_source_read_buffer(baranium_source_token_list* _out, const char* buffer, int lineNumber, uint8_t isString);
void baranium_source_read_letter(baranium_source_token_list* _out, char chr, int lineNumber);

size_t baranium_getdelim(char **buffer, size_t *buffersz, FILE *stream, char delim)
{
    if (buffer == NULL || stream == NULL || buffersz == NULL)
        return -1;

    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

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
        if ((size_t)(p - bufptr) > (size - 1))
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

size_t baranium_getline(char **buffer, size_t *buffersz, FILE *stream)
{
    return baranium_getdelim(buffer, buffersz, stream, '\n');
}

void baranium_source_open_from_file(baranium_source_token_list* _out, FILE* file)
{
    char* currentLine = NULL;
    char* line = NULL;
    size_t linesize = 0;
    int lineNumber = 0;
    int status = 0;
    while (status != EOF)
    {
        status = baranium_getline(&currentLine, &linesize, file);
        if (status == EOF)
            break;
        currentLine[linesize-1] = 0;
        linesize--;

        lineNumber++;
        if (linesize < 1)
            continue;

        line = strtrimleading( strtrimcomment(currentLine) );
        if (line == NULL)
            continue;

        if (strlen(line) < 1)
        {
            free(line);
            continue;
        }

        if (line[0] == '+')
        {
            baranium_preprocessor_parse(&line[1], _out);
            free(line);
            continue;
        }

        baranium_source_read_line(_out, line, lineNumber);

        free(line);
    }
    free(currentLine);
}

void baranium_source_append_source(baranium_source_token_list* tokens, baranium_source_token_list* other)
{
    baranium_source_token_list_push_list(tokens, other);
}

void baranium_source_parse_single_line(baranium_source_token_list* _out, const char* line)
{
    baranium_source_read_line(_out, line, -1);
}

void baranium_source_read_line(baranium_source_token_list* _out, const char* line, int lineNumber)
{
    size_t start = 0;
    size_t end = 0;
    char tmpStr[] = {0, 0};
    size_t lineLength = strlen(line);
    uint8_t inString = 0;
    char lastStringChar = 0;
    char chr = tmpStr[0];
    baranium_source_token_list line_tokens;
    baranium_source_token_list_init(&line_tokens);

    if (lineLength < 1)
        return;

    if (lineLength < 2)
    {
        baranium_source_read_letter(&line_tokens, line[0], lineNumber);
        goto validate;
    }

    for (size_t index = 0; index < lineLength; index++)
    {
        chr = line[index];
        tmpStr[0] = chr;

        if (chr == '\\' && inString && index < lineLength-1)
        {
            index++;
            end = index;
            continue;
        }

        if (isspace(chr) && !inString)
        {
            end++;
            baranium_source_read_buffer(&line_tokens, strsubstr(line, start, end - start), lineNumber, inString);
            start = index+1;
            continue;
        }

        if (chr == lastStringChar && inString)
            inString = 0;

        if (!inString)
        {
            int specialCharIndex = baranium_is_special_char(chr);
            if (specialCharIndex != -1)
            {
                if (chr == '"' && lastStringChar == 0)
                {
                    inString = 1;
                    lastStringChar = chr;
                }

                end++;
                char* buffer = strsubstr(line, start, end - start);
                if (strncmp(buffer, tmpStr, 2) != 0)
                    baranium_source_read_buffer(&line_tokens, buffer, lineNumber, (!inString && chr == '"' && lastStringChar != 0));
                start = index + 1;

                if (!inString && chr == '"' && lastStringChar != 0)
                    lastStringChar = 0;

                int specialOperatorIndex = -1;
                if (line_tokens.count > 0)
                {
                    baranium_source_token lastToken = line_tokens.data[line_tokens.count-1];
                    specialOperatorIndex = baranium_is_special_operator(lastToken.contents[0], chr);
                    if (baranium_is_special_char(lastToken.contents[0]) != -1 && specialOperatorIndex != -1)
                    {
                        baranium_source_token_list_pop_token(&line_tokens);
                        baranium_source_token token = {
                            .contents=(char*)baranium_special_operators[specialOperatorIndex].name,
                            .type=baranium_special_operators[specialOperatorIndex].type,
                            .special_index=specialOperatorIndex,
                            .line_number=lineNumber,
                        };
                        baranium_source_token_list_add(&line_tokens, &token);
                    }
                }
                if (specialOperatorIndex == -1)
                {
                    baranium_source_token token = {
                        .contents=(char*)baranium_special_characters[specialCharIndex].name,
                        .type=baranium_special_characters[specialCharIndex].type,
                        .special_index=specialCharIndex,
                        .line_number=lineNumber,
                    };
                    baranium_source_token_list_add(&line_tokens, &token);
                }
            }
        }

        end = index;
    }

    if (start == 0 && end == lineLength-1)
        baranium_source_read_buffer(&line_tokens, strsubstr(line,0,-1), lineNumber, inString); // the line has to be copied in order to avoid double-free of the same address
    if (start != 0 && end == lineLength-1)
        baranium_source_read_buffer(&line_tokens, strsubstr(line,start,end - start + 1), lineNumber, inString); // the line has to be copied in order to avoid double-free of the same address

validate:
    baranium_preprocessor_assist_in_line(&line_tokens);
    baranium_source_token_list_push_list(_out, &line_tokens);
    baranium_source_token_list_dispose(&line_tokens);
    return;
}

void baranium_source_read_buffer(baranium_source_token_list* _out, const char* buffer, int lineNumber, uint8_t isString)
{
    if (!isString)
        buffer = strtrimleading(buffer);

    if (buffer == NULL)
        return;
    if(strlen(buffer) < 1)
        return;

    int keyword_index = baranium_is_keyword(buffer);
    baranium_source_token token = {
        .contents = (char*)buffer,
        .special_index = keyword_index,
        .type = BARANIUM_SOURCE_TOKEN_TYPE_TEXT,
        .line_number = lineNumber,
    };

    if (keyword_index != -1)
    {
        free((void*)buffer);
        token.contents = (char*)baranium_keywords[keyword_index].name;
        token.type = baranium_keywords[keyword_index].type;
    }
    else if (strisnum(buffer))
        token.type = BARANIUM_SOURCE_TOKEN_TYPE_NUMBER;
    else
        buffer = token.contents = strconescseq(token.contents);

    baranium_compiler_context* context = baranium_get_compiler_context();
    if (context && keyword_index == -1)
    {
        if (baranium_string_map_get_index(&context->nametable, buffer) != -1)
        {
            token.contents = (char*)baranium_string_map_get(&context->nametable, buffer);
            free((void*)buffer);
        }
        else
            baranium_string_map_add_direct(&context->nametable, buffer, buffer);
    }

    baranium_source_token_list_add(_out, &token);
}

void baranium_source_read_letter(baranium_source_token_list* _out, char chr, int lineNumber)
{
    baranium_compiler_context* context = baranium_get_compiler_context();
    int idx = baranium_is_special_char(chr);
    baranium_source_token token;
    token.special_index = -1;
    token.line_number = lineNumber;
    token.type = BARANIUM_SOURCE_TOKEN_TYPE_TEXT;

    if (isdigit(chr))
    {
        token.type = BARANIUM_SOURCE_TOKEN_TYPE_NUMBER;
        char* tmp = malloc(2);
        tmp[0] = chr;
        tmp[1] = 0;
        token.contents = tmp;
        goto end;
    }

    if (idx != -1)
    {
        token.type = baranium_special_characters[idx].type;
        token.contents = (char*)baranium_special_characters[idx].name;
    }
    else
    {
        char* tmp = malloc(2);
        tmp[0] = chr;
        tmp[1] = 0;
        token.contents = tmp;
    }
    token.special_index = idx;

end:

    if (context && idx == -1)
    {
        if (baranium_string_map_get_index(&context->nametable, token.contents) != -1)
        {
            void* oldcontents = token.contents;
            token.contents = (char*)baranium_string_map_get(&context->nametable, token.contents);
            free(oldcontents);
        }
        else
            baranium_string_map_add_direct(&context->nametable, token.contents, token.contents);
    }

    baranium_source_token_list_add(_out, &token);
}
