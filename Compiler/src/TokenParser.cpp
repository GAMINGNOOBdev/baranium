#include "Language/Language.h"
#include "TokenParser.h"
#include "StringUtil.h"
#include "Logging.h"
#include <algorithm>

TokenParser::TokenParser()
{
}

void TokenParser::WriteTokensToJson(std::string name)
{
    BgeFile outputFile = BgeFile(name, true);
    outputFile.WriteLine("[");
    WriteTokens(outputFile, mPublicTokens);
    outputFile.WriteLine("]");
    outputFile.Close();
}

void TokenParser::ParseTokens(SourceTokenIterator& tokenIterator)
{
    ParseTokens(tokenIterator.GetTokens());
}

void TokenParser::ParseTokens(SourceTokenList& tokens)
{
    int index = 0;

    for (; index+1 < tokens.size(); index++)
    {
        auto& token = tokens.at(index);

        if (token.KeywordIndex == KeywordIndex_define)
        {
            ReadFunction(index, token, tokens, mPublicTokens);
            continue;
        }

        if (token.KeywordIndex >= KeywordIndex_do && token.KeywordIndex <= KeywordIndex_while)
            Logging::LogErrorExit(stringf("Line %d: cannot have loops in the global scope", token.LineNumber));

        if (token.KeywordIndex == KeywordIndex_if)
            Logging::LogErrorExit(stringf("Line %d: cannot have `if` in the global scope", token.LineNumber));

        if (token.KeywordIndex == KeywordIndex_else)
            Logging::LogErrorExit(stringf("Line %d: cannot have `else` in the global scope", token.LineNumber));

        if (token.mType == SourceToken::Type::Field)
        {
            ReadField(index, token, tokens, mPublicTokens);
            continue;
        }

        if (Language::IsInternalType(token))
        {
            ReadVariable(index, token, tokens, mPublicTokens);
            continue;
        }

        Logging::LogErrorExit(stringf("Line %d: cannot have expressions in the global scope", token.LineNumber));
    }
}

TokenList& TokenParser::GetTokens()
{
    return mPublicTokens;
}

void TokenParser::ReadVariable(int& index, SourceToken& current, SourceTokenList tokens, TokenList& output, TokenList& globalTokens)
{
    std::shared_ptr<Language::Variable> variable = std::make_shared<Language::Variable>();
    SourceTokenList valueTokens = SourceTokenList();
    variable->Type = Language::Variable::TypeFromToken(current);

    if (variable->Type == Language::VariableType::Invalid)
        Logging::LogErrorExit(stringf("Line %d: Inalid variable type '%s'", current.LineNumber, current.Contents.c_str()));

    index++;
    auto& nameToken = tokens.at(index);
    if (nameToken.mType != SourceToken::Type::Text)
        Logging::LogErrorExit(stringf("Line %d: No valid name has been prodived for variable", nameToken.LineNumber));

    if (TokensListContains(nameToken.Contents, output, globalTokens) != nullptr)
        Logging::LogErrorExit(stringf("Line %d: Name \"%s\" is already occupied", nameToken.LineNumber, nameToken.Contents.c_str()));

    variable->mName = std::string(nameToken.Contents);
    
    index++;
    auto& nextToken = tokens.at(index);
    if (nextToken.mType == SourceToken::Type::Semicolon)
    {
        variable->Value = "";
        goto end;
    }
    else if (nextToken.mType != SourceToken::Type::EqualSign)
        Logging::LogErrorExit(stringf("Line %d: Invalid syntax for variable definition/assignment", nextToken.LineNumber));

    while (nextToken.mType != SourceToken::Type::Semicolon && index+1 < tokens.size())
    {
        index++;
        nextToken = tokens.at(index);
        valueTokens.push_back(nextToken);
    }

    // to remove the last pushed semicolon
    valueTokens.pop_back();

    variable->Value = ParseVariableValue(valueTokens, variable->Type);

end:

    output.push_back(variable);
}

void TokenParser::ReadField(int& index, SourceToken& current, SourceTokenList tokens, TokenList& output, TokenList& globalTokens)
{
    index++;
    auto& typeToken = tokens.at(index);
    std::shared_ptr<Language::Field> field = std::make_shared<Language::Field>();
    SourceTokenList valueTokens = SourceTokenList();
    field->Type = Language::Variable::TypeFromToken(typeToken);
    index++;
    auto& nameToken = tokens.at(index);
    if (nameToken.mType != SourceToken::Type::Text)
        Logging::LogErrorExit(stringf("Line %d: No valid name has been prodived for field", nameToken.LineNumber));

    if (TokensListContains(nameToken.Contents, output, globalTokens) != nullptr)
        Logging::LogErrorExit(stringf("Line %d: Name \"%s\" is already occupied", nameToken.LineNumber, nameToken.Contents.c_str()));

    field->mName = std::string(nameToken.Contents);
    
    index++;
    auto& nextToken = tokens.at(index);
    if (nextToken.mType == SourceToken::Type::Semicolon)
    {
        field->Value = "";
        goto end;
    }
    else if (nextToken.mType != SourceToken::Type::EqualSign)
        Logging::LogErrorExit(stringf("Line %d: Invalid syntax for field definition/assignment", nextToken.LineNumber));

    while (nextToken.mType != SourceToken::Type::Semicolon && index+1 < tokens.size())
    {
        index++;
        nextToken = tokens.at(index);
        valueTokens.push_back(nextToken);
    }

    // to remove the last pushed semicolon
    valueTokens.pop_back();
    
    field->Value = ParseVariableValue(valueTokens, field->Type);

end:

    output.push_back(field);
}

bool TokenParser::ReadExpression(int& index, SourceToken& current, SourceTokenList tokens, TokenList& output, TokenList& globalTokens)
{
    std::shared_ptr<Language::Expression> expression = std::make_shared<Language::Expression>();

    auto& nextToken = tokens.at(index);
    expression->mInnerTokens.push_back(nextToken);
    if (!ReadContentUsingDepth(index, SourceToken::Type::Invalid, SourceToken::Type::Semicolon, tokens, expression->mInnerTokens))
        Logging::LogErrorExit(stringf("Line %d: Expected a ';' at the end", nextToken.LineNumber));

    expression->Identify(output, globalTokens);

    output.push_back(expression);

    return expression->Type == Language::ExpressionType::ReturnStatement;
}

void TokenParser::ReadFunction(int& index, SourceToken& current, SourceTokenList tokens, TokenList& output, TokenList& globalTokens)
{
    std::shared_ptr<Language::Function> function = std::make_shared<Language::Function>();

    index++;
    auto& nameToken = tokens.at(index);
    if (nameToken.mType != SourceToken::Type::Text)
        Logging::LogErrorExit(stringf("Line %d: No valid name has been prodived for function", nameToken.LineNumber));

    if (TokensListContains(nameToken.Contents, output, globalTokens) != nullptr)
        Logging::LogErrorExit(stringf("Line %d: Name \"%s\" is already occupied", nameToken.LineNumber, nameToken.Contents.c_str()));

    function->mName = std::string(nameToken.Contents);

    index++;
    auto& parametersStart = tokens.at(index);
    if (parametersStart.mType != SourceToken::Type::ParenthesisOpen)
        Logging::LogErrorExit(stringf("Line %d: Invalid function syntax", parametersStart.LineNumber));

    index++;
    auto& parameter = tokens.at(index);
    if (parameter.mType == SourceToken::Type::ParenthesisClose)
    {
        goto functionReadContents;
    }
    if (!Language::IsInternalType(parameter))
        Logging::LogErrorExit(stringf("Line %d: Invalid function parameter type", parameter.LineNumber));

    while (parameter.mType != SourceToken::Type::ParenthesisClose && index+1 < tokens.size())
    {
        ReadFunctionParameter(index, function, parameter, tokens);
    }

functionReadContents:

    index++;
    auto& functionContents = tokens.at(index);
    if (functionContents.mType != SourceToken::Type::CurlyBracketOpen)
        Logging::LogErrorExit(stringf("Line %d: Invalid function syntax", functionContents.LineNumber));

    if (!ReadContentUsingDepth(index, SourceToken::Type::CurlyBracketOpen, SourceToken::Type::CurlyBracketClose, tokens, function->mInnerTokens))
        Logging::LogErrorExit(stringf("Line %d: Invalid contents for function contents, invalid content depth", functionContents.LineNumber));

    function->ParseTokens(output);

    output.push_back(function);
}

void TokenParser::ReadIfStatement(int& index, SourceToken& current, SourceTokenList tokens, TokenList& output, TokenList& globalTokens)
{
    index++;
    auto& conditionStart = tokens.at(index);
    if (conditionStart.mType != SourceToken::Type::ParenthesisOpen)
        Logging::LogErrorExit(stringf("Line %d: Invalid start of if-statement, expected '(', got '%s'", conditionStart.LineNumber, conditionStart.Contents));
    std::shared_ptr<Language::IfElseStatement> ifElseStatement = std::make_shared<Language::IfElseStatement>();
    Language::IfElseStatement alternativeCondition = Language::IfElseStatement();
    Language::IfElseStatement elseStatement = Language::IfElseStatement();

    index++;
    auto& nextToken = tokens.at(index);
    ifElseStatement->Condition.mInnerTokens.push_back(nextToken);
    if (!ReadContentUsingDepth(index, SourceToken::Type::ParenthesisOpen, SourceToken::Type::ParenthesisClose, tokens, ifElseStatement->Condition.mInnerTokens))
        Logging::LogErrorExit(stringf("Line %d: Invalid condition contents for if-statement", nextToken.LineNumber));

    ifElseStatement->Condition.Identify(output, globalTokens);

    index++;
    nextToken = tokens.at(index);
    if (nextToken.mType == SourceToken::Type::CurlyBracketOpen)
        goto readStatementContents;

    ifElseStatement->mInnerTokens.push_back(nextToken);
    while (nextToken.mType != SourceToken::Type::Semicolon && index+1 < tokens.size())
    {
        index++;
        nextToken = tokens.at(index);
        ifElseStatement->mInnerTokens.push_back(nextToken);
    }

    goto readNextPart;

readStatementContents:
    if (!ReadContentUsingDepth(index, SourceToken::Type::CurlyBracketOpen, SourceToken::Type::CurlyBracketClose, tokens, ifElseStatement->mInnerTokens))
    {
        nextToken = tokens.at(index);
        Logging::LogErrorExit(stringf("Line %d: Invalid contents for if-statement, invalid content depth", nextToken.LineNumber));
    }

    goto readNextPart;

readNextPart:
    if (index+1 < tokens.size())
    {
        if (tokens.at(index+1).KeywordIndex == KeywordIndex_else)
        {
            index++;
            goto readElse;
        }
    }

    goto end;

readAlternativeConditions:
    index++;
    nextToken = tokens.at(index);
    if (nextToken.mType != SourceToken::Type::ParenthesisOpen)
        Logging::LogErrorExit(stringf("Line %d: Invalid start of if-statement, expected '(', got '%s'", conditionStart.LineNumber, conditionStart.Contents));

    alternativeCondition = Language::IfElseStatement();
    if (!ReadContentUsingDepth(index, SourceToken::Type::ParenthesisOpen, SourceToken::Type::ParenthesisClose, tokens, alternativeCondition.Condition.mInnerTokens))
        Logging::LogErrorExit(stringf("Line %d: Invalid condition contents for if-statement", nextToken.LineNumber));

    alternativeCondition.Condition.Identify(output, globalTokens);

    index++;
    nextToken = tokens.at(index);
    if (nextToken.mType == SourceToken::Type::CurlyBracketOpen)
        goto readAlternaticeConditionContents;

    alternativeCondition.mInnerTokens.push_back(nextToken);
    while (nextToken.mType != SourceToken::Type::Semicolon && index+1 < tokens.size())
    {
        index++;
        nextToken = tokens.at(index);
        alternativeCondition.mInnerTokens.push_back(nextToken);
    }

    goto endAlternateCondition;

readAlternaticeConditionContents:
    if (!ReadContentUsingDepth(index, SourceToken::Type::CurlyBracketOpen, SourceToken::Type::CurlyBracketClose, tokens, alternativeCondition.mInnerTokens))
    {
        nextToken = tokens.at(index);
        Logging::LogErrorExit(stringf("Line %d: Invalid contents for else-if-statement, invalid content depth", nextToken.LineNumber));
    }
    alternativeCondition.ParseTokens(output, globalTokens);

    goto endAlternateCondition;

endAlternateCondition:
    ifElseStatement->ChainedStatements.push_back(Language::IfElseStatement(alternativeCondition));

    goto readNextPart;

readElse:
    index++;
    nextToken = tokens.at(index);
    if (nextToken.KeywordIndex == KeywordIndex_if)
        goto readAlternativeConditions;

    ifElseStatement->HasElseStatement = true;

    if (nextToken.mType == SourceToken::Type::CurlyBracketOpen)
        goto readElseContents;
    
    elseStatement.mInnerTokens.push_back(nextToken);
    while (nextToken.mType != SourceToken::Type::Semicolon && index+1 < tokens.size())
    {
        index++;
        nextToken = tokens.at(index);
        elseStatement.mInnerTokens.push_back(nextToken);
    }
    goto readElseEnd;

readElseContents:
    if (!ReadContentUsingDepth(index, SourceToken::Type::CurlyBracketOpen, SourceToken::Type::CurlyBracketClose, tokens, elseStatement.mInnerTokens))
    {
        nextToken = tokens.at(index);
        Logging::LogErrorExit(stringf("Line %d: Invalid contents for else-statement, invalid content depth", nextToken.LineNumber));
    }

    goto readElseEnd;

readElseEnd:

    elseStatement.ParseTokens(output, globalTokens);
    ifElseStatement->ChainedStatements.push_back(elseStatement);

end:
    ifElseStatement->ParseTokens(output, globalTokens);

    output.push_back(ifElseStatement);
}

void ReadDoWhileLoop(std::shared_ptr<Language::Loop> loop, int& index, SourceToken& current, SourceTokenList tokens, TokenList& output, TokenList& globalTokens);
void ReadForLoop(std::shared_ptr<Language::Loop> loop, int& index, SourceToken& current, SourceTokenList tokens, TokenList& output, TokenList& globalTokens);
void ReadWhileLoop(std::shared_ptr<Language::Loop> loop, int& index, SourceToken& current, SourceTokenList tokens, TokenList& output, TokenList& globalTokens);

void TokenParser::ReadLoop(int& index, SourceToken& current, SourceTokenList tokens, TokenList& output, TokenList& globalTokens)
{
    Language::TokenType type;
    auto loop = std::make_shared<Language::Loop>();
    Language::LoopTypeFromToken(current, loop);
    index++;

    if (current.KeywordIndex == KeywordIndex_do)
    {
        ReadDoWhileLoop(loop, index, current, tokens, output, globalTokens);
        loop->mTokenType = Language::TokenType::DoWhileLoop;
    }

    if (current.KeywordIndex == KeywordIndex_for)
    {
        ReadForLoop(loop, index, current, tokens, output, globalTokens);
        loop->mTokenType = Language::TokenType::ForLoop;
    }

    if (current.KeywordIndex == KeywordIndex_while)
    {
        ReadWhileLoop(loop, index, current, tokens, output, globalTokens);
        loop->mTokenType = Language::TokenType::WhileLoop;
    }
    
    output.push_back(loop);
}

void TokenParser::ReadFunctionParameter(int& index, std::shared_ptr<Language::Function> function, SourceToken& current, SourceTokenList tokens)
{
    std::shared_ptr<Language::Variable> parameter = std::make_shared<Language::Variable>();
    parameter->Type = Language::Variable::TypeFromToken(current);
    index++;
    auto& nameToken = tokens.at(index);
    if (nameToken.mType != SourceToken::Type::Text)
        Logging::LogErrorExit(stringf("Line %d: No valid name ('%s') has been prodived for parameter in function definition", nameToken.LineNumber, nameToken.Contents.c_str()));

    parameter->mName = std::string(nameToken.Contents);

    index++;
    auto& nextToken = tokens.at(index);
    if (nextToken.mType == SourceToken::Type::ParenthesisClose || nextToken.mType == SourceToken::Type::Comma)
    {
        parameter->Value = "";
        function->mParameters.push_back(parameter);

        current = nextToken;
        if (nextToken.mType == SourceToken::Type::Comma)
        {
            index++;
            auto& nextTypeToken = tokens.at(index);
            current = nextTypeToken;
        }
        return;
    }

    Logging::LogErrorExit(stringf("Line %d: Invalid syntax for parameter definition in function definition", nextToken.LineNumber));
}

std::string TokenParser::ParseVariableValue(SourceTokenList tokens, Language::VariableType varType)
{
    int tokenIndex = 0;

    if (varType == Language::VariableType::Invalid || tokens.empty())
        return "null";

    if (varType == Language::VariableType::GameObject)
    {
        auto& dataToken = tokens.at(tokenIndex);
        tokenIndex++;

        if (dataToken.KeywordIndex == KeywordIndex_attached)
            return "attached";

        if (dataToken.mType == SourceToken::Type::Text)
            return std::string(dataToken.Contents);

        if (dataToken.mType == SourceToken::Type::Null)
            return "null";

        Logging::LogErrorExit(stringf("Line %d: Invalid gameobject assignment", dataToken.LineNumber));
    }

    if (varType == Language::VariableType::String)
    {
        auto& stringStart = tokens.at(tokenIndex);

        if (stringStart.mType == SourceToken::Type::Null)
            return "";

        if (stringStart.mType != SourceToken::Type::DoubleQuote)
            Logging::LogErrorExit(stringf("Line %d: Invalid string assignment", stringStart.LineNumber));

        tokenIndex++;
        auto& contentsToken = tokens.at(tokenIndex);
        if (contentsToken.mType == SourceToken::Type::DoubleQuote && tokens.size() == 2)
            return "";

        tokenIndex++;
        auto& stringEnd = tokens.at(tokenIndex);
        tokenIndex++;

        if (stringEnd.mType != SourceToken::Type::DoubleQuote)
            Logging::LogErrorExit(stringf("Line %d: Invalid string assignment", contentsToken.LineNumber));

        return std::string(contentsToken.Contents);
    }

    if (varType == Language::VariableType::Float)
    {
        std::string value = "";

        auto& valueStart = tokens.at(tokenIndex);
        tokenIndex++;

        if (valueStart.mType == SourceToken::Type::Null)
            return "0";

        if (tokenIndex >= tokens.size())
        {
            if (valueStart.mType == SourceToken::Type::Number)
                return std::string(valueStart.Contents);
            
            Logging::LogErrorExit(stringf("Line %d: Invalid float assignment: Not a number", valueStart.LineNumber));
        }

        auto& dotPart = tokens.at(tokenIndex);
        tokenIndex++;

        if (valueStart.mType == SourceToken::Type::Number && dotPart.mType == SourceToken::Type::Dot)
        {
            value.append(valueStart.Contents);
            value.append(".");

            if (tokenIndex+1 < tokens.size())
            {
                auto& decimalPart = tokens.at(tokenIndex);
                if (decimalPart.mType != SourceToken::Type::Number)
                    Logging::LogErrorExit(stringf("Line %d: Invalid float assignment: Invalid literal after dot", decimalPart.LineNumber));
                value.append(decimalPart.Contents);
            }
            else
                value.append("0");
            
            return std::string(value);
        }
        if (valueStart.mType == SourceToken::Type::Dot && dotPart.mType == SourceToken::Type::Number)
        {
            value.append("0.");
            value.append(dotPart.Contents);

            return std::string(value);
        }

        Logging::LogErrorExit(stringf("Line %d: Invalid float assignment", valueStart.LineNumber));
    }

    if (varType == Language::VariableType::Bool)
    {
        if (tokens.size() > 1)
            Logging::LogErrorExit(stringf("Line %d: Invalid bool assignment: Too many initializing values", tokens.at(0).LineNumber));

        auto& value = tokens.at(0);
        if (value.KeywordIndex < KeywordIndex_true || value.KeywordIndex > KeywordIndex_false)
            Logging::LogErrorExit(stringf("Line %d: Invalid bool assignment: Invalid assignment value", value.LineNumber));

        return std::string(value.Contents);
    }

    if (varType == Language::VariableType::Uint ||
        varType == Language::VariableType::Int)
    {
        if (tokens.size() > 2)
            Logging::LogErrorExit(stringf("Line %d: Invalid Int assignment: Too many initializing values", tokens.at(0).LineNumber));

        if (tokens.size() == 2)
        {
            auto& signToken = tokens.at(0);
            auto& numberToken = tokens.at(1);

            if ((signToken.mType != SourceToken::Type::Plus && signToken.mType != SourceToken::Type::Minus) ||
                numberToken.mType != SourceToken::Type::Number)
                Logging::LogErrorExit(stringf("Line %d: Invalid Int assignment: Invalid value", signToken.LineNumber));

            return std::string(signToken.Contents).append(numberToken.Contents);
        }

        auto& numberToken = tokens.at(0);

        if (numberToken.mType == SourceToken::Type::Number)
            return std::string(numberToken.Contents);
        
        if (numberToken.mType == SourceToken::Type::Null)
            return "0";

        Logging::LogErrorExit(stringf("Line %d: Invalid Int assignment: Not a number", numberToken.LineNumber));
    }

    return "null";
}

bool TokenParser::ReadContentUsingDepth(int& index, SourceToken::Type startType, SourceToken::Type endType, SourceTokenList tokens, SourceTokenList& output)
{
    auto nextToken = SourceToken();
    int depth = 1;
    while (depth > 0 && index+1 < tokens.size())
    {
        index++;
        nextToken = tokens.at(index);

        if (nextToken.mType == startType)
            depth++;

        if (nextToken.mType == endType)
            depth--;

        output.push_back(nextToken);
    }

    // to remove the last pushed closing type token
    if (nextToken.mType == endType)
        output.pop_back();
    
    return depth == 0;
}

bool TokenParser::ReadContentUsingDepth(int& index, SourceToken::Type startType, SourceToken::Type endType, SourceTokenList tokens, SourceTokenIterator& output)
{
    auto nextToken = SourceToken();
    int depth = 1;
    while (depth > 0 && index+1 < tokens.size())
    {
        index++;
        nextToken = tokens.at(index);

        if (nextToken.mType == startType)
            depth++;

        if (nextToken.mType == endType)
            depth--;

        output.Push(nextToken);
    }

    // to remove the last pushed closing type token
    if (nextToken.mType == endType)
        output.Pop();
    
    return depth == 0;
}

void TokenParser::WriteTokens(BgeFile& outputFile, TokenList& tokenList, std::string indentation)
{
    int index = 0;
    int tokenCount = tokenList.size();
    for (auto& token : tokenList)
    {
        outputFile.WriteLine(stringf("%s\t{", indentation.c_str()));
        outputFile.WriteLine(stringf("%s\t\t\"token-type\": \"%s\",", indentation.c_str(), Language::TokenTypeToString(token->mTokenType)));

        switch (token->mTokenType)
        {
            case Language::TokenType::Expression:
            {
                auto tokenObject = std::static_pointer_cast<Language::Expression>(token);
                outputFile.WriteLine(stringf("%s\t\t\"name\": \"%s\",", indentation.c_str(), tokenObject->mName.c_str()));
                outputFile.WriteLine(stringf("%s\t\t\"expression-type\": \"%s\"", indentation.c_str(), Language::ExpressionTypeToString(tokenObject->Type)));
                break;
            }

            case Language::TokenType::Field:
            {
                auto tokenObject = std::static_pointer_cast<Language::Field>(token);
                outputFile.WriteLine(stringf("%s\t\t\"name\": \"%s\",", indentation.c_str(), tokenObject->mName.c_str()));
                outputFile.WriteLine(stringf("%s\t\t\"type\": \"%s\",", indentation.c_str(), Language::VariableTypeToString(tokenObject->Type)));
                outputFile.WriteLine(stringf("%s\t\t\"value\": \"%s\"", indentation.c_str(), tokenObject->Value.c_str()));
                break;
            }

            case Language::TokenType::Function:
            {
                auto tokenObject = std::static_pointer_cast<Language::Function>(token);
                outputFile.WriteLine(stringf("%s\t\t\"name\": \"%s\",", indentation.c_str(), tokenObject->mName.c_str()));
                outputFile.WriteLine(stringf("%s\t\t\"return-type\": \"%s\",", indentation.c_str(), Language::VariableTypeToString(tokenObject->ReturnType)));
                outputFile.WriteLine(stringf("%s\t\t\"return-value\": \"%s\",", indentation.c_str(), tokenObject->ReturnValue.c_str()));
                outputFile.WriteLine(stringf("%s\t\t\"return-variable-name\": \"%s\",", indentation.c_str(), tokenObject->ReturnVariableName.c_str()));
                outputFile.WriteLine(stringf("%s\t\t\"parameters\":", indentation.c_str()));
                outputFile.WriteLine(stringf("%s\t\t[", indentation.c_str()));
                int paramIndex = 0;
                int paramCount = tokenObject->mParameters.size();
                for (auto& parameter : tokenObject->mParameters)
                {
                    outputFile.WriteLine(stringf("%s\t\t\t{", indentation.c_str()));
                    outputFile.WriteLine(stringf("%s\t\t\t\t\"type\": \"%s\",", indentation.c_str(), Language::VariableTypeToString(parameter->Type)));
                    outputFile.WriteLine(stringf("%s\t\t\t\t\"name\": \"%s\"", indentation.c_str(), parameter->mName.c_str()));

                    if (paramIndex == paramCount-1)
                        outputFile.WriteLine(stringf("%s\t\t\t}", indentation.c_str()));
                    else
                        outputFile.WriteLine(stringf("%s\t\t\t},", indentation.c_str()));
                    
                    paramIndex++;
                }
                outputFile.WriteLine(stringf("%s\t\t],", indentation.c_str()));
                outputFile.WriteLine(stringf("%s\t\t\"tokens\":", indentation.c_str()));
                outputFile.WriteLine(stringf("%s\t\t[", indentation.c_str()));
                WriteTokens(outputFile, tokenObject->mTokens, stringf("%s\t\t", indentation.c_str()));
                outputFile.WriteLine(stringf("%s\t\t]", indentation.c_str()));
                break;
            }

            case Language::TokenType::ForLoop:
            {
                auto tokenObject = std::static_pointer_cast<Language::Loop>(token);
                outputFile.WriteLine(stringf("%s\t\t\"iteration\": \"idk\",", indentation.c_str()));
                outputFile.WriteLine(stringf("%s\t\t\"condition\": \"idk\",", indentation.c_str()));
                outputFile.WriteLine(stringf("%s\t\t\"variable\": \"idk\",", indentation.c_str()));
                outputFile.WriteLine(stringf("%s\t\t\"tokens\":", indentation.c_str()));
                outputFile.WriteLine(stringf("%s\t\t[", indentation.c_str()));
                WriteTokens(outputFile, tokenObject->mTokens, stringf("%s\t\t", indentation.c_str()));
                outputFile.WriteLine(stringf("%s\t\t]", indentation.c_str()));
                break;
            }

            case Language::TokenType::DoWhileLoop:
            {
                auto tokenObject = std::static_pointer_cast<Language::Loop>(token);
                outputFile.WriteLine(stringf("%s\t\t\"condition\": \"idk\",", indentation.c_str()));
                outputFile.WriteLine(stringf("%s\t\t\"tokens\":", indentation.c_str()));
                outputFile.WriteLine(stringf("%s\t\t[", indentation.c_str()));
                WriteTokens(outputFile, tokenObject->mTokens, stringf("%s\t\t", indentation.c_str()));
                outputFile.WriteLine(stringf("%s\t\t]", indentation.c_str()));
                break;
            }

            case Language::TokenType::WhileLoop:
            {
                auto tokenObject = std::static_pointer_cast<Language::Loop>(token);
                outputFile.WriteLine(stringf("%s\t\t\"condition\": \"idk\",", indentation.c_str()));
                outputFile.WriteLine(stringf("%s\t\t\"tokens\":", indentation.c_str()));
                outputFile.WriteLine(stringf("%s\t\t[", indentation.c_str()));
                WriteTokens(outputFile, tokenObject->mTokens, stringf("%s\t\t", indentation.c_str()));
                outputFile.WriteLine(stringf("%s\t\t]", indentation.c_str()));
                break;
            }

            case Language::TokenType::Variable:
            {
                auto tokenObject = std::static_pointer_cast<Language::Variable>(token);
                outputFile.WriteLine(stringf("%s\t\t\"name\": \"%s\",", indentation.c_str(), tokenObject->mName.c_str()));
                outputFile.WriteLine(stringf("%s\t\t\"type\": \"%s\",", indentation.c_str(), Language::VariableTypeToString(tokenObject->Type)));
                outputFile.WriteLine(stringf("%s\t\t\"value\": \"%s\"", indentation.c_str(), tokenObject->Value.c_str()));
                break;
            }

            case Language::TokenType::IfElseStatement:
            {
                auto tokenObject = std::static_pointer_cast<Language::IfElseStatement>(token);
                outputFile.WriteLine(stringf("%s\t\t\"hasElseStatement\": \"%s\",", indentation.c_str(), tokenObject->HasElseStatement ? "true" : "false"));
                outputFile.WriteLine(stringf("%s\t\t\"chainedStatementCount\": %d,", indentation.c_str(), tokenObject->ChainedStatements.size()));
                outputFile.WriteLine(stringf("%s\t\t\"tokens\":", indentation.c_str()));
                outputFile.WriteLine(stringf("%s\t\t[", indentation.c_str()));
                WriteTokens(outputFile, tokenObject->mTokens, stringf("%s\t\t", indentation.c_str()));
                outputFile.WriteLine(stringf("%s\t\t]", indentation.c_str()));
                break;
            }

            default:
            case Language::TokenType::Invalid:
                outputFile.WriteLine(stringf("%s\t\t\"invalid\": true", indentation.c_str()));
                break;
        }

        if (index == tokenCount-1)
            outputFile.WriteLine(stringf("%s\t}", indentation.c_str()));
        else
            outputFile.WriteLine(stringf("%s\t},", indentation.c_str()));

        index++;
    }
}

void ReadDoWhileLoop(std::shared_ptr<Language::Loop> loop, int& index, SourceToken& current, SourceTokenList tokens, TokenList& output, TokenList& globalTokens)
{
    auto nextToken = tokens.at(index);
    if (nextToken.mType != SourceToken::Type::CurlyBracketOpen)
        Logging::LogErrorExit(stringf("Line %d: Invalid do-while loop, unexpected literal '%s' expected '{'", nextToken.LineNumber, nextToken.Contents.c_str()));

    if (!TokenParser::ReadContentUsingDepth(index, SourceToken::Type::CurlyBracketOpen, SourceToken::Type::CurlyBracketClose, tokens, loop->mInnerTokens))
        Logging::LogErrorExit(stringf("Line %d: Invalid contents for do-while loop contents, invalid content depth", current.LineNumber));
    index++;

    nextToken = tokens.at(index);
    if (nextToken.KeywordIndex != KeywordIndex_while)
        Logging::LogErrorExit(stringf("Line %d: Invalid do-while loop, unexpected literal '%s' expected 'while'", nextToken.LineNumber, nextToken.Contents.c_str()));
    index++;

    nextToken = tokens.at(index);
    if (nextToken.mType != SourceToken::Type::ParenthesisOpen)
        Logging::LogErrorExit(stringf("Line %d: Invalid do-while loop, unexpected literal '%s' expected '('", nextToken.LineNumber, nextToken.Contents.c_str()));

    if (!TokenParser::ReadContentUsingDepth(index, SourceToken::Type::ParenthesisOpen, SourceToken::Type::ParenthesisClose, tokens, loop->Condition.mInnerTokens))
        Logging::LogErrorExit(stringf("Line %d: Invalid do-while loop, invalid depth at condition declaration", nextToken.LineNumber));
    index++;

    nextToken = tokens.at(index);
    if (nextToken.mType != SourceToken::Type::Semicolon)
        Logging::LogErrorExit(stringf("Line %d: Invalid do-while loop, expected ';'", nextToken.LineNumber));

    loop->Condition.Identify(output, globalTokens);
    loop->ParseTokens(output, globalTokens);
}

void ReadForLoop(std::shared_ptr<Language::Loop> loop, int& index, SourceToken& current, SourceTokenList tokens, TokenList& output, TokenList& globalTokens)
{
    auto loopInitializerTokens = SourceTokenList();
    auto loopVarTokens = SourceTokenList();

    auto nextToken = tokens.at(index);
    if (nextToken.mType != SourceToken::Type::ParenthesisOpen)
        Logging::LogErrorExit(stringf("Line %d: Invalid for loop, unexpected literal '%s' expected '('", nextToken.LineNumber, nextToken.Contents.c_str()));

    if (!TokenParser::ReadContentUsingDepth(index, SourceToken::Type::ParenthesisOpen, SourceToken::Type::ParenthesisClose, tokens, loopInitializerTokens))
        Logging::LogErrorExit(stringf("Line %d: Invalid for loop, invalid depth at declaration", nextToken.LineNumber));
    index++;

    nextToken = tokens.at(index);
    if (nextToken.mType == SourceToken::Type::CurlyBracketOpen)
    {
        if (!TokenParser::ReadContentUsingDepth(index, SourceToken::Type::CurlyBracketOpen, SourceToken::Type::CurlyBracketClose, tokens, loop->mInnerTokens))
            Logging::LogErrorExit(stringf("Line %d: Invalid contents for for loop contents, invalid content depth", current.LineNumber));

        goto parseLoopInitializers;
    }
    else if (nextToken.mType == SourceToken::Type::Semicolon)
        goto parseLoopInitializers;

    if (!TokenParser::ReadContentUsingDepth(index, SourceToken::Type::Invalid, SourceToken::Type::Semicolon, tokens, loop->mInnerTokens))
        Logging::LogErrorExit(stringf("Line %d: Invalid for loop, unexpected depth", nextToken.LineNumber));

parseLoopInitializers:

    if (loopInitializerTokens.empty())
        Logging::LogErrorExit(stringf("Line %d: Invalid for loop, unexpected depth", nextToken.LineNumber));

    int subIndex = 0;
    if (loopInitializerTokens.at(subIndex).mType != SourceToken::Type::Semicolon)
    {
        // because the "read content using depth" function increases the value
        // by 1 once the function starts, we have to decrease our index so
        // that it also adds the first token
        subIndex--;
        TokenParser::ReadContentUsingDepth(subIndex, SourceToken::Type::Invalid, SourceToken::Type::Semicolon, loopInitializerTokens, loopVarTokens);

        // because of the `ReadVariable` function we have to add a semicolon at the end of the tokens
        SourceToken finalizerToken = SourceToken();
        finalizerToken.Contents = ";";
        finalizerToken.KeywordIndex = -1;
        finalizerToken.LineNumber = current.LineNumber;
        finalizerToken.mType = SourceToken::Type::Semicolon;
        loopVarTokens.push_back(finalizerToken);
    }
    subIndex++;
    if (loopInitializerTokens.at(subIndex).mType != SourceToken::Type::Semicolon)
    {
        // because the "read content using depth" function increases the value
        // by 1 once the function starts, we have to decrease our index so
        // that it also adds the first token
        subIndex--;
        TokenParser::ReadContentUsingDepth(subIndex, SourceToken::Type::Invalid, SourceToken::Type::Semicolon, loopInitializerTokens, loop->Condition.mInnerTokens);
    }
    subIndex++;
    if (loopInitializerTokens.at(subIndex).mType != SourceToken::Type::Semicolon)
    {
        // because the "read content using depth" function increases the value
        // by 1 once the function starts, we have to decrease our index so
        // that it also adds the first token
        subIndex--;
        TokenParser::ReadContentUsingDepth(subIndex, SourceToken::Type::Invalid, SourceToken::Type::Semicolon, loopInitializerTokens, loop->Iteration.mInnerTokens);
    }

    if (!loopVarTokens.empty())
    {
        int outputSize = output.size();
        subIndex = 0;
        SourceToken varToken = loopVarTokens.at(subIndex);
        TokenParser::ReadVariable(subIndex, varToken, loopVarTokens, output, globalTokens);

        if (outputSize+1 == output.size())
        {
            loop->StartVariable = std::static_pointer_cast<Language::Variable>(output.at(outputSize));
            output.pop_back();
        }
    }

    loop->ParseTokens(output, globalTokens);
}

void ReadWhileLoop(std::shared_ptr<Language::Loop> loop, int& index, SourceToken& current, SourceTokenList tokens, TokenList& output, TokenList& globalTokens)
{
    auto nextToken = tokens.at(index);
    if (nextToken.mType != SourceToken::Type::ParenthesisOpen)
        Logging::LogErrorExit(stringf("Line %d: Invalid while loop, unexpected literal '%s' expected '('", nextToken.LineNumber, nextToken.Contents.c_str()));

    if (!TokenParser::ReadContentUsingDepth(index, SourceToken::Type::ParenthesisOpen, SourceToken::Type::ParenthesisClose, tokens, loop->Condition.mInnerTokens))
        Logging::LogErrorExit(stringf("Line %d: Invalid while loop, invalid depth at declaration", nextToken.LineNumber));
    index++;

    nextToken = tokens.at(index);
    if (nextToken.mType == SourceToken::Type::CurlyBracketOpen)
    {
        if (!TokenParser::ReadContentUsingDepth(index, SourceToken::Type::CurlyBracketOpen, SourceToken::Type::CurlyBracketClose, tokens, loop->mInnerTokens))
            Logging::LogErrorExit(stringf("Line %d: Invalid contents for while loop contents, invalid content depth", current.LineNumber));

        goto end;
    }
    else if (nextToken.mType == SourceToken::Type::Semicolon)
        goto end;

    if (!TokenParser::ReadContentUsingDepth(index, SourceToken::Type::Invalid, SourceToken::Type::Semicolon, tokens, loop->mInnerTokens))
        Logging::LogErrorExit(stringf("Line %d: Invalid while loop, unexpected depth", nextToken.LineNumber));

end:
    loop->ParseTokens(output, globalTokens);
}