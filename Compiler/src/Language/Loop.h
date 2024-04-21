#ifndef __LANGUAGE__LOOP_H_
#define __LANGUAGE__LOOP_H_ 1

#include "../SourceToken.h"
#include "Expression.h"
#include "Variable.h"
#include "Token.h"
#include <memory>
#include <vector>
#include <string>

namespace Language
{

    struct Loop : public Token
    {
        /**
         * @brief Can only be `true` while `While` is also set to `true`.
         *        Specifies if this is a do-while-loop
         */
        bool DoWhile;

        /**
         * @brief Specifies if this is a while-loop or a for-loop
         */
        bool While;

        /**
         * @brief The starting variable declared in for-loops
         */
        std::shared_ptr<Variable> StartVariable;

        /**
         * @brief The condition for the loop to continue executing
         */
        Expression Condition;

        /**
         * @brief The expression that gets executed after every iteration if this is a for-loop
         */
        Expression Iteration;

        /**
         * @brief A list of inner unparsed source code tokens
         */
        SourceTokenList mInnerTokens;

        /**
         * @brief A list of inner parsed tokens
         */
        TokenList mTokens;

        /**
         * @brief Construct a new `Loop` object
         */
        Loop();

        /**
         * @brief Construct a new `Loop` object
         * 
         * @param loopType Token type for the type of this loop
         */
        Loop(TokenType loopType);

        /**
         * @brief Identify and parse inner tokens
         */
        void ParseTokens(TokenList& localTokens, TokenList& globalTokens = Language::EmptyTokenList);
    };

    /**
     * @brief Set the type of a loop by identifying it from a token
     * 
     * @param loopIdentifier The token that will be used to identify the loop type
     * @param loop The loop object that will get the identified type
     */
    void LoopTypeFromToken(SourceToken& loopIdentifier, std::shared_ptr<Loop> loop);

}

#endif