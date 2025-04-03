#include <baranium/compiler/language/field_token.h>
#include <baranium/variable.h>

void baranium_field_token_init(baranium_field_token* field)
{
    field->base.type = BARANIUM_TOKEN_TYPE_FIELD;
    field->base.name = NULL;
    field->type = BARANIUM_VARIABLE_TYPE_INVALID;
    field->value = NULL;
    field->array_size = -1;
}

void baranium_field_token_dispose(baranium_field_token* field)
{
}
