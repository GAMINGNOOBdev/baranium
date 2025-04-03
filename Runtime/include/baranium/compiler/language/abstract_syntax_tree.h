#ifndef __BARANIUM__COMPILER__LANGUAGE__ABSTRACT_SYNTAX_TREE_H_
#define __BARANIUM__COMPILER__LANGUAGE__ABSTRACT_SYNTAX_TREE_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "../source_token.h"
#include <stdint.h>

#define BARANIUM_BINDING_POWER_INVALID              (power_t)-1
#define BARANIUM_BINDING_POWER_NONE                 (power_t)0
#define BARANIUM_BINDING_POWER_ASSIGNMENT           (power_t)1   // variable assignment
#define BARANIUM_BINDING_POWER_COMPARISON           (power_t)2   // comparisons like "==", "!=", "<=" or ">="
#define BARANIUM_BINDING_POWER_COMBINEDCOMPARISON   (power_t)3   // combined comparisons like "&&" and "||"
#define BARANIUM_BINDING_POWER_PRIMARYOPERATION     (power_t)4   // primary as in very simple operations like + or -
#define BARANIUM_BINDING_POWER_SECONDARYOPERATION   (power_t)5   // secondary as in more advanced/complex operations that should be calculated before the primary ones like * or /
#define BARANIUM_BINDING_POWER_BITWISEOPERATION     (power_t)6   // bitwise operations almost always go first, stuff like "|", "^", "&", "~"
#define BARANIUM_BINDING_POWER_PREFIX               (power_t)7   // generic prefix
#define BARANIUM_BINDING_POWER_POSTFIX              (power_t)8   // generic postifx
#define BARANIUM_BINDING_POWER_INDEXINGOPERATION    (power_t)9   // array indexing
#define BARANIUM_BINDING_POWER_FUNCTIONCALL         (power_t)10  // function call

typedef int8_t power_t;

struct baranium_abstract_syntax_tree_node;

#define BARANIUM_TREE_NODE_LIST_BUFFER_SIZE 0x80

typedef struct baranium_abstract_syntax_tree_node_list
{
    struct baranium_abstract_syntax_tree_node** nodes;
    size_t buffer_size;
    size_t count;
} baranium_abstract_syntax_tree_node_list;

/**
 * @brief Initialize a tree node list
*/
BARANIUMAPI void baranium_abstract_syntax_tree_node_list_init(baranium_abstract_syntax_tree_node_list* list);

/**
 * @brief Add an item to the tree node list
*/
BARANIUMAPI void baranium_abstract_syntax_tree_node_list_add(baranium_abstract_syntax_tree_node_list* list, struct baranium_abstract_syntax_tree_node* tree_node);

/**
 * @brief Remove the last item from the tree node list
*/
BARANIUMAPI void baranium_abstract_syntax_tree_node_list_remove_last(baranium_abstract_syntax_tree_node_list* list);

/**
 * @brief Dispose a tree node list
*/
BARANIUMAPI void baranium_abstract_syntax_tree_node_list_dispose(baranium_abstract_syntax_tree_node_list* list);

typedef struct baranium_abstract_syntax_tree_node
{
    /**
     * @brief The contents of this node
    */
    baranium_source_token contents;

    int operation;
    uint8_t special_char;
    struct baranium_abstract_syntax_tree_node* left;
    struct baranium_abstract_syntax_tree_node* right;
    baranium_abstract_syntax_tree_node_list sub_nodes;

} baranium_abstract_syntax_tree_node;

BARANIUMAPI void baranium_abstract_syntax_tree_node_init(baranium_abstract_syntax_tree_node* node, baranium_source_token* token, int opidx, uint8_t spchr);
BARANIUMAPI int baranium_abstract_syntax_tree_node_valid(baranium_abstract_syntax_tree_node* node);
BARANIUMAPI void baranium_abstract_syntax_tree_node_dispose(baranium_abstract_syntax_tree_node* node);

typedef baranium_abstract_syntax_tree_node*(*baranium_preinpostfixhandle_t)(baranium_source_token_list* list, baranium_abstract_syntax_tree_node* node, power_t power);

typedef struct baranium_preinpostfix_token_parser
{
    power_t power;
    baranium_preinpostfixhandle_t handle;
} baranium_preinpostfix_token_parser;

typedef struct baranium_preinpostfix_token_parser_map
{
    baranium_preinpostfix_token_parser* data;
    baranium_source_token_type_t* hashes;
    size_t buffer_size;
    size_t count;
} baranium_preinpostfix_token_parser_map;

/**
 * @brief Initialize a preinpostfix_token_parser map
*/
BARANIUMAPI void baranium_preinpostfix_token_parser_map_init(baranium_preinpostfix_token_parser_map* map);

/**
 * @brief Get the index of a preinpostfix_token_parser inside the map
*/
BARANIUMAPI int baranium_preinpostfix_token_parser_map_get_index(baranium_preinpostfix_token_parser_map* map, baranium_source_token_type_t type);

/**
 * @brief Add an item to the preinpostfix_token_parser map
*/
BARANIUMAPI void baranium_preinpostfix_token_parser_map_add(baranium_preinpostfix_token_parser_map* map, baranium_source_token_type_t type, baranium_preinpostfix_token_parser parser);

/**
 * @brief Remove a given item from the preinpostfix_token_parser map
*/
BARANIUMAPI void baranium_preinpostfix_token_parser_map_remove(baranium_preinpostfix_token_parser_map* map, baranium_source_token_type_t type);

/**
 * @brief Get a preinpostfix_token_parser from the map
*/
BARANIUMAPI baranium_preinpostfix_token_parser baranium_preinpostfix_token_parser_map_get(baranium_preinpostfix_token_parser_map* map, baranium_source_token_type_t type);

/**
 * @brief Dispose a preinpostfix_token_parser map
*/
BARANIUMAPI void baranium_preinpostfix_token_parser_map_dispose(baranium_preinpostfix_token_parser_map* map);

/**
* @brief Initialize an abstract syntax tree
*
* @param tree The actual tree object itself
*/
BARANIUMAPI void baranium_abstract_syntax_tree_init(void);

/**
* @brief Dispose an abstract syntax tree
*
* @param tree The actual tree object itself
*/
BARANIUMAPI void baranium_abstract_syntax_tree_dispose(void);

/**
* @brief Parse tokens into this tree
*
* @param tree The actual tree object itself
* @param tokens The tokens to parse
* @returns The root tree node
*/
BARANIUMAPI baranium_abstract_syntax_tree_node* baranium_abstract_syntax_tree_parse(baranium_source_token_list* tokens);

/**
* @brief Parse tokens
*
* @param tree The actual tree object itself
* @param tokens The tokens that will be parsed
* @param min_power Minimum binding power
* @returns A new tree node
*/
BARANIUMAPI baranium_abstract_syntax_tree_node* baranium_abstract_syntax_tree_parse_tokens(baranium_source_token_list* tokens, power_t min_power);

/**
* @brief Register a specific token type as a prefix
*
* @param tree The actual tree object itself
* @param token_type Specific token type
* @param handle Token parser function pointer
*/
BARANIUMAPI void baranium_abstract_syntax_tree_register_prefix(baranium_source_token_type_t token_type, baranium_preinpostfixhandle_t handle);

/**
* @brief Register a specific token type as an infix
*
* @param tree The actual tree object itself
* @param token_type Specific token type
* @param power The binding power of the token
* @param handle Token parser function pointer
*/
BARANIUMAPI void baranium_abstract_syntax_tree_register_infix(baranium_source_token_type_t token_type, power_t power, baranium_preinpostfixhandle_t handle);

/**
* @brief Register a specific token type as a postfix
*
* @param tree The actual tree object itself
* @param token_type Specific token type
* @param power The binding power of the token
* @param handle Token parser function pointer
*/
BARANIUMAPI void baranium_abstract_syntax_tree_register_postfix(baranium_source_token_type_t token_type, power_t power, baranium_preinpostfixhandle_t handle);

/**
* @brief Get the operation index of a token
*
* @param token The token that will be analyzed
* @param operation_type Output of the operation type
* @param was_special_char Output of the special type
* @returns The operation index
*/
BARANIUMAPI int64_t baranium_abstract_syntax_tree_get_operation_index(baranium_source_token token, baranium_source_token_type_t* operation_type, uint8_t* was_special_char);

#ifdef __cplusplus
}
#endif

#endif
