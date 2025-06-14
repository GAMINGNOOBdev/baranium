#include <baranium/string_util.h>
#include <baranium/runtime.h>
#include <baranium/defines.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

///////////////////
///             ///
/// STRING LIST ///
///             ///
///////////////////

baranium_string_list baranium_string_list_init(void)
{
    baranium_string_list list;
    memset(&list, 0, sizeof(baranium_string_list));
    return list;
}

int baranium_string_list_get_index_of_string(baranium_string_list* list, const char* string)
{
    return baranium_string_list_get_index(list, baranium_get_id_of_name(string));
}

int baranium_string_list_get_index(baranium_string_list* list, index_t hash)
{
    if (list == NULL)
        return -1;

    for (size_t i = 0; i < list->count; i++)
        if (list->hashes[i] == hash)
            return i;

    return -1;
}

void baranium_string_list_add(baranium_string_list* list, const char* string)
{
    if (list == NULL || string == NULL)
        return;

    index_t hash = baranium_get_id_of_name(string);

    if (baranium_string_list_get_index(list, hash) != -1)
        return;

    if (list->count + 1 >= list->buffer_size)
    {
        list->buffer_size += BARANIUM_STRING_LIST_BUFFER_SIZE;
        list->strings = realloc(list->strings, sizeof(char*)*list->buffer_size);
        list->hashes = realloc(list->hashes, sizeof(index_t)*list->buffer_size);
    }

    list->strings[list->count] = strsubstr(string,0,-1);
    list->hashes[list->count] = hash;
    list->count++;
}

void baranium_string_list_remove_last(baranium_string_list* list)
{
    if (list == NULL)
        return;

    if (list->count == 0)
        return;

    free((void*)list->strings[list->count-1]);
    list->count--;
}

const char* baranium_string_list_get_string(baranium_string_list* list, int index)
{
    if (list == NULL || index < 0)
        return NULL;

    if ((size_t)index >= list->count)
        return NULL;

    return list->strings[index];
}

const char* baranium_string_list_get_string_from_hash(baranium_string_list* list, index_t hash)
{
    int index = baranium_string_list_get_index(list, hash);
    return baranium_string_list_get_string(list, index);
}

void baranium_string_list_dispose(baranium_string_list* list)
{
    if (list == NULL || list->strings == NULL)
        return;

    for (size_t i = 0; i < list->count; i++)
    {
        if (list->strings[i] == NULL)
            continue;

        free((void*)list->strings[i]);
    }
    free(list->hashes);
    free(list->strings);

    memset(list, 0, sizeof(baranium_string_list));
}

//////////////////////
///                ///
/// STRING HASHMAP ///
///                ///
//////////////////////

baranium_string_map baranium_string_map_init(void)
{
    baranium_string_map map;
    memset(&map, 0, sizeof(baranium_string_map));
    return map;
}

int baranium_string_map_get_index(baranium_string_map* map, const char* name)
{
    return baranium_string_map_get_index_of_hash(map, baranium_get_id_of_name(name));
}

int baranium_string_map_get_index_of_hash(baranium_string_map* map, index_t hash)
{
    if (map == NULL)
        return -1;

    for (size_t i = 0; i < map->count; i++)
        if (map->hashes[i] == hash)
            return i;

    return -1;
}

void baranium_string_map_add(baranium_string_map* map, const char* name, const char* string)
{
    if (map == NULL || string == NULL)
        return;
    
    index_t hash = baranium_get_id_of_name(name);
    if (baranium_string_map_get_index_of_hash(map, hash) != -1)
        return;

    baranium_string_map_add_direct(map, name, strsubstr(string,0,-1));
}

void baranium_string_map_add_direct(baranium_string_map* map, const char* name, const char* string)
{
    if (map == NULL || string == NULL)
        return;

    index_t hash = baranium_get_id_of_name(name);
    if (baranium_string_map_get_index_of_hash(map, hash) != -1)
        return;

    if (map->count + 1 >= map->buffer_size)
    {
        map->buffer_size += BARANIUM_STRING_MAP_BUFFER_SIZE;
        map->strings = realloc(map->strings, sizeof(char*)*map->buffer_size);
        map->hashes = realloc(map->hashes, sizeof(index_t)*map->buffer_size);
    }

    map->strings[map->count] = string;
    map->hashes[map->count] = hash;
    map->count++;
}

void baranium_string_map_remove(baranium_string_map* map, const char* name)
{
    if (map == NULL)
        return;

    if (map->count == 0)
        return;

    int index = baranium_string_map_get_index(map, name);
    if (index < 0 || (size_t)index >= map->count)
        return;

    if ((size_t)index == map->count-1)
    {
        free((void*)map->strings[index]);
        map->hashes[index] = 0;
        map->count--;
        return;
    }

    free((void*)map->strings[index]);
    map->hashes[index] = 0;
    memmove(&map->strings[index], &map->strings[index+1], sizeof(const char*)*(map->count - index - 1));
    memmove(&map->hashes[index], &map->hashes[index+1], sizeof(index_t)*(map->count - index - 1));
    map->count--;
}

const char* baranium_string_map_get(baranium_string_map* map, const char* name)
{
    return baranium_string_map_get_from_hash(map, baranium_get_id_of_name(name));
}

const char* baranium_string_map_get_from_hash(baranium_string_map* map, index_t hash)
{
    int index = baranium_string_map_get_index_of_hash(map, hash);
    if (index == -1)
        return NULL;

    return map->strings[index];
}

void baranium_string_map_dispose(baranium_string_map* map)
{
    if (map == NULL || map->strings == NULL)
        return;

    for (size_t i = 0; i < map->count; i++)
    {
        if (map->strings[i] == NULL)
            continue;

        free((void*)map->strings[i]);
    }
    free(map->hashes);
    free(map->strings);

    memset(map, 0, sizeof(baranium_string_map));
}

//////////////////////////////////////
///                                ///
/// OTHER STRING RELATED FUNCTIONS ///
///                                ///
//////////////////////////////////////

int stridx(const char* string, char delim)
{
    if (string == NULL)
        return -1;

    for (int i = 0; string[i] != 0; i++)
        if(string[i] == delim)
            return i;

    return -1;
}

int stridxnot(const char* string, char delim)
{
    if (string == NULL)
        return -1;

    for (int i = 0; string[i] != 0; i++)
        if(string[i] != delim)
            return i;

    return -1;
}

int stridxlast(const char* string, char delim)
{
    if (string == NULL)
        return -1;

    int string_last_seperator_index = -1;
    for (int i = 0; string[i] != 0; i++)
        string_last_seperator_index = (string[i] == delim) ? i : string_last_seperator_index;

    return string_last_seperator_index;
}

int stridxnotlast(const char* string, char delim)
{
    if (string == NULL)
        return -1;

    int string_last_seperator_index = -1;
    for (int i = 0; string[i] != 0; i++)
        string_last_seperator_index = (string[i] != delim) ? i : string_last_seperator_index;

    return string_last_seperator_index;
}

int strdelimcount(const char* string, char delim)
{
    if (string == NULL)
        return 0;

    int count = 0;
    for (int i = 0; string[i] != 0; i++)
        if (string[i] == delim) count++;

    return count;
}

char* strtrimleading(const char* src)
{
    if (src == NULL)
        return NULL;

    static const char* WHITESPACES = " \n\t\r\f\v";
    uint8_t index = 0;
    for (int i = 0; i < 6; i++)
        index = (stridxnot(src, WHITESPACES[index]) != -1) ? 1 : index;

    if (!index)
    {
        free((void*)src);
        return NULL;
    }

    int16_t begin_index = 0;
    int16_t end_index = 0;

    for (begin_index = 0; begin_index < (int16_t)strlen(src) && (src[begin_index] == '\t' || src[begin_index] == '\n' || src[begin_index] == ' '); begin_index++);
    for (end_index = strlen(src)-1; end_index >= 0 && (src[end_index] == '\t' || src[end_index] == '\n' || src[end_index] == ' '); end_index--);

    if (begin_index == 0 && end_index == (int16_t)strlen(src)-1)
        return (char*)src;

    char* str = strsubstr(src, begin_index, end_index - begin_index + 1);
    free((void*)src);
    return str;
}

uint8_t striscommentbeginning(const char* str)
{
    if (str == NULL)
        return 0;

    if (str[0] == '#')
        return 1;
    
    if (strlen(str) < 2)
        return 0;

    return str[0] == '/' && str[1] == '/';
}

char* strtrimcomment(const char* src)
{
    char* result = malloc(strlen(src)+1);
    memset(result, 0, strlen(src)+1);
    size_t actual_size = 0;
    uint8_t inString = 0;
    char lastStrChar = 0;
    for (size_t i = 0; i < strlen(src); i++)
    {
        char c = src[i];

        if (lastStrChar != 0 && c == lastStrChar)
        {
            inString = 0;
            lastStrChar = 0;
        }

        if (c == '"' && lastStrChar == 0)
        {
            inString = 1;
            lastStrChar = '"';
        }

        if (c == '\'' && lastStrChar == 0)
        {
            inString = 1;
            lastStrChar = '\'';
        }

        if (!inString && striscommentbeginning(&src[i]))
            break;

        result[actual_size] = c;
        actual_size++;
    }
    if (actual_size != strlen(src))
        result = realloc(result, actual_size+1); // remove the unnecessary extra bytes from the string (if there aren't any, nothing will happen)

    return result;
}

char* strsubstr(const char* src, size_t start, size_t length)
{
    if (length == (size_t)-1)
        length = strlen(src)-start;

    char* result = malloc(length+1);
    memset(result, 0, length+1);
    for (size_t i = 0; i < length; i++)
        result[i] = src[start + i];

    return result;
}

uint8_t strisnum(const char* src)
{
    size_t index = 0;
    char c = 0;

    // check for hex/binary formats beforehand
    if (strlen(src) > 2)
    {
        c = src[1];
        if (c == 'x' || c == 'X')
            goto checkHex;

        if (c == 'b' || c == 'B')
            goto checkBinary;
    }

    index = 0;
    c = src[index];
    if (c == '+' || c == '-')
        index++;

    for (; index < strlen(src); index++)
    {
        c = src[index];

        if (c < '0' || c > '9')
            return 0;
    }

    return 1;

checkHex:

    index = 0;
    c = src[index];
    if (c != '0')
        return 0;
    index = 2;

    for (; index < strlen(src); index++)
    {
        c = src[index];

        if ((c < '0' || c > '9') && (c < 'a' || c > 'f') && (c < 'A' || c > 'F'))
            return 0;
    }

    return 1;

checkBinary:

    index = 0;
    c = src[index];
    if (c != '0')
        return 0;
    index = 2;

    for (; index < strlen(src); index++)
    {
        c = src[index];

        if (c < '0' || c > '1')
            return 0;
    }

    return 1;
}

float strgetfloatval(const char* str)
{
    float result = 0;
    size_t index = 0;

    char chr = str[index];
    uint8_t negative = chr == '-';
    if (chr == '+' || chr == '-')
        index++;

    uint8_t hasDot = 0;
    float dotfactor = 0;

    for (; index < strlen(str); index++)
    {
        chr = str[index];

        if (chr == '.')
        {
            hasDot = 1;
            dotfactor = 1;
            continue;
        }

        if (chr < '0' || chr > '9')
            return INFINITY;

        if (hasDot)
        {
            dotfactor *= 10;
            result += (chr - '0') / dotfactor;
            continue;
        }

        result *= 10;
        result += (chr - '0');
    }

    return negative ? -result : result;
}

double strgetdoubleval(const char* str)
{
    double result = 0;
    size_t index = 0;

    char chr = str[index];
    uint8_t negative = chr == '-';
    if (chr == '+' || chr == '-')
        index++;

    uint8_t hasDot = 0;
    double dotfactor = 0;

    for (; index < strlen(str); index++)
    {
        chr = str[index];

        if (chr == '.')
        {
            hasDot = 1;
            dotfactor = 1;
            continue;
        }

        if (chr < '0' || chr > '9')
            return INFINITY;

        if (hasDot)
        {
            dotfactor *= 10;
            result += (chr - '0') / dotfactor;
            continue;
        }

        result *= 10;
        result += (chr - '0');
    }

    return negative ? -result : result;
}

uint64_t strgetnumval(const char* src)
{
    uint8_t negative = 0;
    uint64_t result = 0;
    size_t index = 0;
    char c = 0;

    // check for hex/binary formats beforehand
    if (strlen(src) > 2)
    {
        c = src[1];
        if (c == 'x' || c == 'X')
            goto checkHex;

        if (c == 'b' || c == 'B')
            goto checkBinary;
    }

    index = 0;
    c = src[index];
    if (c == '+' || c == '-')
        index++;

    negative = c == '-';

    for (; index < strlen(src); index++)
    {
        result*=10;
        c = src[index];

        if (c < '0' || c > '9')
            c = '0';

        result+=c-'0';
    }

    return negative ? -result : result;

checkHex:

    index = 0;
    c = src[index];
    if (c != '0')
        return 0;
    index = 2;

    for (; index < strlen(src); index++)
    {
        result <<= 4;

        c = src[index];

        if ((c >= '0' && c <= '9'))
            result |= c-'0';

        if ((c >= 'a' && c < 'f'))
            result |= 10 + c - 'a';

        if ((c >= 'A' && c <= 'F'))
            result |= 10 + c - 'A';
    }

    return result;

checkBinary:

    index = 0;
    c = src[index];
    if (c != '0')
        return 0;
    index = 2;

    for (; index < strlen(src); index++)
    {
        result <<= 1;

        c = src[index];

        if (c == '0')
            continue;

        result |= 1;
    }

    return result;
}

char strgetescseqchr(char c)
{
    switch (c)
    {
    case 'a':
        return '\a';
    case 'b':
        return '\b';
    case 'f':
        return '\f';
    case 'n':
        return '\n';
    case 'r':
        return '\r';
    case 't':
        return '\t';
    case 'v':
        return '\v';
    case '\\':
        return '\\';
    case '\'':
        return '\'';
    case '"':
        return '"';
    case '?':
        return '\?';
    case '0':
        return 0;
    }
    return 0;
}

char* strconescseq(const char* src)
{
    char* result = malloc(strlen(src)+1);
    memset(result, 0, strlen(src)+1);
    size_t actual_size = 0;

    for (size_t i = 0; i < strlen(src); i++)
    {
        char c = src[i];

        if (c == '\\' && i < strlen(src) - 1)
        {
            i++;
            c = src[i];
            c = strgetescseqchr(c);

            result[actual_size] = c;
            actual_size++;
            continue;
        }

        result[actual_size] = c;
        actual_size++;
    }

    if (actual_size != strlen(src))
        result = realloc(result, actual_size+1); // remove the unnecessary extra bytes from the string (if there aren't any, nothing will happen)

    free((void*)src);

    return result;
}

void strsplit(baranium_string_list* _out, const char* src, char delim)
{
    if (_out == NULL || src == NULL)
        return;

    *_out = baranium_string_list_init();

    size_t count = 0;
    char** strings = NULL;
    index_t* hashes = NULL;

    size_t predictedCount = strdelimcount(src, delim)+1;
    _out->buffer_size = predictedCount;
    strings = malloc(sizeof(char*)*predictedCount);
    if (strings == NULL)
        return;

    hashes = malloc(sizeof(index_t)*predictedCount);
    if (hashes == NULL)
    {
        free(strings);
        return;
    }

    size_t end = 0;
    size_t start = 0;
    size_t length = strlen(src);
    for (end = 0; end < length; end++)
    {
        if (src[end] == delim)
        {
            if (end-start < 1)
            {
                start++;
                continue;
            }
            strings[count] = strsubstr(src, start, end - start);
            hashes[count] = baranium_get_id_of_name(strings[count]);
            start = end+1;
            count++;
        }
    }

    if (start != end)
    {
        strings[count] = strsubstr(src, start, length - start);
        hashes[count] = baranium_get_id_of_name(strings[count]);
        count++;
    }

    if (count != predictedCount)
    {
        strings = realloc(strings, sizeof(char*)*count);
        hashes = realloc(hashes, sizeof(index_t)*count);
    }

    _out->count = count;
    _out->buffer_size = count;
    _out->hashes = hashes;
    _out->strings = (const char**)strings;
}
