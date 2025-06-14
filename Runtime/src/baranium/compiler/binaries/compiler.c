#include <baranium/compiler/language/expression_token.h>
#include <baranium/compiler/language/function_token.h>
#include <baranium/compiler/binaries/symbol_table.h>
#include <baranium/compiler/language/variable_token.h>
#include <baranium/compiler/language/if_else_token.h>
#include <baranium/compiler/language/field_token.h>
#include <baranium/compiler/language/loop_token.h>
#include <baranium/compiler/binaries/compiler.h>
#include <baranium/compiler/language/language.h>
#include <baranium/compiler/compiler_context.h>
#include <baranium/compiler/language/token.h>
#include <baranium/compiler/source_token.h>
#include <baranium/string_util.h>
#include <baranium/variable.h>
#include <baranium/library.h>
#include <baranium/version.h>
#include <baranium/logging.h>
#include <baranium/runtime.h>
#include <baranium/script.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>

//////////////////////////
///                    ///
///   PREDEFINITIONS   ///
///                    ///
//////////////////////////

/**
 * @brief Compile a list of tokens (does not write to a file!)
 * 
 * @param tokens The code tokens
 */
void baranium_compiler_compile(baranium_compiler* compiler, baranium_token_list* tokens);

/**
 * @brief Initialize the `baranium_compiler_code_builder` object
 */
void baranium_compiler_code_builder_init(baranium_compiler* compiler);

/**
 * @brief Dispose the `baranium_compiler_code_builder` object
 */
void baranium_compiler_code_builder_dispose(baranium_compiler* compiler);

/**
 * @brief Clear the currently held code
 */
void baranium_compiler_code_builder_clear(baranium_compiler* compiler);

/**
 * @brief Check if the last instruction is a return statement
 */
uint8_t baranium_compiler_code_builder_returned_from_execution(baranium_compiler* compiler);

// push a string value to the stack (needed for assignments of strings)
void baranium_compiler_code_builder_push_string(baranium_compiler* compiler, const char* str);

// push a bool value to the stack
void baranium_compiler_code_builder_push_bool(baranium_compiler* compiler, uint8_t b);

// push a uint value to the stack
void baranium_compiler_code_builder_push_uint(baranium_compiler* compiler, uint32_t val);

// push a int value to the stack
void baranium_compiler_code_builder_push_int(baranium_compiler* compiler, int32_t val);

// push a float value to the stack
void baranium_compiler_code_builder_push_float(baranium_compiler* compiler, float val);

void baranium_compiler_code_builder_push64(baranium_compiler* compiler, uint64_t data);
void baranium_compiler_code_builder_push32(baranium_compiler* compiler, uint32_t data);
void baranium_compiler_code_builder_push16(baranium_compiler* compiler, uint16_t data);
void baranium_compiler_code_builder_push(baranium_compiler* compiler, uint8_t data);

//////////////////////
///                ///
///   OPERATIONS   ///
///                ///
//////////////////////

// no operation
void baranium_compiler_code_builder_NOP(baranium_compiler* compiler);

// clear compare flag
void baranium_compiler_code_builder_CCF(baranium_compiler* compiler);

// set compare flag
void baranium_compiler_code_builder_SCF(baranium_compiler* compiler);

// clear compare value
void baranium_compiler_code_builder_CCV(baranium_compiler* compiler);

// invert compare value
void baranium_compiler_code_builder_ICV(baranium_compiler* compiler);

// push the compare value to the stack
void baranium_compiler_code_builder_PUSHCV(baranium_compiler* compiler);

// pop the compare value from the stack
void baranium_compiler_code_builder_POPCV(baranium_compiler* compiler);

// push a variable's value to the stack (can't be used on string variables fyi)
void baranium_compiler_code_builder_PUSHVAR(baranium_compiler* compiler, index_t id);

// pop a value from the stack into a variable
void baranium_compiler_code_builder_POPVAR(baranium_compiler* compiler, index_t id);

// push a value to the stack
void baranium_compiler_code_builder_PUSH(baranium_compiler* compiler, uint64_t val);

// call a function with a specific id
void baranium_compiler_code_builder_CALL(baranium_compiler* compiler, index_t id);

// return from a function
void baranium_compiler_code_builder_RET(baranium_compiler* compiler);

// jump to
void baranium_compiler_code_builder_JMP(baranium_compiler* compiler, uint64_t addr);

// jump offset-ed from the current position to
void baranium_compiler_code_builder_JMPOFF(baranium_compiler* compiler, int16_t offset);

// jump if equal to
void baranium_compiler_code_builder_JMPC(baranium_compiler* compiler, uint64_t addr);

// jump offset-ed from the current position
void baranium_compiler_code_builder_JMPCOFF(baranium_compiler* compiler, int16_t addr);

// modulo two values from the stack
void baranium_compiler_code_builder_MOD(baranium_compiler* compiler);

// divide two values from the stack
void baranium_compiler_code_builder_DIV(baranium_compiler* compiler);

// multiply two values from the stack
void baranium_compiler_code_builder_MUL(baranium_compiler* compiler);

// subtrack two values from the stack
void baranium_compiler_code_builder_SUB(baranium_compiler* compiler);

// add two values from the stack
void baranium_compiler_code_builder_ADD(baranium_compiler* compiler);

// bitwise and two values from the stack
void baranium_compiler_code_builder_AND(baranium_compiler* compiler);

// bitwise or two values from the stack
void baranium_compiler_code_builder_OR(baranium_compiler* compiler);

// bitwise exclusive-or two values from the stack
void baranium_compiler_code_builder_XOR(baranium_compiler* compiler);

// bitwise shift left
void baranium_compiler_code_builder_SHFTL(baranium_compiler* compiler);

// bitwise shift right
void baranium_compiler_code_builder_SHFTR(baranium_compiler* compiler);

// compare two values on the stack
void baranium_compiler_code_builder_CMP(baranium_compiler* compiler, uint8_t compareMethod);

// compare two compared values together
void baranium_compiler_code_builder_CMPC(baranium_compiler* compiler, uint8_t compareCombineMethod);

// allocate memory
void baranium_compiler_code_builder_MEM(baranium_compiler* compiler, size_t size, uint8_t type, index_t id);

// deallocate memory
void baranium_compiler_code_builder_FEM(baranium_compiler* compiler, index_t id);

// set var data
void baranium_compiler_code_builder_SET(baranium_compiler* compiler, index_t id, size_t size, baranium_value_t data, baranium_variable_type_t type);

// instantiate an object
void baranium_compiler_code_builder_INSTANTIATE(baranium_compiler* compiler);

// delete an object
void baranium_compiler_code_builder_DELETE(baranium_compiler* compiler);

// attach to an object
void baranium_compiler_code_builder_ATTACH(baranium_compiler* compiler);

// detach from an object
void baranium_compiler_code_builder_DETACH(baranium_compiler* compiler);

// stop execution with return code
void baranium_compiler_code_builder_KILL(baranium_compiler* compiler, int64_t code);
void baranium_compiler_compile_variable(baranium_compiler* compiler, baranium_variable_token* token);
void baranium_compiler_compile_expression(baranium_compiler* compiler, baranium_expression_token* token);
void baranium_compiler_compile_if_else_statement(baranium_compiler* compiler, baranium_if_else_token* token);
void baranium_compiler_compile_if_else_sub_statement(baranium_compiler* compiler, baranium_if_else_token* token);
void baranium_compiler_compile_else_statement(baranium_compiler* compiler, baranium_if_else_token* token);
void baranium_compiler_compile_do_while_loop(baranium_compiler* compiler, baranium_loop_token* token);
void baranium_compiler_compile_while_loop(baranium_compiler* compiler, baranium_loop_token* token);
void baranium_compiler_compile_for_loop(baranium_compiler* compiler, baranium_loop_token* token);
void baranium_compiler_compile_ast_node(baranium_compiler* compiler, baranium_abstract_syntax_tree_node* node, uint8_t isRoot);
void baranium_compiler_compile_assignment(baranium_compiler* compiler, baranium_abstract_syntax_tree_node* root);
void baranium_compiler_compile_return_statement(baranium_compiler* compiler, baranium_expression_token* expression);
void baranium_compiler_compile_arithmetic_operation(baranium_compiler* compiler, baranium_abstract_syntax_tree_node* root, uint8_t isRoot);
void baranium_compiler_compile_condition(baranium_compiler* compiler, baranium_abstract_syntax_tree_node* root);
void baranium_compiler_compile_keyword_expression(baranium_compiler* compiler, baranium_expression_token* expression);
void baranium_compiler_compile_function_call(baranium_compiler* compiler, baranium_abstract_syntax_tree_node* node);
static uint8_t baranium_compiler_get_compare_method(baranium_source_token_type_t type);

baranium_compiler baranium_compiler_predict_code_size(baranium_compiler* compiler, baranium_token_list* tokens);
baranium_compiler baranium_compiler_predict_code_size_expression(baranium_compiler* compiler, baranium_expression_token* token);

index_t baranium_compiler_get_id(baranium_compiler* compiler, const char* name, int lineNumber);


/////////////////////////////////
///                           ///
///   FUNCTION DECLARATIONS   ///
///                           ///
/////////////////////////////////


void baranium_compiler_init(baranium_compiler* compiler)
{
    if (compiler == NULL)
        return;

    memset(compiler, 0, sizeof(baranium_compiler));
    compiler->loop_begin_addr = -1;
    baranium_symbol_table_init(&compiler->var_table);
}

void baranium_compiler_dispose(baranium_compiler* compiler)
{
    if (compiler == NULL)
        return;

    if (compiler->code)
        free(compiler->code);

    if (compiler->sections)
        free(compiler->sections);

    if (compiler->dependencies)
        free(compiler->dependencies);

    baranium_symbol_table_dispose(&compiler->var_table);

    memset(compiler, 0, sizeof(baranium_compiler));
}

void baranium_compiler_copy_variable_data(void* __dst, const char* __src, baranium_variable_type_t varType);
void baranium_compiler_compile_variables(baranium_compiler* compiler, baranium_token_list* variables);
void baranium_compiler_clear_variables(baranium_compiler* compiler, baranium_token_list* variables);

void baranium_compiler_add_dependency(baranium_compiler* compiler, baranium_library* dependency)
{
    if (compiler == NULL || dependency == NULL)
        return;

    const char* libid = dependency->name;

    if (compiler->dependencies == NULL)
    {
        compiler->dependency_count++;
        compiler->dependencies = malloc(sizeof(const char*)*compiler->dependency_count);
        compiler->dependencies[0] = libid;
        return;
    }

    for (size_t i = 0; i < compiler->dependency_count; i++)
        if (compiler->dependencies[i] == libid)
            return;
    
    compiler->dependency_count++;
    compiler->dependencies = realloc(compiler->dependencies, sizeof(const char*)*compiler->dependency_count);
    compiler->dependencies[compiler->dependency_count-1] = libid;
}

void baranium_compiler_write(baranium_compiler* compiler, baranium_token_list* tokens, FILE* file, uint8_t library)
{
    if (compiler == NULL || tokens == NULL || file == NULL || tokens->count == 0)
        return;

    size_t tmp = 0;
    if (!library)
    {
        baranium_script_header header = {
            .magic = { BARANIUM_MAGIC_NUM0, BARANIUM_MAGIC_NUM1, BARANIUM_MAGIC_NUM2, BARANIUM_MAGIC_NUM3 },
            .version = BARANIUM_VERSION_CURRENT,
            .section_count = tokens->count,
        };
        fwrite(&header, sizeof(baranium_script_header), 1, file);

        size_t tokencountposition = offsetof(struct baranium_script_header, section_count);
        size_t uselesssectioncount = 0;
        for (size_t i = 0; i < tokens->count; i++)
        {
            baranium_token* token = tokens->data[i];
            if (token->type == BARANIUM_TOKEN_TYPE_FUNCTION)
            {
                baranium_function_token* function = (baranium_function_token*)token;
                baranium_symbol_table_add_from_name_and_id(&compiler->var_table, token->name, token->id);
                if (function->only_declaration)
                {
                    uselesssectioncount++;
                    continue;
                }

                tmp = (uint8_t)BARANIUM_SCRIPT_SECTION_TYPE_FUNCTIONS; fwrite(&tmp, sizeof(uint8_t), 1, file);
                fwrite(&token->id, sizeof(index_t), 1, file);

                // "compile" the code
                baranium_compiler_code_builder_clear(compiler);
                baranium_compiler_compile_variables(compiler, &function->parameters);
                baranium_compiler_compile(compiler, &function->tokens);
                baranium_compiler_clear_variables(compiler, &function->parameters);

                // Size calculation: parameter count + return type + compiled code size
                uint64_t dataSize = 2 + compiler->code_length;
                fwrite(&dataSize, sizeof(uint64_t), 1, file);

                tmp = (uint8_t)function->parameters.count; fwrite(&tmp, sizeof(uint8_t), 1, file);
                tmp = (uint8_t)function->return_type; fwrite(&tmp, sizeof(uint8_t), 1, file);
                fwrite(compiler->code, sizeof(uint8_t), compiler->code_length, file);

                baranium_compiler_code_builder_clear(compiler);
                continue;
            }

            if (token->type == BARANIUM_TOKEN_TYPE_VARIABLE)
            {
                baranium_variable_token* variable = (baranium_variable_token*)token;
                tmp = (uint8_t)BARANIUM_SCRIPT_SECTION_TYPE_FIELDS; fwrite(&tmp, sizeof(uint8_t), 1, file);
                fwrite(&token->id, sizeof(index_t), 1, file);
                int8_t dataTypeSize = baranium_variable_get_size_of_type(variable->type);
                if (dataTypeSize == -1) // meaning this is a string
                    dataTypeSize = strlen(variable->value) + 1; // for now store the initial string's length + 1 because of the null-char at the end

                // Size calculation: data type(1 byte) + data size
                uint64_t dataSize = sizeof(uint8_t) + dataTypeSize;
                fwrite(&dataSize, sizeof(uint64_t), 1, file);
                void* data = (uint8_t*)malloc(dataSize);
                memset(data, 0, dataSize);
                baranium_compiler_copy_variable_data(data, variable->value, variable->type);
                fwrite(data, sizeof(uint8_t), dataSize, file);
                free(data);

                baranium_symbol_table_add_from_name_and_id(&compiler->var_table, token->name, token->id);
                continue;
            }

            if (token->type == BARANIUM_TOKEN_TYPE_FIELD)
            {
                baranium_field_token* field = (baranium_field_token*)token;
                tmp = (uint8_t)BARANIUM_SCRIPT_SECTION_TYPE_FIELDS; fwrite(&tmp, sizeof(uint8_t), 1, file);
                fwrite(&token->id, sizeof(index_t), 1, file);
                int8_t dataTypeSize = baranium_variable_get_size_of_type(field->type);
                if (dataTypeSize == -1) // meaning this is a string
                    dataTypeSize = strlen(field->value) + 1; // for now store the initial string's length + 1 because of the null-char at the end

                // Size calculation: data type(1 byte) + data size
                uint64_t dataSize = sizeof(uint8_t) + dataTypeSize;
                fwrite(&dataSize, sizeof(uint64_t), 1, file);
                void* data = (uint8_t*)malloc(dataSize);
                memset(data, 0, dataSize);
                baranium_compiler_copy_variable_data(data, field->value, field->type);
                fwrite(data, sizeof(uint8_t), dataSize, file);
                free(data);

                baranium_symbol_table_add_from_name_and_id(&compiler->var_table, token->name, token->id);
                continue;
            }
        }

        // update the old section count
        size_t end_pos = ftell(file);
        fseek(file, tokencountposition, SEEK_SET);
        tmp = tokens->count - uselesssectioncount;
        fwrite(&tmp, sizeof(uint64_t), 1, file);
        fseek(file, end_pos, SEEK_SET);
    }
    else
    {
        baranium_library_header header = {
            .magic = {BARANIUM_LIBRARY_MAGIC_NUM0,BARANIUM_LIBRARY_MAGIC_NUM1,BARANIUM_LIBRARY_MAGIC_NUM2,BARANIUM_LIBRARY_MAGIC_NUM3},
            .version = BARANIUM_VERSION_CURRENT,
            .exports_count = 0,
            .section_count = tokens->count,
        };
        size_t exportcountposition = offsetof(struct baranium_library_header, exports_count);
        size_t tokencountposition = offsetof(struct baranium_library_header, section_count);
        fwrite(&header, sizeof(baranium_library_header), 1, file);

        size_t externalsymbolnameappendixlength = 7;
        const char* externalsymbolnameappendix = "_extern";
        for (size_t i = 0; i < tokens->count; i++)
        {
            baranium_token* token = tokens->data[i];
            baranium_library_export export = {
                .type = -1,
                .id = token->id,
                .num_params = -1,
                .return_type = -1,
                .symnamelen = 0,
                .symname = NULL,
            };

            if (token->type == BARANIUM_TOKEN_TYPE_VARIABLE)
                continue;

            if (token->type == BARANIUM_TOKEN_TYPE_FUNCTION)
            {
                baranium_function_token* function = (baranium_function_token*)token;
                export.type = BARANIUM_SCRIPT_SECTION_TYPE_FUNCTIONS;
                export.return_type = function->return_type;
                export.num_params = function->parameters.count;
                if (function->only_declaration)
                {
                    export.symnamelen = strlen(token->name) + externalsymbolnameappendixlength;
                    export.symname = token->name;
                }
            }

            if (token->type == BARANIUM_TOKEN_TYPE_FIELD)
                export.type = BARANIUM_SCRIPT_SECTION_TYPE_FIELDS;

            fwrite(&export.type, sizeof(baranium_script_section_type_t), 1, file);
            fwrite(&export.id, sizeof(index_t), 1, file);
            fwrite(&export.num_params, sizeof(int), 1, file);
            fwrite(&export.return_type, sizeof(baranium_variable_type_t), 1, file);
            fwrite(&export.symnamelen, sizeof(size_t), 1, file);
            if (export.symname)
            {
                fwrite(export.symname, sizeof(char), export.symnamelen-externalsymbolnameappendixlength, file);
                fwrite(externalsymbolnameappendix, sizeof(char), externalsymbolnameappendixlength, file);
            }

            header.exports_count++;
        }

        size_t uselesssectioncount = 0;
        for (size_t i = 0; i < tokens->count; i++)
        {
            baranium_token* token = tokens->data[i];
            if (token->type == BARANIUM_TOKEN_TYPE_FUNCTION)
            {
                baranium_function_token* function = (baranium_function_token*)token;
                baranium_symbol_table_add_from_name_and_id(&compiler->var_table, token->name, token->id);
                if (function->only_declaration)
                {
                    uselesssectioncount++;
                    continue;
                }

                tmp = (uint8_t)BARANIUM_SCRIPT_SECTION_TYPE_FUNCTIONS; fwrite(&tmp, sizeof(uint8_t), 1, file);
                fwrite(&token->id, sizeof(index_t), 1, file);

                // "compile" the code
                baranium_compiler_code_builder_clear(compiler);
                baranium_compiler_compile_variables(compiler, &function->parameters);
                baranium_compiler_compile(compiler, &function->tokens);
                baranium_compiler_clear_variables(compiler, &function->parameters);

                // Size calculation: parameter count + return type + compiled code size
                uint64_t dataSize = 2 + compiler->code_length;
                fwrite(&dataSize, sizeof(uint64_t), 1, file);

                tmp = (uint8_t)function->parameters.count; fwrite(&tmp, sizeof(uint8_t), 1, file);
                tmp = (uint8_t)function->return_type; fwrite(&tmp, sizeof(uint8_t), 1, file);
                fwrite(compiler->code, sizeof(uint8_t), compiler->code_length, file);

                baranium_compiler_code_builder_clear(compiler);
                continue;
            }

            if (token->type == BARANIUM_TOKEN_TYPE_VARIABLE)
            {
                baranium_variable_token* variable = (baranium_variable_token*)token;
                tmp = (uint8_t)BARANIUM_SCRIPT_SECTION_TYPE_FIELDS; fwrite(&tmp, sizeof(uint8_t), 1, file);
                fwrite(&token->id, sizeof(index_t), 1, file);
                int8_t dataTypeSize = baranium_variable_get_size_of_type(variable->type);
                if (dataTypeSize == -1) // meaning this is a string
                    dataTypeSize = strlen(variable->value) + 1; // for now store the initial string's length + 1 because of the null-char at the end

                // Size calculation: data type(1 byte) + data size
                uint64_t dataSize = sizeof(uint8_t) + dataTypeSize;
                fwrite(&dataSize, sizeof(uint64_t), 1, file);
                void* data = (uint8_t*)malloc(dataSize);
                memset(data, 0, dataSize);
                baranium_compiler_copy_variable_data(data, variable->value, variable->type);
                fwrite(data, sizeof(uint8_t), dataSize, file);
                free(data);

                baranium_symbol_table_add_from_name_and_id(&compiler->var_table, token->name, token->id);
                continue;
            }

            if (token->type == BARANIUM_TOKEN_TYPE_FIELD)
            {
                baranium_field_token* field = (baranium_field_token*)token;
                tmp = (uint8_t)BARANIUM_SCRIPT_SECTION_TYPE_FIELDS; fwrite(&tmp, sizeof(uint8_t), 1, file);
                fwrite(&token->id, sizeof(index_t), 1, file);
                int8_t dataTypeSize = baranium_variable_get_size_of_type(field->type);
                if (dataTypeSize == -1) // meaning this is a string
                    dataTypeSize = strlen(field->value) + 1; // for now store the initial string's length + 1 because of the null-char at the end

                // Size calculation: data type(1 byte) + data size
                uint64_t dataSize = sizeof(uint8_t) + dataTypeSize;
                fwrite(&dataSize, sizeof(uint64_t), 1, file);
                void* data = (uint8_t*)malloc(dataSize);
                memset(data, 0, dataSize);
                baranium_compiler_copy_variable_data(data, field->value, field->type);
                fwrite(data, sizeof(uint8_t), dataSize, file);
                free(data);

                baranium_symbol_table_add_from_name_and_id(&compiler->var_table, token->name, token->id);
                continue;
            }
        }

        // update the old section count
        size_t end_pos = ftell(file);
        fseek(file, tokencountposition, SEEK_SET);
        header.section_count = tokens->count - uselesssectioncount;
        fwrite(&header.section_count, sizeof(uint64_t), 1, file);
        fseek(file, exportcountposition, SEEK_SET);
        fwrite(&header.exports_count, sizeof(uint64_t), 1, file);
        fseek(file, end_pos, SEEK_SET);
    }

    // write dependencies (shared feature on both library files and executable scripts)
    fwrite(&compiler->dependency_count, sizeof(size_t), 1, file);
    for (size_t i = 0; i < compiler->dependency_count; i++)
    {
        const char* dependency = compiler->dependencies[i];
        size_t len = strlen(dependency);
        fwrite(&len, sizeof(size_t), 1, file);
        fwrite(dependency, sizeof(char), len, file);
    }
}

void baranium_compiler_clear_compiled_code(baranium_compiler* compiler)
{
    if (compiler == NULL || compiler->code == NULL)
        return;

    baranium_symbol_table_clear(&compiler->var_table);
    memset(compiler->code, 0, compiler->code_length);
}

void baranium_compiler_copy_variable_data(void* __dst, const char* __src, baranium_variable_type_t varType)
{
    (*(uint8_t*)__dst) = (uint8_t)varType;
    void* dest = (void*)((uint64_t)__dst + 1);

    switch (varType)
    {
        default:
        case BARANIUM_VARIABLE_TYPE_INVALID:
        case BARANIUM_VARIABLE_TYPE_VOID:
            break;

        case BARANIUM_VARIABLE_TYPE_OBJECT:
        {
            int64_t objID;
            if (__src == NULL || strlen(__src) == 0)
                objID = 0;
            else if (__src == baranium_keywords[BARANIUM_KEYWORD_INDEX_NULL].name) // in this instance it *is* possible to just use a normal == comparison since keywords should not be allocated seperately at any point
                objID = 0;
            else if (__src == baranium_keywords[BARANIUM_KEYWORD_INDEX_ATTACHED].name) // in this instance it *is* possible to just use a normal == comparison since keywords should not be allocated seperately at any point
                objID = -1;
            else
                objID = (int64_t)strgetnumval(__src);
            memcpy(dest, &objID, sizeof(int64_t));
            break;
        }

        case BARANIUM_VARIABLE_TYPE_STRING:
        {
            memcpy(dest, __src, strlen(__src));
            break;
        }

        case BARANIUM_VARIABLE_TYPE_FLOAT:
        {
            float fValue;
            if (strlen(__src) != 0)
                fValue = strgetfloatval(__src);
            memcpy(dest, &fValue, sizeof(float));
            break;
        }

        case BARANIUM_VARIABLE_TYPE_BOOL:
        {
            uint8_t bValue = 0;
            if (strlen(__src) != 0)
                bValue = strcmp(__src, baranium_keywords[BARANIUM_KEYWORD_INDEX_TRUE].name) == 0;
            memcpy(dest, &bValue, sizeof(uint8_t));
            break;
        }

        case BARANIUM_VARIABLE_TYPE_INT32:
        {
            int32_t iValue = 0;
            if (strlen(__src) != 0)
                iValue = (int32_t)strgetnumval(__src);
            memcpy(dest, &iValue, sizeof(int32_t));
            break;
        }

        case BARANIUM_VARIABLE_TYPE_UINT32:
        {
            uint32_t uValue = 0;
            if (strlen(__src) != 0)
                uValue = (uint32_t)strgetnumval(__src);
            memcpy(dest, &uValue, sizeof(uint32_t));
            break;
        }

        case BARANIUM_VARIABLE_TYPE_DOUBLE:
        {
            double fValue;
            if (strlen(__src) != 0)
                fValue = strgetdoubleval(__src);
            memcpy(dest, &fValue, sizeof(double));
            break;
        }

        case BARANIUM_VARIABLE_TYPE_INT8:
        {
            int8_t iValue = 0;
            if (strlen(__src) != 0)
                iValue = (int8_t)strgetnumval(__src);
            memcpy(dest, &iValue, sizeof(int8_t));
            break;
        }

        case BARANIUM_VARIABLE_TYPE_UINT8:
        {
            uint8_t iValue = 0;
            if (strlen(__src) != 0)
                iValue = (uint8_t)strgetnumval(__src);
            memcpy(dest, &iValue, sizeof(uint8_t));
            break;
        }

        case BARANIUM_VARIABLE_TYPE_INT16:
        {
            int16_t iValue = 0;
            if (strlen(__src) != 0)
                iValue = (int16_t)strgetnumval(__src);
            memcpy(dest, &iValue, sizeof(int16_t));
            break;
        }

        case BARANIUM_VARIABLE_TYPE_UINT16:
        {
            uint16_t iValue = 0;
            if (strlen(__src) != 0)
                iValue = (uint16_t)strgetnumval(__src);
            memcpy(dest, &iValue, sizeof(uint16_t));
            break;
        }

        case BARANIUM_VARIABLE_TYPE_INT64:
        {
            int64_t iValue = 0;
            if (strlen(__src) != 0)
                iValue = (int64_t)strgetnumval(__src);
            memcpy(dest, &iValue, sizeof(int64_t));
            break;
        }

        case BARANIUM_VARIABLE_TYPE_UINT64:
        {
            uint64_t iValue = 0;
            if (strlen(__src) != 0)
                iValue = (uint64_t)strgetnumval(__src);
            memcpy(dest, &iValue, sizeof(uint64_t));
            break;
        }

    }
}

baranium_compiler baranium_compiler_predict_code_size(baranium_compiler* compiler, baranium_token_list* tokens)
{
    baranium_compiler c;
    baranium_compiler_init(&c);
    for (size_t i = 0; i < compiler->var_table.count; i++)
        baranium_symbol_table_add_from_name_and_id(&c.var_table, compiler->var_table.data[i].name, compiler->var_table.data[i].id);

    baranium_compiler_compile(&c, tokens);
    return c;
}

baranium_compiler baranium_compiler_predict_code_size_expression(baranium_compiler* compiler, baranium_expression_token* token)
{
    baranium_compiler c;
    baranium_compiler_init(&c);
    for (size_t i = 0; i < compiler->var_table.count; i++)
        baranium_symbol_table_add_from_name_and_id(&c.var_table, compiler->var_table.data[i].name, compiler->var_table.data[i].id);

    baranium_compiler_compile_expression(&c, token);
    return c;
}

baranium_value_t baranium_compiler_get_variable_value_as_data(const char* value, baranium_variable_type_t type)
{
    baranium_value_t data = {0};

    if (type == BARANIUM_VARIABLE_TYPE_INVALID || value == NULL || strlen(value) == 0)
        return data;

    if (type == BARANIUM_VARIABLE_TYPE_BOOL)
        data.num8 = strcmp(value, "1") == 0;

    if (type == BARANIUM_VARIABLE_TYPE_FLOAT)
        data.numfloat = strgetfloatval(value);

    if (type == BARANIUM_VARIABLE_TYPE_OBJECT)
        return data;

    if (type == BARANIUM_VARIABLE_TYPE_INT32)
        data.snum32 = strgetnumval(value);

    if (type == BARANIUM_VARIABLE_TYPE_UINT32)
        data.num32 = strgetnumval(value);

    if (type == BARANIUM_VARIABLE_TYPE_DOUBLE)
        data.numdouble = strgetdoubleval(value);

    if (type == BARANIUM_VARIABLE_TYPE_INT8)
        data.snum8 = strgetnumval(value);

    if (type == BARANIUM_VARIABLE_TYPE_UINT8)
        data.num8 = strgetnumval(value);

    if (type == BARANIUM_VARIABLE_TYPE_INT16)
        data.snum16 = strgetnumval(value);

    if (type == BARANIUM_VARIABLE_TYPE_UINT16)
        data.num16 = strgetnumval(value);

    if (type == BARANIUM_VARIABLE_TYPE_INT64)
        data.snum64 = strgetnumval(value);

    if (type == BARANIUM_VARIABLE_TYPE_UINT64)
        data.num64 = strgetnumval(value);

    if (type == BARANIUM_VARIABLE_TYPE_STRING)
    {
        data.ptr = malloc(strlen(value) + 1);
        memset(data.ptr, 0, strlen(value) + 1);
        memcpy(data.ptr, value, strlen(value));
    }

    return data;
}

void baranium_compiler_compile_variable(baranium_compiler* compiler, baranium_variable_token* token)
{
    size_t size = baranium_variable_get_size_of_type(token->type);
    if (token->type == BARANIUM_VARIABLE_TYPE_STRING)
        size = strlen(token->value) + 1; // plus the nullchar at the end

    baranium_compiler_code_builder_MEM(compiler, size, (uint8_t)token->type, token->base.id);
    if (token->init_expression.expression_type != BARANIUM_EXPRESSION_TYPE_INVALID)
        baranium_compiler_compile_expression(compiler, &token->init_expression);
    else if (token->type != BARANIUM_VARIABLE_TYPE_OBJECT)
    {
        baranium_value_t data = baranium_compiler_get_variable_value_as_data(token->value, token->type);
        baranium_compiler_code_builder_SET(compiler, token->base.id, size, data, token->type);
    }
    else
    {
        if (token->value == baranium_keywords[BARANIUM_KEYWORD_INDEX_ATTACHED].name)
            baranium_compiler_code_builder_PUSH(compiler, -1);
        else if (token->value == baranium_keywords[BARANIUM_KEYWORD_INDEX_NULL].name)
            baranium_compiler_code_builder_PUSH(compiler, 0);
        else if (strisnum(token->value))
            baranium_compiler_code_builder_PUSH(compiler, strgetnumval(token->value));
        else
        {
            index_t varID = baranium_compiler_get_id(compiler, token->value, -1);
            baranium_compiler_code_builder_PUSHVAR(compiler, varID);
            baranium_compiler_code_builder_POPVAR(compiler, token->base.id);
        }
        baranium_compiler_code_builder_POPVAR(compiler, token->base.id);
    }

    baranium_symbol_table_add(&compiler->var_table, token);
}

void baranium_compiler_compile_if_else_statement(baranium_compiler* compiler, baranium_if_else_token* token)
{
    baranium_compiler code = baranium_compiler_predict_code_size(compiler, &token->tokens);
    baranium_compiler_code_builder_SCF(compiler);
    baranium_compiler_compile_expression(compiler, &token->condition);
    baranium_compiler_code_builder_PUSHCV(compiler);
    baranium_compiler_code_builder_ICV(compiler);
    baranium_compiler_code_builder_JMPCOFF(compiler, code.code_length);
    for (size_t i = 0; i < code.code_length; i++)
        baranium_compiler_code_builder_push(compiler, code.code[i]);

    for (size_t i = 0; i < token->chained_statements.count; i++)
        baranium_compiler_compile_if_else_sub_statement(compiler, (baranium_if_else_token*)token->chained_statements.data[i]);

    baranium_compiler_code_builder_POPCV(compiler);
    baranium_compiler_code_builder_CCF(compiler);

    baranium_compiler_dispose(&code);
}

void baranium_compiler_compile_if_else_sub_statement(baranium_compiler* compiler, baranium_if_else_token* token)
{
    if (token->condition.base.type == BARANIUM_TOKEN_TYPE_INVALID)
    {
        baranium_compiler_compile_else_statement(compiler, token);
        return;
    }

    baranium_compiler code = baranium_compiler_predict_code_size(compiler, &token->tokens);
    baranium_compiler condition = baranium_compiler_predict_code_size_expression(compiler, &token->condition);
    baranium_compiler_code_builder_SCF(compiler);
    baranium_compiler_code_builder_POPCV(compiler);
    baranium_compiler_code_builder_PUSHCV(compiler);
    baranium_compiler_code_builder_JMPCOFF(compiler, condition.code_length + 3); // + 3 because POPCV, PUSHCV and ICV are one byte instructions each
    baranium_compiler_code_builder_POPCV(compiler);
    for (size_t i = 0; i < condition.code_length; i++)
        baranium_compiler_code_builder_push(compiler, condition.code[i]);
    baranium_compiler_code_builder_PUSHCV(compiler);
    baranium_compiler_code_builder_ICV(compiler);
    baranium_compiler_code_builder_JMPCOFF(compiler, code.code_length);
    for (size_t i = 0; i < code.code_length; i++)
        baranium_compiler_code_builder_push(compiler, code.code[i]);

    baranium_compiler_dispose(&code);
    baranium_compiler_dispose(&condition);
}

void baranium_compiler_compile_else_statement(baranium_compiler* compiler, baranium_if_else_token* token)
{
    if (token->condition.base.type != BARANIUM_TOKEN_TYPE_INVALID)
        return;

    baranium_compiler code = baranium_compiler_predict_code_size(compiler, &token->tokens);
    baranium_compiler_code_builder_SCF(compiler);
    baranium_compiler_code_builder_POPCV(compiler);
    baranium_compiler_code_builder_PUSHCV(compiler);
    baranium_compiler_code_builder_JMPCOFF(compiler, code.code_length);
    for (size_t i = 0; i < code.code_length; i++)
        baranium_compiler_code_builder_push(compiler, code.code[i]);

    baranium_compiler_dispose(&code);
}

void baranium_compiler_compile_do_while_loop(baranium_compiler* compiler, baranium_loop_token* token)
{
    uint64_t pointer = compiler->code_length;
    baranium_compiler offset0 = baranium_compiler_predict_code_size(compiler, &token->tokens);
    baranium_compiler offset1 = baranium_compiler_predict_code_size_expression(compiler, &token->condition);
    compiler->loop_begin_addr = pointer + offset0.code_length;
    compiler->loop_end_addr = compiler->loop_begin_addr + offset1.code_length + 13; // SCF, CCV, CCF each 1 byte, JMPC is 1 byte + 8 byte address
    baranium_compiler_compile(compiler, &token->tokens);
    baranium_compiler_code_builder_SCF(compiler);
    baranium_compiler_code_builder_CCV(compiler);
    baranium_compiler_compile_expression(compiler, &token->condition);
    baranium_compiler_code_builder_JMPC(compiler, pointer);
    baranium_compiler_code_builder_CCV(compiler);
    baranium_compiler_code_builder_CCF(compiler);

    baranium_compiler_dispose(&offset0);
    baranium_compiler_dispose(&offset1);
}

void baranium_compiler_compile_while_loop(baranium_compiler* compiler, baranium_loop_token* token)
{
    baranium_compiler offset0 = baranium_compiler_predict_code_size(compiler, &token->tokens);
    baranium_compiler offset1 = baranium_compiler_predict_code_size_expression(compiler, &token->condition);
    baranium_compiler_code_builder_JMPOFF(compiler, offset0.code_length);
    uint64_t pointer = compiler->code_length;
    compiler->loop_begin_addr = pointer + offset0.code_length;
    compiler->loop_end_addr = compiler->loop_begin_addr + offset1.code_length + 13; // SCF, CCV, CCF each 1 byte, JMPC is 1 byte + 8 byte address
    baranium_compiler_compile(compiler, &token->tokens);
    baranium_compiler_code_builder_SCF(compiler);
    baranium_compiler_code_builder_CCV(compiler);
    baranium_compiler_compile_expression(compiler, &token->condition);
    baranium_compiler_code_builder_JMPC(compiler, pointer);
    baranium_compiler_code_builder_CCV(compiler);
    baranium_compiler_code_builder_CCF(compiler);

    baranium_compiler_dispose(&offset0);
    baranium_compiler_dispose(&offset1);
}

void baranium_compiler_compile_for_loop(baranium_compiler* compiler, baranium_loop_token* token)
{
    baranium_compiler_compile_variable(compiler, &token->start_variable); // either a variable was declared
    baranium_compiler_compile_expression(compiler, &token->start_expression);   // or a starting expression
    baranium_compiler offset0 = baranium_compiler_predict_code_size(compiler, &token->tokens);
    baranium_compiler offset1 = baranium_compiler_predict_code_size_expression(compiler, &token->iteration);
    baranium_compiler offset2 = baranium_compiler_predict_code_size_expression(compiler, &token->condition);
    int offset = offset0.code_length + offset1.code_length;
    baranium_compiler_code_builder_JMPOFF(compiler, offset);
    uint64_t pointer = compiler->code_length;
    compiler->loop_begin_addr = pointer + offset0.code_length;
    compiler->loop_end_addr = compiler->loop_begin_addr + offset2.code_length + offset1.code_length + 11; // SCF, CCV each 1 byte, JMPC is 1 byte + 8 byte address
    baranium_compiler_compile(compiler, &token->tokens);
    baranium_compiler_compile_expression(compiler, &token->iteration);
    baranium_compiler_code_builder_SCF(compiler);
    baranium_compiler_code_builder_CCV(compiler);
    baranium_compiler_compile_expression(compiler, &token->condition);
    baranium_compiler_code_builder_JMPC(compiler, pointer);

    if (token->start_variable.base.id != BARANIUM_INVALID_INDEX)
    {
        baranium_symbol_table_remove(&compiler->var_table, &token->start_variable);
        baranium_compiler_code_builder_FEM(compiler, token->start_variable.base.id);
    }

    baranium_compiler_dispose(&offset0);
    baranium_compiler_dispose(&offset1);
    baranium_compiler_dispose(&offset2);
}

index_t baranium_compiler_get_id(baranium_compiler* compiler, const char* name, int lineNumber)
{
    if (compiler == NULL || name == NULL)
        return BARANIUM_INVALID_INDEX;

    index_t varID = BARANIUM_INVALID_INDEX;
    varID = baranium_symbol_table_lookup(&compiler->var_table, name);
    if (varID == BARANIUM_INVALID_INDEX) // lookup inside a library
    {
        baranium_compiler_context* ctx = baranium_get_compiler_context();
        baranium_library* library = baranium_compiler_context_lookup(ctx, name);
        baranium_compiler_add_dependency(compiler, library);
        varID = baranium_get_id_of_name(name);
    }

    if (varID == BARANIUM_INVALID_INDEX)// ok, the script author really doesn't know
    {
        LOGERROR("Line %d: No symbol with name '%s' (ID: '%lld') found", lineNumber, name, varID);
        return BARANIUM_INVALID_INDEX;
    }

    return varID;
}

void baranium_compiler_compile_expression(baranium_compiler* compiler, baranium_expression_token* token)
{
    baranium_abstract_syntax_tree_node* ast_root = token->ast;

    if (token->expression_type == BARANIUM_EXPRESSION_TYPE_INVALID || token->expression_type == BARANIUM_EXPRESSION_TYPE_NONE || ast_root == NULL)
    {
        baranium_compiler_code_builder_NOP(compiler);
        baranium_compiler_code_builder_KILL(compiler, -69420);
        return;
    }

    if (token->expression_type == BARANIUM_EXPRESSION_TYPE_CONDITION)
    {
        baranium_compiler_compile_condition(compiler, ast_root);
        return;
    }

    if (token->expression_type == BARANIUM_EXPRESSION_TYPE_ASSIGNMENT)
    {
        baranium_compiler_compile_assignment(compiler, ast_root);
        return;
    }

    if (token->expression_type == BARANIUM_EXPRESSION_TYPE_FUNCTION_CALL)
    {
        baranium_compiler_compile_function_call(compiler, token->ast);
        return;
    }

    if (token->expression_type == BARANIUM_EXPRESSION_TYPE_RETURN_STATEMENT)
    {
        baranium_compiler_compile_return_statement(compiler, token);
        return;
    }

    if (token->expression_type == BARANIUM_EXPRESSION_TYPE_KEYWORD_EXPRESSION)
    {
        baranium_compiler_compile_keyword_expression(compiler, token);
        return;
    }

    if (token->expression_type == BARANIUM_EXPRESSION_TYPE_ARITHMETIC_OPERATION)
    {
        baranium_compiler_compile_arithmetic_operation(compiler, ast_root, 1);
        return;
    }
}

void baranium_compiler_compile_ast_node(baranium_compiler* compiler, baranium_abstract_syntax_tree_node* node, uint8_t isRoot)
{
    if (node == NULL) return;

    if (node->sub_nodes.count > 0)
    {
        baranium_compiler_compile_function_call(compiler, node);
        return;
    }

    baranium_source_token token = node->contents;

    if (token.type == BARANIUM_SOURCE_TOKEN_TYPE_NUMBER && !isRoot)
    {
        uint64_t value = 0;
        if (stridx(token.contents, '.') != -1)
        {
            float val = strgetfloatval(token.contents);
            baranium_compiler_code_builder_push_float(compiler, val);
        }
        else
        {
            value = strgetnumval(token.contents);
            baranium_compiler_code_builder_push_int(compiler, value);
        }
    }
    else if (token.type == BARANIUM_SOURCE_TOKEN_TYPE_KEYWORD && !isRoot)
    {
        if (token.special_index == BARANIUM_KEYWORD_INDEX_FALSE)
            baranium_compiler_code_builder_push_bool(compiler, 0);
        else if (token.special_index == BARANIUM_KEYWORD_INDEX_TRUE)
            baranium_compiler_code_builder_push_bool(compiler, 1);
        else
        {
            LOGERROR("Line %d: Invalid keyword '%s'", token.line_number, token.contents);
            return;
        }
    }
    else if (token.type == BARANIUM_SOURCE_TOKEN_TYPE_NULL && !isRoot)
        baranium_compiler_code_builder_push_uint(compiler, 0);
    else if (token.type == BARANIUM_SOURCE_TOKEN_TYPE_TEXT && !isRoot)
        baranium_compiler_code_builder_PUSHVAR(compiler, baranium_compiler_get_id(compiler, token.contents, token.line_number));
    else if (token.type == BARANIUM_SOURCE_TOKEN_TYPE_DOUBLEQUOTE && !isRoot)
    {
        char* contents = "";
        if (node->left)
            contents = node->left->contents.contents;

        baranium_compiler_code_builder_push_string(compiler, contents);
    }

    if (token.type == BARANIUM_SOURCE_TOKEN_TYPE_PLUS          || token.type == BARANIUM_SOURCE_TOKEN_TYPE_MINUS      ||
        token.type == BARANIUM_SOURCE_TOKEN_TYPE_ASTERISK      || token.type == BARANIUM_SOURCE_TOKEN_TYPE_SLASH      ||
        token.type == BARANIUM_SOURCE_TOKEN_TYPE_MODULO        || token.type == BARANIUM_SOURCE_TOKEN_TYPE_AND        ||
        token.type == BARANIUM_SOURCE_TOKEN_TYPE_OR            || token.type == BARANIUM_SOURCE_TOKEN_TYPE_TILDE      ||
        token.type == BARANIUM_SOURCE_TOKEN_TYPE_MINUSMINUS    || token.type == BARANIUM_SOURCE_TOKEN_TYPE_PLUSPLUS   ||
        token.type == BARANIUM_SOURCE_TOKEN_TYPE_CARET)
        baranium_compiler_compile_arithmetic_operation(compiler, node, 0);

    if (token.type == BARANIUM_SOURCE_TOKEN_TYPE_EQUALSIGN    || token.type == BARANIUM_SOURCE_TOKEN_TYPE_MODEQUAL  ||
        token.type == BARANIUM_SOURCE_TOKEN_TYPE_DIVEQUAL     || token.type == BARANIUM_SOURCE_TOKEN_TYPE_MULEQUAL  ||
        token.type == BARANIUM_SOURCE_TOKEN_TYPE_MINUSEQUAL   || token.type == BARANIUM_SOURCE_TOKEN_TYPE_PLUSEQUAL ||
        token.type == BARANIUM_SOURCE_TOKEN_TYPE_ANDEQUAL     || token.type == BARANIUM_SOURCE_TOKEN_TYPE_OREQUAL   ||
        token.type == BARANIUM_SOURCE_TOKEN_TYPE_XOREQUAL)
        baranium_compiler_compile_assignment(compiler, node);

    if (token.type == BARANIUM_SOURCE_TOKEN_TYPE_EQUALTO   || token.type == BARANIUM_SOURCE_TOKEN_TYPE_NOTEQUAL       ||
        token.type == BARANIUM_SOURCE_TOKEN_TYPE_LESSEQUAL || token.type == BARANIUM_SOURCE_TOKEN_TYPE_GREATEREQUAL   ||
        token.type == BARANIUM_SOURCE_TOKEN_TYPE_LESSTHAN  || token.type == BARANIUM_SOURCE_TOKEN_TYPE_GREATERTHAN    ||
        token.type == BARANIUM_SOURCE_TOKEN_TYPE_ANDAND    || token.type == BARANIUM_SOURCE_TOKEN_TYPE_OROR)
    {
        baranium_compiler_compile_condition(compiler, node);
        baranium_compiler_code_builder_PUSHCV(compiler);
    }
}

void baranium_compiler_compile_assignment(baranium_compiler* compiler, baranium_abstract_syntax_tree_node* root)
{
    if (root == NULL) return;

    baranium_source_token leftToken = root->left->contents;
    if (leftToken.type != BARANIUM_SOURCE_TOKEN_TYPE_TEXT)
    {
        LOGERROR("Line %d: Invalid assignment, no variable name given, instead found '%s'", leftToken.line_number, leftToken.contents);
        return;
    }

    const char* varName = leftToken.contents;
    index_t varID = baranium_compiler_get_id(compiler, varName, leftToken.line_number);

    baranium_compiler_compile_ast_node(compiler, root->right, 0);

    if (root->contents.type != BARANIUM_SOURCE_TOKEN_TYPE_EQUALSIGN)
        baranium_compiler_code_builder_PUSHVAR(compiler, varID);

    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_MODEQUAL)
        baranium_compiler_code_builder_MOD(compiler);
    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_DIVEQUAL)
        baranium_compiler_code_builder_DIV(compiler);
    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_MULEQUAL)
        baranium_compiler_code_builder_MUL(compiler);
    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_MINUSEQUAL)
        baranium_compiler_code_builder_SUB(compiler);
    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_PLUSEQUAL)
        baranium_compiler_code_builder_ADD(compiler);
    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_ANDEQUAL)
        baranium_compiler_code_builder_AND(compiler);
    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_OREQUAL)
        baranium_compiler_code_builder_OR(compiler);
    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_XOREQUAL)
        baranium_compiler_code_builder_XOR(compiler);

    baranium_compiler_code_builder_POPVAR(compiler, varID);
}

void baranium_compiler_compile_return_statement(baranium_compiler* compiler, baranium_expression_token* expression)
{
    if (expression->return_variable != NULL && strcmp(expression->return_variable, "") != 0)
    {
        const char* varName = expression->return_variable;
        index_t varID = baranium_compiler_get_id(compiler, varName, expression->line_number);
        baranium_compiler_code_builder_PUSHVAR(compiler, varID);
    }
    else if (expression->return_expression != NULL)
        baranium_compiler_compile_ast_node(compiler, expression->return_expression->ast, 1);
    else if (expression->return_type != BARANIUM_VARIABLE_TYPE_VOID)
    {
        baranium_variable_token var;
        baranium_variable_token_init(&var);
        var.type = expression->return_type;
        var.value = expression->return_value;
        var.base.id = 0;
        baranium_compiler_compile_variable(compiler, &var);
        baranium_compiler_code_builder_PUSHVAR(compiler, 0);
        baranium_compiler_code_builder_FEM(compiler, 0);
        baranium_symbol_table_remove(&compiler->var_table, &var);
    }

    // clear all currently allocated variables
    for (size_t i = 0; i < compiler->var_table.count; i++)
        baranium_compiler_code_builder_FEM(compiler, compiler->var_table.data[i].id);

    baranium_compiler_code_builder_RET(compiler);
}

void baranium_compiler_compile_arithmetic_operation(baranium_compiler* compiler, baranium_abstract_syntax_tree_node* root, uint8_t isRoot)
{
    if (root == NULL) return;

    baranium_abstract_syntax_tree_node* lhs = root->left;
    baranium_abstract_syntax_tree_node* rhs = root->right;

    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_MINUSMINUS && lhs->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_TEXT)
    {
        // unfortunately i don't know any better way right now
        // to do this other than having duplicated code, sorry
        const char* varName = lhs->contents.contents;
        index_t varID = baranium_compiler_get_id(compiler, varName, lhs->contents.line_number);
        baranium_compiler_code_builder_PUSHVAR(compiler, varID);
        baranium_compiler_code_builder_push_int(compiler, 1);
        baranium_compiler_code_builder_SUB(compiler);
        baranium_compiler_code_builder_POPVAR(compiler, varID);
        return;
    }
    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_PLUSPLUS && lhs->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_TEXT)
    {
        // unfortunately i don't know any better way right now
        // to do this other than having duplicated code, sorry
        const char* varName = lhs->contents.contents;
        index_t varID = baranium_compiler_get_id(compiler, varName, lhs->contents.line_number);
        baranium_compiler_code_builder_PUSHVAR(compiler, varID);
        baranium_compiler_code_builder_push_int(compiler, 1);
        baranium_compiler_code_builder_ADD(compiler);
        baranium_compiler_code_builder_POPVAR(compiler, varID);
        return;
    }

    if (isRoot)
    {
        // having an arithmetic operation as a normal expression can
        // be harmful to the stack so we have to be careful and avoid
        // compilation of arithmetic expressions if they are the root
        // expression, even if a subexpression is something else
        baranium_compiler_code_builder_NOP(compiler);
        return;
    }

    if (lhs)
        baranium_compiler_compile_ast_node(compiler, lhs, 0);
    else
        baranium_compiler_code_builder_push_int(compiler, 0);

    if (rhs)
        baranium_compiler_compile_ast_node(compiler, rhs, 0);
    else
        baranium_compiler_code_builder_push_int(compiler, 0);

    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_MODULO)
        baranium_compiler_code_builder_MOD(compiler);
    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_SLASH)
        baranium_compiler_code_builder_DIV(compiler);
    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_ASTERISK)
        baranium_compiler_code_builder_MUL(compiler);
    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_MINUS)
        baranium_compiler_code_builder_SUB(compiler);
    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_PLUS)
        baranium_compiler_code_builder_ADD(compiler);
    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_AND)
        baranium_compiler_code_builder_AND(compiler);
    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_OR)
        baranium_compiler_code_builder_OR(compiler);
    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_CARET)
        baranium_compiler_code_builder_XOR(compiler);
    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_BITSHIFTLEFT)
        baranium_compiler_code_builder_SHFTL(compiler);
    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_BITSHIFTRIGHT)
        baranium_compiler_code_builder_SHFTR(compiler);
}

void baranium_compiler_compile_condition(baranium_compiler* compiler, baranium_abstract_syntax_tree_node* root)
{
    if (root == NULL) return;

    baranium_abstract_syntax_tree_node* lhs = root->left;
    baranium_abstract_syntax_tree_node* rhs = root->right;

    if (lhs)
        baranium_compiler_compile_ast_node(compiler, lhs, 0);
    else
        baranium_compiler_code_builder_push_int(compiler, 0);

    if (rhs)
        baranium_compiler_compile_ast_node(compiler, rhs, 0);
    else
        baranium_compiler_code_builder_push_int(compiler, 0);

    if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_ANDAND)
        baranium_compiler_code_builder_CMPC(compiler, BARANIUM_CMP_AND);
    else if (root->contents.type == BARANIUM_SOURCE_TOKEN_TYPE_OROR)
        baranium_compiler_code_builder_CMPC(compiler, BARANIUM_CMP_OR);
    else
        baranium_compiler_code_builder_CMP(compiler, baranium_compiler_get_compare_method(root->contents.type));
}

void baranium_compiler_compile_keyword_expression(baranium_compiler* compiler, baranium_expression_token* expression)
{
    const char* keyword = expression->ast->contents.contents;

    if (expression->return_value == baranium_keywords[BARANIUM_KEYWORD_INDEX_NULL].name)
    {
        // it wouldn't make sense to do any sort of operation on a non-existent object
        baranium_compiler_code_builder_NOP(compiler);
        return;
    }
    else if (expression->return_value == baranium_keywords[BARANIUM_KEYWORD_INDEX_ATTACHED].name)
        baranium_compiler_code_builder_push_int(compiler, -1);
    else if (expression->return_value != NULL)
    {
        index_t id = baranium_compiler_get_id(compiler, expression->return_value, expression->line_number);
        ///TODO: check if variable is an signed/unsigned integer or an object, since any other type doesn't make sense
        baranium_compiler_code_builder_PUSHVAR(compiler, id);
    }

    if (keyword == baranium_keywords[BARANIUM_KEYWORD_INDEX_INSTANTIATE].name)
    {
        baranium_compiler_code_builder_INSTANTIATE(compiler);
        return;
    }
    else if (keyword == baranium_keywords[BARANIUM_KEYWORD_INDEX_DELETE].name)
    {
        baranium_compiler_code_builder_DELETE(compiler);
        return;
    }
    else if (keyword == baranium_keywords[BARANIUM_KEYWORD_INDEX_ATTACH].name)
    {
        baranium_compiler_code_builder_ATTACH(compiler);
        return;
    }
    else if (keyword == baranium_keywords[BARANIUM_KEYWORD_INDEX_DETACH].name)
    {
        baranium_compiler_code_builder_DETACH(compiler);
        return;
    }
    else if (keyword == baranium_keywords[BARANIUM_KEYWORD_INDEX_BREAK].name)
    {
        baranium_compiler_code_builder_JMP(compiler, compiler->loop_end_addr);
        return;
    }
    else if (keyword == baranium_keywords[BARANIUM_KEYWORD_INDEX_CONTINUE].name)
    {
        baranium_compiler_code_builder_JMP(compiler, compiler->loop_begin_addr);
        return;
    }

    LOGERROR("Line %d: Unknown keyword '%s' or maybe variable doesn't exist?", expression->line_number, keyword);
}

void baranium_compiler_compile_function_call(baranium_compiler* compiler, baranium_abstract_syntax_tree_node* node)
{
    const char* functionName = node->contents.contents;
    index_t id = baranium_compiler_get_id(compiler, functionName, node->contents.line_number);
    for (size_t i = 0; i < node->sub_nodes.count; i++)
        baranium_compiler_compile_ast_node(compiler, node->sub_nodes.nodes[i], 0);

    baranium_compiler_code_builder_CALL(compiler, id);
}

uint8_t baranium_compiler_get_compare_method(baranium_source_token_type_t type)
{
    switch (type)
    {
        default:
        case BARANIUM_SOURCE_TOKEN_TYPE_EQUALTO:
            return BARANIUM_CMP_EQUAL;
        case BARANIUM_SOURCE_TOKEN_TYPE_NOTEQUAL:
            return BARANIUM_CMP_NOTEQUAL;
        case BARANIUM_SOURCE_TOKEN_TYPE_LESSTHAN:
            return BARANIUM_CMP_LESS_THAN;
        case BARANIUM_SOURCE_TOKEN_TYPE_LESSEQUAL:
            return BARANIUM_CMP_LESS_EQUAL;
        case BARANIUM_SOURCE_TOKEN_TYPE_GREATERTHAN:
            return BARANIUM_CMP_GREATER_THAN;
        case BARANIUM_SOURCE_TOKEN_TYPE_GREATEREQUAL:
            return BARANIUM_CMP_GREATER_EQUAL;
    }
    return BARANIUM_CMP_EQUAL;
}

void baranium_compiler_compile_variables(baranium_compiler* compiler, baranium_token_list* variables)
{
    for (size_t var = variables->count; var > 0; var--)
    {
        baranium_variable_token* entry = (baranium_variable_token*)variables->data[var-1];
        baranium_compiler_compile_variable(compiler, entry);
        baranium_compiler_code_builder_POPVAR(compiler, entry->base.id);
    }
}

void baranium_compiler_clear_variables(baranium_compiler* compiler, baranium_token_list* variables)
{
    if (baranium_compiler_code_builder_returned_from_execution(compiler))
        return;

    for (size_t i = 0; i < variables->count; i++)
    {
        baranium_compiler_code_builder_FEM(compiler, variables->data[i]->id);
        baranium_symbol_table_remove(&compiler->var_table, (baranium_variable_token*)variables->data[i]);
    }
}

void baranium_compiler_compile(baranium_compiler* compiler, baranium_token_list* tokens)
{
    for (size_t i = 0; i < tokens->count; i++)
    {
        baranium_token* token = tokens->data[i];

        switch(token->type)
        {
        default:
            continue;

        case BARANIUM_TOKEN_TYPE_FUNCTION:
        {
            LOGERROR("Trying to compile function inside function, bruh");
            return;
        }

        case BARANIUM_TOKEN_TYPE_FIELD:
        {
            LOGERROR("Fields should not be inside functions my man");
            return;
        }

        case BARANIUM_TOKEN_TYPE_VARIABLE:
            baranium_compiler_compile_variable(compiler, (baranium_variable_token*)token);
            continue;

        case BARANIUM_TOKEN_TYPE_EXPRESSION:
            baranium_compiler_compile_expression(compiler, (baranium_expression_token*)token);
            continue;

        case BARANIUM_TOKEN_TYPE_IFELSESTATEMENT:
            baranium_compiler_compile_if_else_statement(compiler, (baranium_if_else_token*)token);
            continue;

        case BARANIUM_TOKEN_TYPE_DOWHILELOOP:
            baranium_compiler_compile_do_while_loop(compiler, (baranium_loop_token*)token);
            continue;

        case BARANIUM_TOKEN_TYPE_WHILELOOP:
            baranium_compiler_compile_while_loop(compiler, (baranium_loop_token*)token);
            continue;

        case BARANIUM_TOKEN_TYPE_FORLOOP:
            baranium_compiler_compile_for_loop(compiler, (baranium_loop_token*)token);
            continue;
        }
    }
}

void baranium_compiler_code_builder_init(baranium_compiler* compiler)
{
    compiler->code_buffer_size = 0;
    compiler->code_length = 0;
    compiler->code = NULL;
}

void baranium_compiler_code_builder_dispose(baranium_compiler* compiler)
{
    if (compiler->code_length == 0 || compiler->code == NULL)
        return;

    free(compiler->code);
    compiler->code_buffer_size = 0;
    compiler->code_length = 0;
    compiler->code = NULL;
}

void baranium_compiler_code_builder_clear(baranium_compiler* compiler)
{
    compiler->code_length = 0;
}

uint8_t baranium_compiler_code_builder_returned_from_execution(baranium_compiler* compiler)
{
    if (compiler->code_length == 0 || compiler->code == NULL)
        return 1;

    return compiler->code[compiler->code_length-1] == 0x0F;
}

void baranium_compiler_code_builder_push_string(baranium_compiler* compiler, const char* str)
{
    uint64_t data = 0;

    size_t strLength = strlen(str);
    size_t leftOverSize = strLength;
    void* value = (void*)str;
    void* valPtr = (void*)str;
    size_t index = 0;
    for (index = 0; index * 8 < strLength; index++)
    {
        if (leftOverSize <= 8)
            break;

        valPtr = (void*)((uint64_t)value + index*8);
        memcpy(&data, valPtr, 8);
        leftOverSize -= 8;
        baranium_compiler_code_builder_PUSH(compiler, data);
    }
    valPtr = (void*)((uint64_t)value + index*8);
    data = 0;
    memcpy(&data, valPtr, leftOverSize);
    baranium_compiler_code_builder_PUSH(compiler, data);
    baranium_compiler_code_builder_PUSH(compiler, strLength);
    baranium_compiler_code_builder_PUSH(compiler, (uint64_t)BARANIUM_VARIABLE_TYPE_STRING);
}

void baranium_compiler_code_builder_push_bool(baranium_compiler* compiler, uint8_t b)
{
    baranium_compiler_code_builder_PUSH(compiler, b);
    baranium_compiler_code_builder_PUSH(compiler, sizeof(uint8_t));
    baranium_compiler_code_builder_PUSH(compiler, (uint64_t)BARANIUM_VARIABLE_TYPE_BOOL);
}

void baranium_compiler_code_builder_push_uint(baranium_compiler* compiler, uint32_t val)
{
    baranium_compiler_code_builder_PUSH(compiler, val);
    baranium_compiler_code_builder_PUSH(compiler, sizeof(uint32_t));
    baranium_compiler_code_builder_PUSH(compiler, (uint64_t)BARANIUM_VARIABLE_TYPE_UINT32);
}

void baranium_compiler_code_builder_push_int(baranium_compiler* compiler, int32_t val)
{
    baranium_compiler_code_builder_PUSH(compiler, val);
    baranium_compiler_code_builder_PUSH(compiler, sizeof(int32_t));
    baranium_compiler_code_builder_PUSH(compiler, (uint64_t)BARANIUM_VARIABLE_TYPE_INT32);
}

void baranium_compiler_code_builder_push_float(baranium_compiler* compiler, float val)
{
    uint64_t data = 0;
    memcpy(&data, &val, sizeof(float));
    baranium_compiler_code_builder_PUSH(compiler, data);
    baranium_compiler_code_builder_PUSH(compiler, sizeof(float));
    baranium_compiler_code_builder_PUSH(compiler, (uint64_t)BARANIUM_VARIABLE_TYPE_FLOAT);
}

void baranium_compiler_code_builder_push64(baranium_compiler* compiler, uint64_t data)
{
    baranium_compiler_code_builder_push(compiler, (uint8_t)((0xFF00000000000000 & data) >> 56));
    baranium_compiler_code_builder_push(compiler, (uint8_t)((0xFF000000000000 & data) >> 48));
    baranium_compiler_code_builder_push(compiler, (uint8_t)((0xFF0000000000 & data) >> 40));
    baranium_compiler_code_builder_push(compiler, (uint8_t)((0xFF00000000 & data) >> 32));
    baranium_compiler_code_builder_push(compiler, (uint8_t)((0xFF000000 & data) >> 24));
    baranium_compiler_code_builder_push(compiler, (uint8_t)((0xFF0000 & data) >> 16));
    baranium_compiler_code_builder_push(compiler, (uint8_t)((0xFF00 & data) >> 8));
    baranium_compiler_code_builder_push(compiler, (uint8_t)(0xFF & data));
}

void baranium_compiler_code_builder_push32(baranium_compiler* compiler, uint32_t data)
{
    baranium_compiler_code_builder_push(compiler, (uint8_t)((0xFF000000 & data) >> 24));
    baranium_compiler_code_builder_push(compiler, (uint8_t)((0xFF0000 & data) >> 16));
    baranium_compiler_code_builder_push(compiler, (uint8_t)((0xFF00 & data) >> 8));
    baranium_compiler_code_builder_push(compiler, (uint8_t)(0xFF & data));
}

void baranium_compiler_code_builder_push16(baranium_compiler* compiler, uint16_t data)
{
    baranium_compiler_code_builder_push(compiler, (uint8_t)((0xFF00 & data) >> 8));
    baranium_compiler_code_builder_push(compiler, (uint8_t)(0xFF & data));
}

void baranium_compiler_code_builder_push(baranium_compiler* compiler, uint8_t data)
{
    if (compiler->code_length + 1 >= compiler->code_buffer_size)
    {
        compiler->code_buffer_size += BARANIUM_CODE_BUFFER_SIZE;
        compiler->code = realloc(compiler->code, compiler->code_buffer_size);
    }

    compiler->code[compiler->code_length] = data;
    compiler->code_length++;
}

void baranium_compiler_code_builder_NOP(baranium_compiler* compiler)
{
    baranium_compiler_code_builder_push(compiler, 0x00);
}

void baranium_compiler_code_builder_CCF(baranium_compiler* compiler)
{
    baranium_compiler_code_builder_push(compiler, 0x01);
}

void baranium_compiler_code_builder_SCF(baranium_compiler* compiler)
{
    baranium_compiler_code_builder_push(compiler, 0x02);
}

void baranium_compiler_code_builder_CCV(baranium_compiler* compiler)
{
    baranium_compiler_code_builder_push(compiler, 0x03);
}

void baranium_compiler_code_builder_ICV(baranium_compiler* compiler)
{
    baranium_compiler_code_builder_push(compiler, 0x04);
}

void baranium_compiler_code_builder_PUSHCV(baranium_compiler* compiler)
{
    baranium_compiler_code_builder_push(compiler, 0x05);
}

void baranium_compiler_code_builder_POPCV(baranium_compiler* compiler)
{
    baranium_compiler_code_builder_push(compiler, 0x06);
}

void baranium_compiler_code_builder_PUSHVAR(baranium_compiler* compiler, index_t id)
{
    baranium_compiler_code_builder_push(compiler, 0x07);
    baranium_compiler_code_builder_push64(compiler, id);
}

void baranium_compiler_code_builder_POPVAR(baranium_compiler* compiler, index_t id)
{
    baranium_compiler_code_builder_push(compiler, 0x08);
    baranium_compiler_code_builder_push64(compiler, id);
}

void baranium_compiler_code_builder_PUSH(baranium_compiler* compiler, uint64_t val)
{
    baranium_compiler_code_builder_push(compiler, 0x09);
    baranium_compiler_code_builder_push64(compiler, val);
}

void baranium_compiler_code_builder_CALL(baranium_compiler* compiler, index_t id)
{
    baranium_compiler_code_builder_push(compiler, 0x0E);
    baranium_compiler_code_builder_push64(compiler, id);
}

void baranium_compiler_code_builder_RET(baranium_compiler* compiler)
{
    baranium_compiler_code_builder_push(compiler, 0x0F);
}

void baranium_compiler_code_builder_JMP(baranium_compiler* compiler, uint64_t addr)
{
    baranium_compiler_code_builder_push(compiler, 0x10);
    baranium_compiler_code_builder_push64(compiler, addr);
}

void baranium_compiler_code_builder_JMPOFF(baranium_compiler* compiler, int16_t offset)
{
    baranium_compiler_code_builder_push(compiler, 0x11);
    baranium_compiler_code_builder_push16(compiler, offset);
}

void baranium_compiler_code_builder_JMPC(baranium_compiler* compiler, uint64_t addr)
{
    baranium_compiler_code_builder_push(compiler, 0x12);
    baranium_compiler_code_builder_push64(compiler, addr);
}

void baranium_compiler_code_builder_JMPCOFF(baranium_compiler* compiler, int16_t addr)
{
    baranium_compiler_code_builder_push(compiler, 0x13);
    baranium_compiler_code_builder_push16(compiler, addr);
}

void baranium_compiler_code_builder_MOD(baranium_compiler* compiler)     { baranium_compiler_code_builder_push(compiler, 0x20); }
void baranium_compiler_code_builder_DIV(baranium_compiler* compiler)     { baranium_compiler_code_builder_push(compiler, 0x21); }
void baranium_compiler_code_builder_MUL(baranium_compiler* compiler)     { baranium_compiler_code_builder_push(compiler, 0x22); }
void baranium_compiler_code_builder_SUB(baranium_compiler* compiler)     { baranium_compiler_code_builder_push(compiler, 0x23); }
void baranium_compiler_code_builder_ADD(baranium_compiler* compiler)     { baranium_compiler_code_builder_push(compiler, 0x24); }
void baranium_compiler_code_builder_AND(baranium_compiler* compiler)     { baranium_compiler_code_builder_push(compiler, 0x25); }
void baranium_compiler_code_builder_OR(baranium_compiler* compiler)      { baranium_compiler_code_builder_push(compiler, 0x26); }
void baranium_compiler_code_builder_XOR(baranium_compiler* compiler)     { baranium_compiler_code_builder_push(compiler, 0x27); }
void baranium_compiler_code_builder_SHFTL(baranium_compiler* compiler)   { baranium_compiler_code_builder_push(compiler, 0x28); }
void baranium_compiler_code_builder_SHFTR(baranium_compiler* compiler)   { baranium_compiler_code_builder_push(compiler, 0x29); }

void baranium_compiler_code_builder_CMP(baranium_compiler* compiler, uint8_t compareMethod)
{
    baranium_compiler_code_builder_push(compiler, 0x30);
    baranium_compiler_code_builder_push(compiler, compareMethod);
}

void baranium_compiler_code_builder_CMPC(baranium_compiler* compiler, uint8_t compareCombineMethod)
{
    baranium_compiler_code_builder_push(compiler, 0x31);
    baranium_compiler_code_builder_push(compiler, compareCombineMethod);
}

void baranium_compiler_code_builder_MEM(baranium_compiler* compiler, size_t size, uint8_t type, index_t id)
{
    baranium_compiler_code_builder_push(compiler, 0x80);
    baranium_compiler_code_builder_push64(compiler, size);
    baranium_compiler_code_builder_push(compiler, type);
    baranium_compiler_code_builder_push64(compiler, id);
}

void baranium_compiler_code_builder_FEM(baranium_compiler* compiler, index_t id)
{
    baranium_compiler_code_builder_push(compiler, 0x81);
    baranium_compiler_code_builder_push64(compiler, id);
}

void baranium_compiler_code_builder_SET(baranium_compiler* compiler, index_t id, size_t size, baranium_value_t data, baranium_variable_type_t type)
{
    baranium_compiler_code_builder_push(compiler, 0x82);
    baranium_compiler_code_builder_push64(compiler, id);
    baranium_compiler_code_builder_push64(compiler, size);

    uint8_t* dataPtr = (uint8_t*)&data.ptr;
    if (type == BARANIUM_VARIABLE_TYPE_STRING)
        dataPtr = data.ptr;
    for (size_t i = 0; i < size; i++)
    {
        baranium_compiler_code_builder_push(compiler, *dataPtr);
        dataPtr++;
    }
}

void baranium_compiler_code_builder_INSTANTIATE(baranium_compiler* compiler)     { baranium_compiler_code_builder_push(compiler, 0xD0); }
void baranium_compiler_code_builder_DELETE(baranium_compiler* compiler)          { baranium_compiler_code_builder_push(compiler, 0xD1); }
void baranium_compiler_code_builder_ATTACH(baranium_compiler* compiler)          { baranium_compiler_code_builder_push(compiler, 0xD2); }
void baranium_compiler_code_builder_DETACH(baranium_compiler* compiler)          { baranium_compiler_code_builder_push(compiler, 0xD3); }

void baranium_compiler_code_builder_KILL(baranium_compiler* compiler, int64_t code)
{
    baranium_compiler_code_builder_push(compiler, 0xFF);
    baranium_compiler_code_builder_push64(compiler, code);
}
