#include "../TokenParser.h"
#include "../StringUtil.h"
#include "../Logging.h"
#include "Expression.h"
#include <algorithm>

namespace Language
{

    /**
     * @brief Get the string representation of an expression type
     * 
     * @param type The expression type that will be "stringified"
     * 
     * @return The string representation of `type`
     */
    const char* ExpressionTypeToString(ExpressionType type)
    {
        switch (type)
        {
            default:
            case ExpressionType::Invalid:
                return "Invalid";

            case ExpressionType::Condition:
                return "Condition";

            case ExpressionType::Assignment:
                return "Assignment";

            case ExpressionType::FunctionCall:
                return "FunctionCall";

            case ExpressionType::ReturnStatement:
                return "ReturnStatement";

            case ExpressionType::KeywordExpression:
                return "KeywordExpression";

            case ExpressionType::ArithmeticOperation:
                return "ArithmeticOperation";
        }
    }

    /**
     * @brief Construct a new `Expression` object
     */
    Expression::Expression()
    {
        mTokenType = TokenType::Expression;
        Type = ExpressionType::Invalid;
        ReturnValue = "";;
        ReturnType = VariableType::Invalid;
        ReturnVariableName = "";
    }

    /**
     * @brief Identify the type of this expression
     */
    void Expression::Identify(TokenList& localTokens, TokenList& globalTokens)
    {
        ParseTokens(mInnerTokens, localTokens, globalTokens);
        switch (Type)
        {
            case ExpressionType::Assignment:
            {
                break;
            }

            case ExpressionType::FunctionCall:
            {
                break;
            }

            case ExpressionType::Invalid:
            case ExpressionType::ReturnStatement:
                return;

            case ExpressionType::KeywordExpression:
            {
                break;
            }

            case ExpressionType::ArithmeticOperation:
            {
                break;
            }
        }
    }

    void Expression::ParseTokens(SourceTokenList& tokens, TokenList& localTokens, TokenList& globalTokens)
    {
        if (tokens.empty())
            return;

        // if (CheckExpressionDividers(tokens))
        //     return;

        auto& firstToken = tokens.front();

        if (firstToken.mType == SourceToken::Type::Text)
        {
            auto& nextToken = tokens.at(1);
            auto& nextNextToken = tokens.at(2);

            if ((nextToken.mType == SourceToken::Type::EqualSign        && nextNextToken.mType == SourceToken::Type::EqualSign) || 
                (nextToken.mType == SourceToken::Type::ExclamationPoint && nextNextToken.mType == SourceToken::Type::EqualSign) || 
                (nextToken.mType == SourceToken::Type::LessThan         && nextNextToken.mType == SourceToken::Type::EqualSign) || 
                (nextToken.mType == SourceToken::Type::GreaterThan      && nextNextToken.mType == SourceToken::Type::EqualSign) || 
                (nextToken.mType == SourceToken::Type::LessThan         && nextNextToken.mType != SourceToken::Type::EqualSign) ||
                (nextToken.mType == SourceToken::Type::GreaterThan      && nextNextToken.mType != SourceToken::Type::EqualSign))
            {
                Type = ExpressionType::Condition;
                ParseCondition(tokens, localTokens, globalTokens);
                return;
            }

            if (nextToken.mType == SourceToken::Type::ParenthesisOpen)
            {
                Type = ExpressionType::FunctionCall;
                ParseFunctionCall(tokens, localTokens, globalTokens);
                return;
            }

            if (nextToken.mType != SourceToken::Type::EqualSign)
                return;

            Type = ExpressionType::Assignment;
            ParseAssignment(tokens, localTokens, globalTokens);
            return;
        }

        if (firstToken.mType == SourceToken::Type::Keyword)
        {
            if (firstToken.KeywordIndex == KeywordIndex_return)
            {
                Type = ExpressionType::ReturnStatement;
                ParseReturnStatement(tokens, localTokens, globalTokens);
                return;
            }

            if (tokens.size() < 3)
            {
                if (firstToken.KeywordIndex >= KeywordIndex_instantiate && firstToken.KeywordIndex <= KeywordIndex_detach)
                {
                    SourceToken objectToken;

                    objectToken = tokens.at(1);

                    auto globalNameIterator = std::find_if(globalTokens.begin(), globalTokens.end(), [objectToken](std::shared_ptr<Token>& token)
                    {
                        return token->mName == objectToken.Contents;
                    });
                    auto nameIterator = std::find_if(localTokens.begin(), localTokens.end(), [objectToken](std::shared_ptr<Token>& token)
                    {
                        return token->mName == objectToken.Contents;
                    });
                    if (nameIterator == localTokens.end() && globalNameIterator == globalTokens.end() && (objectToken.Contents == "null" || objectToken.Contents == "ATTACHED"))
                    {
                        Logging::Log(stringf("Line %d: Cannot parse keyword expression: Cannot find variable named '%s'", firstToken.LineNumber, objectToken.Contents), Logging::Level::Error);
                        Logging::Dispose();
                        exit(-1);
                    }

                    Type = ExpressionType::KeywordExpression;
                }
            }
        }
    }

    /**
     * @brief Check if a token list contains a token with the specified type and return it's index
     * 
     * @param tokens The list of tokens
     * @param tokenType The desired token type
     * 
     * @returns The index of found token, -1 if not found
     */
    int SourceTokenListContains(SourceTokenList& tokens, SourceToken::Type tokenType)
    {
        if (tokens.empty())
            return -1;

        auto iterator = std::find_if(tokens.begin(), tokens.end(), [tokenType](SourceToken& a){
            return a.mType == tokenType;
        });

        if (iterator == tokens.end())
            return -1;
        
        return iterator - tokens.begin();
    }

    using SourceTokenSequence = std::vector<SourceToken::Type>;

    /**
     * @brief Check if a specific sequence of tokens are within a list of tokens and return it's starting index
     * 
     * @param tokens The list of tokens
     * @param tokenSequence The desired sequence of tokens
     * 
     * @returns The index of the start of found sequence, -1 if not found
     */
    int SourceTokenListContainsSequence(SourceTokenList& tokens, SourceTokenSequence tokenSequence)
    {
        if (tokens.empty() || tokenSequence.empty())
            return -1;

        auto firstTokenType = tokenSequence.at(0);
        
        if (tokenSequence.size() < 2)
            return SourceTokenListContains(tokens, firstTokenType);

        auto iterator = tokens.begin();
        while ((iterator = std::find_if(tokens.begin(), tokens.end(), [firstTokenType](SourceToken& a){
            return a.mType == firstTokenType;
        })) != tokens.end())
        {
            
        }

        if (iterator == tokens.end())
            return -1;
        
        return iterator - tokens.begin();
    }

    /**
     * @brief Check if the provided list contains any tokens that split up expressions
     * 
     * @param tokens The list of tokens
     * 
     * @returns `true` if there are splitting tokens, `false` if there aren't
     */
    bool Expression::CheckExpressionDividers(SourceTokenList& tokens)
    {
        int orIndex = SourceTokenListContains(tokens, SourceToken::Type::Or);
        int andIndex = SourceTokenListContains(tokens, SourceToken::Type::And);

        if (orIndex != -1 && orIndex + 1 < tokens.size())
            orIndex = tokens.at(orIndex+1).mType == SourceToken::Type::Or ? orIndex : -1;

        if (andIndex != -1 && andIndex + 1 < tokens.size())
            andIndex = tokens.at(andIndex+1).mType == SourceToken::Type::Or ? andIndex : -1;

        ///
        /// TODO: implement more, including dividing and parsing each part of the expression
        ///       combining them according to the divider type
        ///

        return orIndex == -1 && andIndex == -1;
    }

    void Expression::ParseCondition(SourceTokenList& tokens, TokenList& localTokens, TokenList& globalTokens)
    {
    }

    void Expression::ParseAssignment(SourceTokenList& tokens, TokenList& localTokens, TokenList& globalTokens)
    {
    }

    void Expression::ParseFunctionCall(SourceTokenList& tokens, TokenList& localTokens, TokenList& globalTokens)
    {
    }

    void Expression::ParseReturnStatement(SourceTokenList& tokens, TokenList& localTokens, TokenList& globalTokens)
    {
        if (tokens.size() < 2)
        {
            ReturnType = VariableType::Void;
            ReturnValue = "";
            return;
        }

        SourceTokenList returnValueList = SourceTokenList(tokens.begin()+1, tokens.end());
        auto& valueToken = returnValueList.front();
        ReturnType = Variable::PredictType(returnValueList);

        if (ReturnType != VariableType::Invalid && ReturnType != VariableType::Void)
        {
            ReturnValue = TokenParser::ParseVariableValue(returnValueList, ReturnType);
            return;
        }

        if (ReturnType == VariableType::Void)
        {
            ReturnValue = "";
            return;
        }

        if (returnValueList.size() > 1)
        {
            Expression returnValueExpression = Expression();
            returnValueExpression.mInnerTokens = SourceTokenList(returnValueList.begin(), returnValueList.end());
            returnValueExpression.Identify(localTokens, globalTokens);
            mInnerExpressions.push_back(returnValueExpression);
            return;
        }

        auto globalNameIterator = std::find_if(globalTokens.begin(), globalTokens.end(), [valueToken](std::shared_ptr<Token>& token)
        {
            return token->mName == valueToken.Contents;
        });
        auto nameIterator = std::find_if(localTokens.begin(), localTokens.end(), [valueToken](std::shared_ptr<Token>& token)
        {
            return token->mName == valueToken.Contents;
        });
        if (nameIterator == localTokens.end() && globalNameIterator == globalTokens.end())
            Logging::LogErrorExit(stringf("Line %d: Invalid return value \"%s\"", valueToken.LineNumber, valueToken.Contents.c_str()), -1);

        std::shared_ptr<Token> token;
        if (globalNameIterator != globalTokens.end())
            token = *globalNameIterator;
        else
            token = *nameIterator;

        ReturnVariableName = std::string(token->mName);

        if (token->mTokenType == TokenType::Field)
        {
            auto field = std::static_pointer_cast<Field, Token>(token);
            ReturnType = VariableType(field->Type);
            return;
        }

        if (token->mTokenType == TokenType::Variable)
        {
            auto variable = std::static_pointer_cast<Variable, Token>(token);
            ReturnType = VariableType(variable->Type);
            return;
        }

        Logging::LogErrorExit(stringf("Line %d: Invalid return value \"%s\"", valueToken.LineNumber, valueToken.Contents.c_str()),  -1);
    }

    void Expression::ParseKeywordExpression(SourceTokenList& tokens, TokenList& localTokens, TokenList& globalTokens)
    {
    }

    void Expression::ParseArithmeticOperation(SourceTokenList& tokens, TokenList& localTokens, TokenList& globalTokens)
    {
    }

}