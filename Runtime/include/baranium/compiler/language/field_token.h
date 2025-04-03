#ifndef __BARANIUM__COMPILER__LANGUAGE__FIELD_H_
#define __BARANIUM__COMPILER__LANGUAGE__FIELD_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <baranium/compiler/language/token.h>
#include <baranium/compiler/source_token.h>
#include <baranium/variable.h>
#include <stdint.h>

// since fields are a little different than
// variables we have a seperate class for them,
// but they technically have the same
// functionality, except that fields can be set
// from outside the program while variables can't
// AND SHOULDN'T ;)
typedef struct
{
    baranium_token base;
    baranium_variable_type_t type;
    size_t array_size;
    const char* value;
} baranium_field_token;

BARANIUMAPI void baranium_field_token_init(baranium_field_token* field);
BARANIUMAPI void baranium_field_token_dispose(baranium_field_token* field);

#ifdef __cplusplus
}
#endif

#endif
