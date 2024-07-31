#include <baranium/field.h>
#include <stdlib.h>

void baranium_field_dispose(BaraniumField* field)
{
    if (field == NULL)
        return;

    free(field);
}