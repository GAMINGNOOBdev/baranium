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
            
            case ExpressionType::None:
                return "None";

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
    }

    void Expression::ParseTokens(SourceTokenList& tokens, TokenList& localTokens, TokenList& globalTokens)
    {
        if (tokens.empty())
            return;

        SourceTokenIterator it = SourceTokenIterator();
        it.Push(tokens);
        mAST.Parse(it);
        it.Clear();

        TreeNodeObject rootNode = mAST.GetRoot();
        auto firstToken = rootNode->contents;

        switch (firstToken.mType)
        {
            default:
            case SourceToken::Type::Null:
            case SourceToken::Type::Number:
            case SourceToken::Type::EqualTo:
            case SourceToken::Type::NotEqual:
            case SourceToken::Type::LessEqual:
            case SourceToken::Type::GreaterEqual:
            {
                Type = ExpressionType::Condition;
            }

            case SourceToken::Type::EqualSign:
            case SourceToken::Type::AndEqual:
            case SourceToken::Type::OrEqual:
            case SourceToken::Type::DivEqual:
            case SourceToken::Type::ModEqual:
            case SourceToken::Type::MulEqual:
            case SourceToken::Type::XorEqual:
            case SourceToken::Type::PlusEqual:
            case SourceToken::Type::MinusEqual:
            {
                // check if variable exists
                Type = ExpressionType::Assignment;
                break;
            }

            case SourceToken::Type::Plus:
            case SourceToken::Type::PlusPlus:
            case SourceToken::Type::Minus:
            case SourceToken::Type::MinusMinus:
            case SourceToken::Type::Modulo:
            case SourceToken::Type::Asterisk:
            case SourceToken::Type::Slash:
            case SourceToken::Type::And:
            case SourceToken::Type::Or:
            case SourceToken::Type::Caret:
            {
                Type = ExpressionType::ArithmeticOperation;
                break;
            }

            case SourceToken::Type::Text:
            {
                Type = ExpressionType::Condition;

                if (!rootNode->subNodes.empty())
                    Type = ExpressionType::FunctionCall;

                break;
            }

            case SourceToken::Type::Keyword:
            {
                if (firstToken.KeywordIndex == KeywordIndex_return)
                {
                    Type = ExpressionType::ReturnStatement;
                    ParseReturnStatement(tokens, localTokens, globalTokens);
                    break;
                }

                if (tokens.size() < 3 && firstToken.KeywordIndex >= KeywordIndex_instantiate && firstToken.KeywordIndex <= KeywordIndex_detach)
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

                break;
            }
        }
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

}
