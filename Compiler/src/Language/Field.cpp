#include "Field.h"
#include "../StringUtil.h"

namespace Language
{

    /**
     * @brief Construct a new `Field` object
     */
    Field::Field()
        : Token()
    {
        mTokenType = TokenType::Field;
        AssignID();
    }

}