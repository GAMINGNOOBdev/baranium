#include "../StringUtil.h"
#include "Variable.h"

namespace Language
{

    const char* VariableTypeToString(VariableType type)
    {
        switch (type)
        {
            default:
            case VariableType::Invalid:
                return "Invalid";
            
            case VariableType::Void:
                return "Void";

            case VariableType::Object:
                return "Object";

            case VariableType::String:
                return "String";

            case VariableType::Float:
                return "Float";

            case VariableType::Bool:
                return "Bool";

            case VariableType::Int:
                return "Int";

            case VariableType::Uint:
                return "Uint";
        }
    }

    int8_t VariableTypeBytes(VariableType type)
    {
        switch (type)
        {
            default:
            case VariableType::Void:
            case VariableType::Invalid:
                return 0; // zero means that nothing is being stored here

            case VariableType::Object:
                return sizeof(int64_t); // int64_t internally

            case VariableType::String:
                return -1;// -1 should inform about storing a string, therefore it could have any length

            case VariableType::Float:
                return sizeof(float); // floats are still 32-bit, we can change to using double-like 64-bits later

            case VariableType::Bool:
                return sizeof(bool); // just use a simple single-byte integer

            case VariableType::Int:
            case VariableType::Uint:
                return sizeof(uint32_t); // default 32-bit integer
        }
    }

    VariableType Variable::TypeFromToken(SourceToken& token)
    {
        if (!IsInternalType(token))
        {
            return VariableType::Invalid;
        }

        return (VariableType)( (int)VariableType::Object + token.KeywordIndex - (KEYWORDS_TYPES_START) );
    }

    VariableType Variable::PredictType(SourceTokenList& tokens)
    {
        int tokenIndex = 0;

        auto& firstToken = tokens.at(tokenIndex++);

        if (firstToken.KeywordIndex == KeywordIndex_true || firstToken.KeywordIndex == KeywordIndex_false)
            return VariableType::Bool;

        if (firstToken.mType == SourceToken::Type::DoubleQuote)
            return VariableType::String;

        if (firstToken.mType == SourceToken::Type::Minus || firstToken.mType == SourceToken::Type::Plus)
        {
            if (tokenIndex >= tokens.size())
                return VariableType::Invalid;

            auto& secondToken = tokens.at(tokenIndex++);
            if (secondToken.mType == SourceToken::Type::Number)
            {
                if (tokenIndex+1 < tokens.size())
                {
                    auto& thirdToken = tokens.at(tokenIndex++);
                    if (thirdToken.mType == SourceToken::Type::Dot)
                        return VariableType::Float;
                }
                return firstToken.mType == SourceToken::Type::Minus ? VariableType::Int : VariableType::Uint;
            }
        }

        if (firstToken.mType == SourceToken::Type::Number)
        {
            if (tokenIndex >= tokens.size())
                return VariableType::Uint;
            
            auto& secondToken = tokens.at(tokenIndex++);
            if (secondToken.mType == SourceToken::Type::Dot)
                return VariableType::Float;
        }

        if (firstToken.mType == SourceToken::Type::Null)
            return VariableType::Void;

        return VariableType::Invalid;
    }

    Variable::Variable()
        : Token()
    {
        mTokenType = TokenType::Variable;
        AssignID();
    }

    Variable::Variable(std::shared_ptr<Variable> object)
        : Token()
    {
        mTokenType = TokenType::Variable;
        mName = object->mName;
        Value = object->Value;
        Type = object->Type;
        AssignID();
    }

}