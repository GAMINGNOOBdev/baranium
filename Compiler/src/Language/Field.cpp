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
    }

    /**
     * @returns The string representation of this `Field`
     */
    std::string Field::ToString()
    {
        return std::string(stringf("Field{ Name{'%s'} Type{%s} Value{'%s'} }", mName.c_str(), VariableTypeToString(Type), Value.c_str()));
    }

}