#ifndef __LANGUAGE__FUNCTION_H_
#define __LANGUAGE__FUNCTION_H_ 1

#include "../SourceToken.h"
#include "Variable.h"
#include "Token.h"
#include <memory>
#include <vector>

namespace Language
{

    struct Function : public Token
    {
        VariableType ReturnType;
        std::string ReturnValue;
        std::string ReturnVariableName;

        std::vector<std::shared_ptr<Variable>> mParameters;
        SourceTokenList mInnerTokens;
        TokenList mTokens;

        /**
         * @brief Construct a new `Function` object
         */
        Function();

        /**
         * @brief Parse the current inner tokens
         */
        void ParseTokens(TokenList& globalTokens);
    
    private:
        bool FunctionReturnRequested();
    };

}

#endif
