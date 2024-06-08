#ifndef __LANGUAGE__VARIABLE_H_
#define __LANGUAGE__VARIABLE_H_ 1

#include "Language.h"
#include "Token.h"
#include <string>

namespace Language
{

    enum class VariableType : uint8_t
    {
        Invalid,
        Void,
        Object,
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

    /**
     * @brief Get the amount of bytes a specific variable type takes up
     * 
     * @param type The variable type
     * 
     * @return The number of bytes the variable type takes up
     */
    int8_t VariableTypeBytes(VariableType type);

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
         * @brief Construct a new `Variable` object based on another object
         * 
         * @param object The object whose contents will be copied
         */
        Variable(std::shared_ptr<Variable> object);
    };

}


// shortcut for writing convenience
using VariableList = std::vector<std::shared_ptr<Language::Variable>>;

#endif