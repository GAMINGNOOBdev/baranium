#ifndef __LANGUAGE__VARIABLE_H_
#define __LANGUAGE__VARIABLE_H_ 1

#include "Language.h"
#include "Token.h"
#include <string>

using SourceTokenList = std::vector<SourceToken>;

namespace Language
{

    enum class VariableType
    {
        Invalid = -1,
        Void,
        GameObject,
        String,
        Float,
        Bool,
        Int,
        Uint,
    };

    /**
     * @brief Get the string representation of a variable type
     * 
     * @param type The variable type that will be "stringified"
     * 
     * @return The string representation of `type`
     */
    const char* VariableTypeToString(VariableType type);

    struct Variable : public Token
    {
        /**
         * @brief Get the variable type from a single token
         * 
         * @param token The token that will be converted into the variable type
         * @return The variable type
         */
        static VariableType TypeFromToken(SourceToken& token);

        /**
         * @brief Predict the variable type from a list of tokens
         * 
         * @param tokens The list of tokens that represent the variable type
         * @return The predicted variable type 
         */
        static VariableType PredictType(SourceTokenList& tokens);

        VariableType Type;

        // because values are going to be parsed later
        // when writing the final binary, we can store
        // the value of a variable in a string until
        // that point
        std::string Value;

        /**
         * @brief Construct a new `Variable` object
         */
        Variable();

        /**
         * @returns The string representation of this `Variable`
         */
        std::string ToString() override;
    };

}

#endif