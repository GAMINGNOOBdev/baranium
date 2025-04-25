#include <baranium/compiler/language/abstract_syntax_tree.h>
#include <baranium/compiler/language/language.h>
#include <baranium/compiler/source_token.h>
#include <baranium/string_util.h>
#include <baranium/logging.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
#   undef min
#   undef max
#endif

#define nop (void)0

baranium_preinpostfix_token_parser_map* baranium_ast_prefix_map = NULL;
baranium_preinpostfix_token_parser_map* baranium_ast_infix_map = NULL;

power_t baranium_abstract_syntax_tree_get_next_precedence(baranium_source_token_list* tokens)
{
    baranium_source_token* token = baranium_source_token_list_peek(tokens);
    if (token == NULL)
        return 0;

    return baranium_preinpostfix_token_parser_map_get(baranium_ast_infix_map, token->type).power;
}

void baranium_abstract_syntax_tree_node_list_init(baranium_abstract_syntax_tree_node_list* list)
{
    if (list == NULL)
        return;

    list->nodes = NULL;
    list->buffer_size = 0;
    list->count = 0;
}

void baranium_abstract_syntax_tree_node_list_add(baranium_abstract_syntax_tree_node_list* list, baranium_abstract_syntax_tree_node* tree_node)
{
    if (list == NULL)
        return;

    if (list->count + 1 >= list->buffer_size)
    {
        list->buffer_size += BARANIUM_TREE_NODE_LIST_BUFFER_SIZE;
        list->nodes = realloc(list->nodes, sizeof(baranium_abstract_syntax_tree_node*)*list->buffer_size);
    }

    list->nodes[list->count] = tree_node;
    list->count++;
}

void baranium_abstract_syntax_tree_node_list_remove_last(baranium_abstract_syntax_tree_node_list* list)
{
    if (list == NULL)
        return;

    if (list->count == 0)
        return;

    list->nodes[list->count-1] = NULL;
    list->count--;
}

void baranium_abstract_syntax_tree_node_list_dispose(baranium_abstract_syntax_tree_node_list* list)
{
    if (list == NULL || list->nodes == NULL)
        return;

    for (size_t i = 0; i < list->count; i++)
    {
        baranium_abstract_syntax_tree_node_dispose(list->nodes[i]);
    }
    free(list->nodes);

    list->count = 0;
    list->buffer_size = 0;
    list->nodes = NULL;
}

void baranium_abstract_syntax_tree_node_init(baranium_abstract_syntax_tree_node* node, baranium_source_token* token, int opidx, uint8_t spchr)
{
    if (node == NULL || token == NULL)
        return;

    node->sub_nodes = (baranium_abstract_syntax_tree_node_list){NULL,0,0};
    node->contents = *token;
    node->operation = opidx;
    node->special_char = spchr;
    node->left = NULL;
    node->right = NULL;

    baranium_abstract_syntax_tree_node_list_init(&node->sub_nodes);
}

int baranium_abstract_syntax_tree_node_valid(baranium_abstract_syntax_tree_node* node)
{
    if (node == NULL)
        return 0;

    return node->contents.type != BARANIUM_SOURCE_TOKEN_TYPE_INVALID || node->operation != -1;
}

void baranium_abstract_syntax_tree_node_dispose(baranium_abstract_syntax_tree_node* node)
{
    if (node == NULL)
        return;

    baranium_abstract_syntax_tree_node_dispose(node->left);
    baranium_abstract_syntax_tree_node_dispose(node->right);

    baranium_abstract_syntax_tree_node_list_dispose(&node->sub_nodes);
    free(node);
}

//////////////////////////////////////////
///                                    ///
/// TOKEN PARSER  PRE-/IN-/POSTFIX MAP ///
///                                    ///
//////////////////////////////////////////

void baranium_preinpostfix_token_parser_map_init(baranium_preinpostfix_token_parser_map* map)
{
    if (map == NULL)
        return;

    map->count = 0;
    map->buffer_size = 0;

    map->data = NULL;
    map->hashes = NULL;
}

int baranium_preinpostfix_token_parser_map_get_index(baranium_preinpostfix_token_parser_map* map, baranium_source_token_type_t type)
{
    if (map == NULL)
        return -1;

    for (size_t i = 0; i < map->count; i++)
        if (map->hashes[i] == type)
            return i;

    return -1;
}

void baranium_preinpostfix_token_parser_map_add(baranium_preinpostfix_token_parser_map* map, baranium_source_token_type_t type, baranium_preinpostfix_token_parser parser)
{
    if (map == NULL || parser.handle == NULL)
        return;

    if (baranium_preinpostfix_token_parser_map_get_index(map, type) != -1)
        return;

    if (map->count + 1 >= map->buffer_size)
    {
        map->buffer_size += BARANIUM_STRING_MAP_BUFFER_SIZE;
        map->data = realloc(map->data, sizeof(baranium_preinpostfix_token_parser)*map->buffer_size);
        map->hashes = realloc(map->hashes, sizeof(baranium_source_token_type_t)*map->buffer_size);
    }

    map->data[map->count] = parser;
    map->hashes[map->count] = type;
    map->count++;
}

void baranium_preinpostfix_token_parser_map_remove(baranium_preinpostfix_token_parser_map* map, baranium_source_token_type_t type)
{
    if (map == NULL)
        return;

    if (map->count == 0)
        return;

    int index = baranium_preinpostfix_token_parser_map_get_index(map, type);
    if (index < 0 || (size_t)index >= map->count)
        return;

    if ((size_t)index == map->count-1)
    {
        map->data[index] = (baranium_preinpostfix_token_parser){
            .power = 0,
            .handle = 0,
        };
        map->hashes[index] = BARANIUM_SOURCE_TOKEN_TYPE_INVALID;
        map->count--;
        return;
    }

    map->data[index] = (baranium_preinpostfix_token_parser){
        .power = 0,
        .handle = 0,
    };
    map->hashes[index] = BARANIUM_SOURCE_TOKEN_TYPE_INVALID;
    memmove(&map->data[index], &map->data[index+1], sizeof(baranium_preinpostfix_token_parser)*(map->count - index - 1));
    memmove(&map->hashes[index], &map->hashes[index+1], sizeof(baranium_source_token_type_t)*(map->count - index - 1));
    map->count--;
}

baranium_preinpostfix_token_parser baranium_preinpostfix_token_parser_map_get(baranium_preinpostfix_token_parser_map* map, baranium_source_token_type_t type)
{
    int index = baranium_preinpostfix_token_parser_map_get_index(map, type);
    if (index == -1)
        return (baranium_preinpostfix_token_parser){ .power=0, .handle=0 };

    return map->data[index];
}

void baranium_preinpostfix_token_parser_map_dispose(baranium_preinpostfix_token_parser_map* map)
{
    if (map == NULL || map->data == NULL)
        return;

    free(map->hashes);
    free(map->data);

    map->count = 0;
    map->buffer_size = 0;
    map->hashes = NULL;
    map->data = NULL;
}

////////////////////////////
///                      ///
/// ABSTRACT SYNTAX TREE ///
///                      ///
////////////////////////////

baranium_abstract_syntax_tree_node* baranium_ast_generic_parser(baranium_source_token_list* tokens, baranium_abstract_syntax_tree_node* parent_node, power_t power)
{
    baranium_abstract_syntax_tree_node* result = (baranium_abstract_syntax_tree_node*)malloc(sizeof(baranium_abstract_syntax_tree_node));
    if (result == NULL)
        return NULL;
    if (parent_node != NULL)
        free(parent_node);
    memset(result, 0, sizeof(baranium_abstract_syntax_tree_node));
    result->contents = *baranium_source_token_list_current(tokens);
    result->operation = (uint8_t)-1;
    return result;
}

baranium_abstract_syntax_tree_node* baranium_ast_string_parser(baranium_source_token_list* tokens, baranium_abstract_syntax_tree_node* parent_node, power_t power)
{
    baranium_abstract_syntax_tree_node* result = (baranium_abstract_syntax_tree_node*)malloc(sizeof(baranium_abstract_syntax_tree_node));
    if (result == NULL)
        return NULL;
    if (parent_node != NULL)
        free(parent_node);
    memset(result, 0, sizeof(baranium_abstract_syntax_tree_node));
    result->contents = *baranium_source_token_list_current(tokens);
    result->operation = (uint8_t)-1;
    baranium_source_token_list_next(tokens);

    result->left = (baranium_abstract_syntax_tree_node*)malloc(sizeof(baranium_abstract_syntax_tree_node));
    if (result->left == NULL)
        return NULL;
    memset(result->left, 0, sizeof(baranium_abstract_syntax_tree_node));
    result->left->operation = (uint8_t)-1;

    result->left->contents = *baranium_source_token_list_current(tokens);
    if (!baranium_source_token_list_next_matches(tokens, BARANIUM_SOURCE_TOKEN_TYPE_DOUBLEQUOTE))
    {
        LOGERROR(stringf("Line %d: missing \" at the end of string", result->left->contents.line_number));
        return NULL;
    }
    return result;
}

baranium_abstract_syntax_tree_node*  baranium_ast_prefix_operator_parser(baranium_source_token_list* tokens, baranium_abstract_syntax_tree_node* parent_node, power_t power)
{
    baranium_abstract_syntax_tree_node* result = (baranium_abstract_syntax_tree_node*)malloc(sizeof(baranium_abstract_syntax_tree_node));
    if (result == NULL)
        return NULL;
    if (parent_node != NULL)
        free(parent_node);
    memset(result, 0, sizeof(baranium_abstract_syntax_tree_node));
    result->contents = *baranium_source_token_list_current(tokens);
    baranium_source_token_type_t type;
    uint8_t was_special_char = 0;
    int64_t operationIndex = baranium_abstract_syntax_tree_get_operation_index(*baranium_source_token_list_current(tokens), &type, &was_special_char);
    result->operation = operationIndex;
    result->special_char = was_special_char;
    result->right = baranium_abstract_syntax_tree_parse_tokens(tokens, BARANIUM_BINDING_POWER_PREFIX);
    return result;
}

baranium_abstract_syntax_tree_node* baranium_ast_indecrement_operator_parser(baranium_source_token_list* tokens, baranium_abstract_syntax_tree_node* parent_node, power_t power)
{
    if (parent_node == NULL)
    {
        LOGERROR("Parent node was null (how)");
        return NULL;
    }

    baranium_abstract_syntax_tree_node* result = (baranium_abstract_syntax_tree_node*)malloc(sizeof(baranium_abstract_syntax_tree_node));
    if (result == NULL)
        return NULL;
    memset(result, 0, sizeof(baranium_abstract_syntax_tree_node));
    result->contents = *baranium_source_token_list_next(tokens);
    baranium_source_token_type_t type;
    uint8_t was_special_char = 0;
    int64_t operationIndex = baranium_abstract_syntax_tree_get_operation_index(result->contents, &type, &was_special_char);
    result->operation = operationIndex;
    result->special_char = was_special_char;
    result->left = (baranium_abstract_syntax_tree_node*)malloc(sizeof(baranium_abstract_syntax_tree_node));
    if (result->left == NULL)
        return NULL;
    memset(result->left, 0, sizeof(baranium_abstract_syntax_tree_node));
    result->left->contents = *baranium_source_token_list_current(tokens);
    operationIndex = baranium_abstract_syntax_tree_get_operation_index(result->contents, &type, &was_special_char);
    result->left->operation = operationIndex;
    result->left->special_char = was_special_char;

    if (result->left->contents.type != BARANIUM_SOURCE_TOKEN_TYPE_TEXT)
    {
        LOGERROR(stringf("Line %d: Invalid assignment, expected variable name, got '%s'", result->left->contents.line_number, result->left->contents.contents));
        return NULL;
    }

    return result;
}

baranium_abstract_syntax_tree_node* baranium_ast_parenthesis_order_parser(baranium_source_token_list* tokens, baranium_abstract_syntax_tree_node* parent_node, power_t power)
{
    baranium_abstract_syntax_tree_node* result = baranium_abstract_syntax_tree_parse_tokens(tokens, BARANIUM_BINDING_POWER_NONE);

    baranium_source_token_list_next(tokens);
    baranium_source_token closing_token = *baranium_source_token_list_current(tokens);
    if (closing_token.type != BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISCLOSE)
    {
        LOGERROR(stringf("Line %d: Expected ')', got '%s'", closing_token.line_number, closing_token.contents));
        return NULL;
    }

    return result;
}

baranium_abstract_syntax_tree_node* baranium_ast_array_indexing_parser(baranium_source_token_list* tokens, baranium_abstract_syntax_tree_node* parent_node, power_t power)
{
    baranium_abstract_syntax_tree_node* result = (baranium_abstract_syntax_tree_node*)malloc(sizeof(baranium_abstract_syntax_tree_node));
    if (result == NULL)
        return NULL;
    memset(result, 0, sizeof(baranium_abstract_syntax_tree_node));
    // copy the parent node, which will probably be the array name, well hopefully
    memcpy(result, parent_node, sizeof(baranium_abstract_syntax_tree_node));
    if (parent_node != NULL)
        free(parent_node);
    baranium_source_token_type_t type;
    uint8_t was_special_char = 0;
    int64_t operation_index = baranium_abstract_syntax_tree_get_operation_index(result->contents, &type, &was_special_char);
    result->operation = operation_index;
    result->special_char = was_special_char;

    if (!baranium_source_token_list_next_matches(tokens, BARANIUM_SOURCE_TOKEN_TYPE_BRACKETCLOSE))
    {
        baranium_abstract_syntax_tree_node_list_init(&result->sub_nodes);
        baranium_abstract_syntax_tree_node_list_add(&result->sub_nodes, baranium_abstract_syntax_tree_parse_tokens(tokens, BARANIUM_BINDING_POWER_NONE));
        if (!baranium_source_token_list_next_matches(tokens, BARANIUM_SOURCE_TOKEN_TYPE_BRACKETCLOSE))
        {
            LOGERROR(stringf("Line %d: Missing ']'", baranium_source_token_list_current(tokens)->line_number));
            return NULL;
        }
    }
    else
    {
        baranium_abstract_syntax_tree_node_list_add(&result->sub_nodes, NULL);
    }

    return result;
}

baranium_abstract_syntax_tree_node* baranium_ast_function_call_parser(baranium_source_token_list* tokens, baranium_abstract_syntax_tree_node* parent_node, power_t power)
{
    baranium_abstract_syntax_tree_node* result = (baranium_abstract_syntax_tree_node*)malloc(sizeof(baranium_abstract_syntax_tree_node));
    if (result == NULL)
        return NULL;
    memset(result, 0, sizeof(baranium_abstract_syntax_tree_node));

    // copy the parent node, which will probably be the function name, well hopefully
    memcpy(result, parent_node, sizeof(baranium_abstract_syntax_tree_node));
    if (parent_node != NULL)
        free(parent_node);
    baranium_source_token_type_t type;
    uint8_t was_special_char = 0;
    int64_t operationIndex = baranium_abstract_syntax_tree_get_operation_index(result->contents, &type, &was_special_char);
    result->operation = operationIndex;
    result->special_char = was_special_char;

    if (!baranium_source_token_list_next_matches(tokens, BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISCLOSE))
    {
        do
        {
            baranium_abstract_syntax_tree_node_list_add(&result->sub_nodes, baranium_abstract_syntax_tree_parse_tokens(tokens, BARANIUM_BINDING_POWER_NONE));
        }
        while (baranium_source_token_list_next_matches(tokens, BARANIUM_SOURCE_TOKEN_TYPE_COMMA));
        if (!baranium_source_token_list_next_matches(tokens, BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISCLOSE))
        {
            LOGERROR(stringf("Line %d: Missing ')'", baranium_source_token_list_current(tokens)->line_number));
            return NULL;
        }
    }
    else
    {
        baranium_abstract_syntax_tree_node_list_add(&result->sub_nodes, NULL);
    }

    return result;
}

baranium_abstract_syntax_tree_node* baranium_ast_comparison_operator_parser(baranium_source_token_list* tokens, baranium_abstract_syntax_tree_node* parent_node, power_t power)
{
    baranium_abstract_syntax_tree_node* result = (baranium_abstract_syntax_tree_node*)malloc(sizeof(baranium_abstract_syntax_tree_node));
    if (result == NULL)
        return NULL;
    memset(result, 0, sizeof(baranium_abstract_syntax_tree_node));
    result->contents = *baranium_source_token_list_current(tokens);
    baranium_source_token_type_t type;
    uint8_t was_special_char = 0;
    int64_t operationIndex = baranium_abstract_syntax_tree_get_operation_index(*baranium_source_token_list_current(tokens), &type, &was_special_char);
    result->operation = operationIndex;
    result->special_char = was_special_char;
    result->right = baranium_abstract_syntax_tree_parse_tokens(tokens, BARANIUM_BINDING_POWER_COMBINEDCOMPARISON);

    result->left = (baranium_abstract_syntax_tree_node*)malloc(sizeof(baranium_abstract_syntax_tree_node));
    if (result->left == NULL)
        return NULL;
    // copy the parent node, which will probably be the function name, well hopefully
    memcpy(result->left, parent_node, sizeof(baranium_abstract_syntax_tree_node));
    if (parent_node != NULL)
        free(parent_node);

    return result;
}

baranium_abstract_syntax_tree_node* baranium_ast_combined_comparison_operator_parser(baranium_source_token_list* tokens, baranium_abstract_syntax_tree_node* parent_node, power_t power)
{
    baranium_abstract_syntax_tree_node* result = (baranium_abstract_syntax_tree_node*)malloc(sizeof(baranium_abstract_syntax_tree_node));
    if (result == NULL)
        return NULL;
    memset(result, 0, sizeof(baranium_abstract_syntax_tree_node));
    result->contents = *baranium_source_token_list_current(tokens);
    LOGDEBUG(stringf("Previous token: '%s'", parent_node->contents.contents));
    LOGDEBUG(stringf("Current token: '%s'", *baranium_source_token_list_current(tokens)->contents));
    LOGDEBUG(stringf("Next token: '%s'", baranium_source_token_list_peek(tokens)->contents));
    baranium_source_token_type_t type;
    uint8_t was_special_char = 0;
    int64_t operationIndex = baranium_abstract_syntax_tree_get_operation_index(*baranium_source_token_list_current(tokens), &type, &was_special_char);
    result->operation = operationIndex;
    result->special_char = was_special_char;
    result->right = baranium_abstract_syntax_tree_parse_tokens(tokens, BARANIUM_BINDING_POWER_NONE);

    result->left = (baranium_abstract_syntax_tree_node*)malloc(sizeof(baranium_abstract_syntax_tree_node));
    if (result->left == NULL)
        return NULL;
    // copy the parent node, which will probably be the function name, well hopefully
    memcpy(result->left, parent_node, sizeof(baranium_abstract_syntax_tree_node));
    if (parent_node != NULL)
        free(parent_node);

    LOGDEBUG(stringf("left token: %s", result->left->contents.contents));
    LOGDEBUG(stringf("right token: %s", result->right->contents.contents));
    return result;
}

baranium_abstract_syntax_tree_node* baranium_ast_infix_operator_parser(baranium_source_token_list* tokens, baranium_abstract_syntax_tree_node* parent_node, power_t power)
{
    baranium_abstract_syntax_tree_node* result = (baranium_abstract_syntax_tree_node*)malloc(sizeof(baranium_abstract_syntax_tree_node));
    if (result == NULL)
        return NULL;
    memset(result, 0, sizeof(baranium_abstract_syntax_tree_node));
    result->contents = *baranium_source_token_list_current(tokens);
    baranium_source_token_type_t type;
    uint8_t was_special_char = 0;
    int64_t operationIndex = baranium_abstract_syntax_tree_get_operation_index(*baranium_source_token_list_current(tokens), &type, &was_special_char);
    result->operation = operationIndex;
    result->special_char = was_special_char;
    result->right = baranium_abstract_syntax_tree_parse_tokens(tokens, power);

    result->left = (baranium_abstract_syntax_tree_node*)malloc(sizeof(baranium_abstract_syntax_tree_node));
    if (result->left == NULL)
        return NULL;
    // copy the parent node, which will probably be the function name, well hopefully
    memcpy(result->left, parent_node, sizeof(baranium_abstract_syntax_tree_node));
    if (parent_node != NULL)
        free(parent_node);

    return result;
}

baranium_abstract_syntax_tree_node* baranium_ast_assignment_operator_parser(baranium_source_token_list* tokens, baranium_abstract_syntax_tree_node* parent_node, power_t power)
{
    if (parent_node == NULL)
    {
        LOGERROR("Parent node was null (how)");
        return NULL;
    }

    if (parent_node->contents.type != BARANIUM_SOURCE_TOKEN_TYPE_TEXT)
    {
        LOGERROR(stringf("Line %d: Invalid assignment, expected variable/field name, got '%s'", parent_node->contents.line_number, parent_node->contents.contents));
        return NULL;
    }

    baranium_abstract_syntax_tree_node* result = (baranium_abstract_syntax_tree_node*)malloc(sizeof(baranium_abstract_syntax_tree_node));
    if (result == NULL)
        return NULL;
    memset(result, 0, sizeof(baranium_abstract_syntax_tree_node));
    result->contents = *baranium_source_token_list_current(tokens);
    baranium_source_token_type_t type;
    uint8_t was_special_char = 0;
    int64_t operationIndex = baranium_abstract_syntax_tree_get_operation_index(*baranium_source_token_list_current(tokens), &type, &was_special_char);
    result->operation = operationIndex;
    result->special_char = was_special_char;
    result->right = baranium_abstract_syntax_tree_parse_tokens(tokens, BARANIUM_BINDING_POWER_NONE);

    result->left = (baranium_abstract_syntax_tree_node*)malloc(sizeof(baranium_abstract_syntax_tree_node));
    if (result->left == NULL)
        return NULL;
    // copy the parent node, which will probably be the function name, well hopefully
    memcpy(result->left, parent_node, sizeof(baranium_abstract_syntax_tree_node));
    if (parent_node != NULL)
        free(parent_node);

    return result;
}

baranium_abstract_syntax_tree_node* baranium_ast_postfix_operator_parser(baranium_source_token_list* tokens, baranium_abstract_syntax_tree_node* parent_node, power_t power)
{
    if (parent_node == NULL)
    {
        LOGERROR("Parent node was null (how)");
        return NULL;
    }

    if (parent_node->contents.type != BARANIUM_SOURCE_TOKEN_TYPE_TEXT)
    {
        LOGERROR(stringf("Line %d: Invalid in-/decrementation, expected variable name, got '%s'", parent_node->contents.line_number, parent_node->contents.contents));
        return NULL;
    }

    baranium_abstract_syntax_tree_node* result = (baranium_abstract_syntax_tree_node*)malloc(sizeof(baranium_abstract_syntax_tree_node));
    if (result == NULL)
        return NULL;
    memset(result, 0, sizeof(baranium_abstract_syntax_tree_node));
    result->contents = *baranium_source_token_list_current(tokens);
    baranium_source_token_type_t type;
    uint8_t was_special_char = 0;
    int64_t operationIndex = baranium_abstract_syntax_tree_get_operation_index(*baranium_source_token_list_current(tokens), &type, &was_special_char);
    result->operation = operationIndex;
    result->special_char = was_special_char;

    result->left = (baranium_abstract_syntax_tree_node*)malloc(sizeof(baranium_abstract_syntax_tree_node));
    if (result->left == NULL)
        return NULL;
    // copy the parent node, which will probably be the function name, well hopefully
    memcpy(result->left, parent_node, sizeof(baranium_abstract_syntax_tree_node));
    if (parent_node != NULL)
        free(parent_node);

    return result;
}

void baranium_abstract_syntax_tree_init(void)
{
    baranium_ast_infix_map = malloc(sizeof(baranium_preinpostfix_token_parser_map));
    baranium_ast_prefix_map = malloc(sizeof(baranium_preinpostfix_token_parser_map));
    baranium_preinpostfix_token_parser_map_init(baranium_ast_infix_map);
    baranium_preinpostfix_token_parser_map_init(baranium_ast_prefix_map);

    ///////////////////////////
    /// Generic item parser ///
    ///////////////////////////

    baranium_abstract_syntax_tree_register_prefix(BARANIUM_SOURCE_TOKEN_TYPE_NULL, baranium_ast_generic_parser);
    baranium_abstract_syntax_tree_register_prefix(BARANIUM_SOURCE_TOKEN_TYPE_TEXT, baranium_ast_generic_parser);
    baranium_abstract_syntax_tree_register_prefix(BARANIUM_SOURCE_TOKEN_TYPE_NUMBER, baranium_ast_generic_parser);
    baranium_abstract_syntax_tree_register_prefix(BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD, baranium_ast_generic_parser);

    /////////////////////
    /// String parser ///
    /////////////////////

    baranium_abstract_syntax_tree_register_prefix(BARANIUM_SOURCE_TOKEN_TYPE_DOUBLEQUOTE, baranium_ast_string_parser);

    ////////////////////////
    /// Prefix operators ///
    ////////////////////////

    baranium_abstract_syntax_tree_register_prefix(BARANIUM_SOURCE_TOKEN_TYPE_PLUS, baranium_ast_prefix_operator_parser);
    baranium_abstract_syntax_tree_register_prefix(BARANIUM_SOURCE_TOKEN_TYPE_MINUS, baranium_ast_prefix_operator_parser);
    baranium_abstract_syntax_tree_register_prefix(BARANIUM_SOURCE_TOKEN_TYPE_TILDE, baranium_ast_prefix_operator_parser);
    baranium_abstract_syntax_tree_register_prefix(BARANIUM_SOURCE_TOKEN_TYPE_EXCLAMATIONPOINT, baranium_ast_prefix_operator_parser);

    /////////////////////////////////
    /// In-/De-crement operations ///
    /////////////////////////////////

    baranium_abstract_syntax_tree_register_prefix(BARANIUM_SOURCE_TOKEN_TYPE_PLUSPLUS, baranium_ast_indecrement_operator_parser);
    baranium_abstract_syntax_tree_register_prefix(BARANIUM_SOURCE_TOKEN_TYPE_MINUSMINUS, baranium_ast_indecrement_operator_parser);

    /////////////////////////////////
    /// Parenthesis order reading ///
    /////////////////////////////////

    baranium_abstract_syntax_tree_register_prefix(BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISOPEN, baranium_ast_parenthesis_order_parser);

    ////////////////////////////////////////
    /// Array indexing operation reading ///
    ////////////////////////////////////////

    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_BRACKETOPEN, BARANIUM_BINDING_POWER_INDEXINGOPERATION, baranium_ast_array_indexing_parser);

    /////////////////////////////
    /// Function call reading ///
    /////////////////////////////

    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_PARENTHESISOPEN, BARANIUM_BINDING_POWER_FUNCTIONCALL, baranium_ast_function_call_parser);

    ////////////////////////////
    /// Comparison operators ///
    ////////////////////////////

    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_EQUALTO, BARANIUM_BINDING_POWER_COMPARISON, baranium_ast_comparison_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_NOTEQUAL, BARANIUM_BINDING_POWER_COMPARISON, baranium_ast_comparison_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_LESSEQUAL, BARANIUM_BINDING_POWER_COMPARISON, baranium_ast_comparison_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_GREATEREQUAL, BARANIUM_BINDING_POWER_COMPARISON, baranium_ast_comparison_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_LESSTHAN, BARANIUM_BINDING_POWER_COMPARISON, baranium_ast_comparison_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_GREATERTHAN, BARANIUM_BINDING_POWER_COMPARISON, baranium_ast_comparison_operator_parser);

    /////////////////////////////////////
    /// Combined comparison operators ///
    /////////////////////////////////////

    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_ANDAND, BARANIUM_BINDING_POWER_COMBINEDCOMPARISON, baranium_ast_combined_comparison_operator_parser);  // basically the same as normal comparisons except that they are higher priority
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_OROR, BARANIUM_BINDING_POWER_COMBINEDCOMPARISON, baranium_ast_combined_comparison_operator_parser);    // basically the same as normal comparisons except that tehy are higher priority

    ///////////////////////
    /// Infix operators ///
    ///////////////////////

    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_PLUS, BARANIUM_BINDING_POWER_PRIMARYOPERATION, baranium_ast_infix_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_MINUS, BARANIUM_BINDING_POWER_PRIMARYOPERATION, baranium_ast_infix_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_ASTERISK, BARANIUM_BINDING_POWER_SECONDARYOPERATION, baranium_ast_infix_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_SLASH, BARANIUM_BINDING_POWER_SECONDARYOPERATION, baranium_ast_infix_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_MODULO, BARANIUM_BINDING_POWER_SECONDARYOPERATION, baranium_ast_infix_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_OR, BARANIUM_BINDING_POWER_BITWISEOPERATION, baranium_ast_infix_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_AND, BARANIUM_BINDING_POWER_BITWISEOPERATION, baranium_ast_infix_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_CARET, BARANIUM_BINDING_POWER_BITWISEOPERATION, baranium_ast_infix_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_BITSHIFTLEFT, BARANIUM_BINDING_POWER_BITWISEOPERATION, baranium_ast_infix_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_BITSHIFTRIGHT, BARANIUM_BINDING_POWER_BITWISEOPERATION, baranium_ast_infix_operator_parser);

    ////////////////////////////
    /// Assignment operation ///
    ////////////////////////////

    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_EQUALSIGN, BARANIUM_BINDING_POWER_ASSIGNMENT, baranium_ast_assignment_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_ANDEQUAL, BARANIUM_BINDING_POWER_ASSIGNMENT, baranium_ast_assignment_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_OREQUAL, BARANIUM_BINDING_POWER_ASSIGNMENT, baranium_ast_assignment_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_XOREQUAL, BARANIUM_BINDING_POWER_ASSIGNMENT, baranium_ast_assignment_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_PLUSEQUAL, BARANIUM_BINDING_POWER_ASSIGNMENT, baranium_ast_assignment_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_MINUSEQUAL, BARANIUM_BINDING_POWER_ASSIGNMENT, baranium_ast_assignment_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_MULEQUAL, BARANIUM_BINDING_POWER_ASSIGNMENT, baranium_ast_assignment_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_DIVEQUAL, BARANIUM_BINDING_POWER_ASSIGNMENT, baranium_ast_assignment_operator_parser);
    baranium_abstract_syntax_tree_register_infix(BARANIUM_SOURCE_TOKEN_TYPE_MODEQUAL, BARANIUM_BINDING_POWER_ASSIGNMENT, baranium_ast_assignment_operator_parser);

    /////////////////////////
    /// Postfix operators ///
    /////////////////////////

    baranium_abstract_syntax_tree_register_postfix(BARANIUM_SOURCE_TOKEN_TYPE_PLUSPLUS, BARANIUM_BINDING_POWER_POSTFIX, baranium_ast_postfix_operator_parser);
    baranium_abstract_syntax_tree_register_postfix(BARANIUM_SOURCE_TOKEN_TYPE_MINUSMINUS, BARANIUM_BINDING_POWER_POSTFIX, baranium_ast_postfix_operator_parser);    

}

void baranium_abstract_syntax_tree_dispose(void)
{
    baranium_preinpostfix_token_parser_map_dispose(baranium_ast_prefix_map);
    baranium_preinpostfix_token_parser_map_dispose(baranium_ast_infix_map);
    free(baranium_ast_prefix_map);
    free(baranium_ast_infix_map);
    baranium_ast_prefix_map = NULL;
    baranium_ast_infix_map = NULL;
}

baranium_abstract_syntax_tree_node* baranium_abstract_syntax_tree_parse(baranium_source_token_list* tokens)
{
    if (tokens == NULL)
        return NULL;

    return baranium_abstract_syntax_tree_parse_tokens(tokens, 0);
}

baranium_abstract_syntax_tree_node* baranium_abstract_syntax_tree_parse_tokens(baranium_source_token_list* tokens, power_t min_power)
{
    if (tokens == NULL)
        return NULL;

    if (baranium_source_token_list_end_of_list(tokens))
    {
        LOGERROR("Invalid expression, quit before expression was finished");
        return NULL;
    }

    baranium_abstract_syntax_tree_node* left = NULL;
    baranium_source_token* token = baranium_source_token_list_next(tokens);

    if(baranium_preinpostfix_token_parser_map_get_index(baranium_ast_prefix_map, token->type) == -1)
    {
        LOGERROR(stringf("Invalid prefix '%s'", token->contents));
        return NULL;
    }

    baranium_preinpostfix_token_parser parser = baranium_preinpostfix_token_parser_map_get(baranium_ast_prefix_map, token->type);
    if (parser.handle != NULL)
        left = parser.handle(tokens, left, parser.power);

    while (min_power < baranium_abstract_syntax_tree_get_next_precedence(tokens))
    {
        token = baranium_source_token_list_next(tokens);
        parser = baranium_preinpostfix_token_parser_map_get(baranium_ast_infix_map, token->type);
    
        if (parser.handle != NULL)
            left = parser.handle(tokens, left, parser.power);
    }

    return left;
}

void baranium_abstract_syntax_tree_register_prefix(baranium_source_token_type_t token_type, baranium_preinpostfixhandle_t handle)
{
    baranium_preinpostfix_token_parser_map_add(baranium_ast_prefix_map, token_type, (baranium_preinpostfix_token_parser){.power = BARANIUM_BINDING_POWER_NONE, .handle = handle});
}

void baranium_abstract_syntax_tree_register_infix(baranium_source_token_type_t token_type, power_t power, baranium_preinpostfixhandle_t handle)
{
    baranium_preinpostfix_token_parser_map_add(baranium_ast_infix_map, token_type, (baranium_preinpostfix_token_parser){.power = power, .handle = handle});
}

void baranium_abstract_syntax_tree_register_postfix(baranium_source_token_type_t token_type, power_t power, baranium_preinpostfixhandle_t handle)
{
    baranium_abstract_syntax_tree_register_infix(token_type, power, handle);
}

int64_t baranium_abstract_syntax_tree_get_operation_index(baranium_source_token token, baranium_source_token_type_t* operation_type, uint8_t* was_special_char)
{
    if (operation_type == NULL || was_special_char == NULL)
        return (int64_t)-1;

    for (uint64_t i = 0; baranium_special_operators[i].name != 0; i++)
    {
        if (token.type == baranium_special_operators[i].type)
        {
            *operation_type = baranium_special_operators[i].type;
            *was_special_char = 0;
            return i;
        }
    }

    for (uint64_t i = 0; baranium_special_operation_characters[i].name != 0; i++)
    {
        if (token.type == baranium_special_operation_characters[i].type)
        {
            *operation_type = baranium_special_operation_characters[i].type;
            *was_special_char = 1;
            return i;
        }
    }

    return (int64_t)-1;
}
