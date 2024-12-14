#include "Field.h"
#include "../StringUtil.h"

namespace Language
{

    Field::Field()
        : Token()
    {
        mTokenType = TokenType::Field;
        AssignID();
    }

}
