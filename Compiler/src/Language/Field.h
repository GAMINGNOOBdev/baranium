#ifndef __LANGUAGE__FIELD_H_
#define __LANGUAGE__FIELD_H_ 1

#include "Variable.h"
#include "Token.h"

namespace Language
{

    // since fields are a little different than
    // variables we have a seperate class for them,
    // but they technically have the same
    // functionality, except that fields can be set
    // from outside the program while variables can't
    // AND SHOULDN'T ;)
    struct Field : public Token
    {
        VariableType Type;

        // because values are going to be parsed later
        // when writing the final binary, we can store
        // the value of a variable in a string until
        // that point
        std::string Value;

        /**
         * @brief Construct a new `Field` object
         */
        Field();

        /**
         * @returns The string representation of this `Field`
         */
        std::string ToString() override;
    };

}

#endif