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
        ParseTokens(localTokens, globalTokens);
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

    /**
     * @returns The string representation of this `Expression`
     */
    std::string Expression::ToString()
    {
        return std::string(stringf("Expression{ Name{'%s'} InnerTokenCount{%ld} Type{'%s'} ReturnType{'%s'} ReturnValue{'%s'} ReturnVariableName{'%s'} }",
                                   mName.c_str(), mInnerTokens.size(), ExpressionTypeToString(Type), VariableTypeToString(ReturnType), ReturnValue.c_str(), ReturnVariableName.c_str()));
    }

    void Expression::ParseTokens(TokenList& localTokens, TokenList& globalTokens)
    {
        if (mInnerTokens.empty())
            return;

        auto& firstToken = mInnerTokens.front();

        if (firstToken.mType == SourceToken::Type::Text)
        {
            auto& nextToken = mInnerTokens.at(1);
            auto& nextNextToken = mInnerTokens.at(2);

            if ((nextToken.mType == SourceToken::Type::EqualSign        && nextNextToken.mType == SourceToken::Type::EqualSign) || 
                (nextToken.mType == SourceToken::Type::ExclamationPoint && nextNextToken.mType == SourceToken::Type::EqualSign) || 
                (nextToken.mType == SourceToken::Type::LessThan         && nextNextToken.mType == SourceToken::Type::EqualSign) || 
                (nextToken.mType == SourceToken::Type::GreaterThan      && nextNextToken.mType == SourceToken::Type::EqualSign) || 
                (nextToken.mType == SourceToken::Type::LessThan         && nextNextToken.mType != SourceToken::Type::EqualSign) ||
                (nextToken.mType == SourceToken::Type::GreaterThan      && nextNextToken.mType != SourceToken::Type::EqualSign))
            {
                Type = ExpressionType::Condition;
                ParseCondition(localTokens, globalTokens);
                return;
            }

            if (nextToken.mType == SourceToken::Type::ParenthesisOpen)
            {
                Type = ExpressionType::FunctionCall;
                ParseFunctionCall(localTokens, globalTokens);
                return;
            }

            if (nextToken.mType != SourceToken::Type::EqualSign)
                return;

            Type = ExpressionType::Assignment;
            ParseAssignment(localTokens, globalTokens);
            return;
        }

        if (firstToken.mType == SourceToken::Type::Keyword)
        {
            if (firstToken.KeywordIndex == KeywordIndex_return)
            {
                Type = ExpressionType::ReturnStatement;
                ParseReturnStatement(localTokens, globalTokens);
                return;
            }

            if (mInnerTokens.size() < 3)
            {
                if (firstToken.KeywordIndex >= KeywordIndex_instantiate && firstToken.KeywordIndex <= KeywordIndex_detach)
                {
                    SourceToken objectToken;

                    objectToken = mInnerTokens.at(1);

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

    void Expression::ParseCondition(TokenList& localTokens, TokenList& globalTokens)
    {
    }

    void Expression::ParseAssignment(TokenList& localTokens, TokenList& globalTokens)
    {
    }

    void Expression::ParseFunctionCall(TokenList& localTokens, TokenList& globalTokens)
    {
    }

    void Expression::ParseReturnStatement(TokenList& localTokens, TokenList& globalTokens)
    {
        if (mInnerTokens.size() < 2)
        {
            ReturnType = VariableType::Void;
            ReturnValue = "";
            return;
        }

        SourceTokenList returnValueList = SourceTokenList(mInnerTokens.begin()+1, mInnerTokens.end());
        auto& valueToken = returnValueList.front();
        ReturnType = Variable::PredictType(returnValueList);

        if (ReturnType != VariableType::Invalid && ReturnType != VariableType::Void)
        {
            ReturnValue = TokenParser::ParseVariableValue(returnValueList, ReturnType);
            Logging::Log(stringf("predict type '%s', value '%s'", VariableTypeToString(ReturnType), ReturnValue.c_str()));
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
        {
            Logging::Log(stringf("Line %d: Invalid return value \"%s\"", valueToken.LineNumber, valueToken.Contents.c_str()), Logging::Level::Error);
            Logging::Dispose();
            exit(-1);
        }

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

        Logging::Log(stringf("Line %d: Invalid return value \"%s\"", valueToken.LineNumber, valueToken.Contents.c_str()), Logging::Level::Error);
        Logging::Dispose();
        exit(-1);
    }

    void Expression::ParseKeywordExpression(TokenList& localTokens, TokenList& globalTokens)
    {
    }

    void Expression::ParseArithmeticOperation(TokenList& localTokens, TokenList& globalTokens)
    {
    }

}