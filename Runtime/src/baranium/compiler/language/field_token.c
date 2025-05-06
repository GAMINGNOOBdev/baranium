#include <baranium/compiler/language/field_token.h>
#include <baranium/variable.h>
#include <memory.h>

void baranium_field_token_init(baranium_field_token* field)
{
    memset(field, 0, sizeof(baranium_field_token));
    field->base.id = BARANIUM_INVALID_INDEX;
    field->base.type = BARANIUM_TOKEN_TYPE_FIELD;
    field->type = BARANIUM_VARIABLE_TYPE_INVALID;
    field->array_size = -1;
}

void baranium_field_token_dispose(baranium_field_token* field)
{
}
