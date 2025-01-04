#include "AbstractSyntaxTree.h"
#include "AST_TokenParsers.h"
#include "../StringUtil.h"
#include "../Logging.h"
#include "Language.h"
#include <algorithm>

#ifdef _WIN32
#   undef min
#   undef max
#endif

#define nop (void)0

namespace Language
{

    TreeNode::TreeNode()
        : contents(), operation(-1), specialChar(false), left(nullptr), right(nullptr)
    {
    }

    TreeNode::TreeNode(SourceToken& token, int op, bool spChr)
        : left(nullptr), right(nullptr)
    {
        contents = token;
        operation = op;
        specialChar = spChr;
    }

    bool TreeNode::Valid()
    {
        return contents != SourceToken::empty || operation != -1;
    }

    void TreeNode::operator=(const Language::TreeNode& other)
    {
        contents = other.contents;
        operation = other.operation;
        specialChar = other.specialChar;
        left = other.left;
        right = other.right;
        subNodes = other.subNodes;
    }

    TreeNodeObject TreeNode::Create()
    {
        return std::make_shared<TreeNode>();
    }

    TreeNodeObject TreeNode::Create(SourceToken& token, int opIdx, bool spChr)
    {
        return std::make_shared<TreeNode>(token, opIdx, spChr);
    }

    PreInPostFixTokenParser::PreInPostFixTokenParser()
    {
        Handle = nullptr;
        Power = -1;
    }

    PreInPostFixTokenParser::PreInPostFixTokenParser(power_t power, PreInPostFixHandle handle)
    {
        Handle = handle;
        Power = power;
    }

    PreInPostFixTokenParser::PreInPostFixTokenParser(BindingPower power, PreInPostFixHandle handle)
    {
        Handle = handle;
        Power = (power_t)power;
    }

    AbstractSyntaxTree::AbstractSyntaxTree()
        : mRoot(nullptr)
    {
        SetupParserHandles(*this);
    }

    TreeNodeObject AbstractSyntaxTree::GetRoot()
    {
        return mRoot;
    }

    void AbstractSyntaxTree::Parse(SourceTokenIterator& tokens)
    {
        mRoot = ParseTokens(tokens, 0);
    }

    TreeNodeObject AbstractSyntaxTree::ParseTokens(SourceTokenIterator& tokens, BindingPower minPower)
    {
        return ParseTokens(tokens, (power_t)minPower);
    }

    TreeNodeObject AbstractSyntaxTree::ParseTokens(SourceTokenIterator& tokens, power_t minPower)
    {
        if (tokens.EndOfList())
            Logging::LogErrorExit("Invalid expression, quit before expression was finished");

        auto left = TreeNode::Create();
        auto token = tokens.Next();

        auto prefixIterator = mPrefixLookup.find(token.mType);
        prefixIterator == mPrefixLookup.end() ? Logging::LogErrorExit(stringf("Invalid prefix '%s'", token.Contents.c_str())) : nop;

        auto tokenParser = prefixIterator->second;
        if (tokenParser.Handle != nullptr)
            left = tokenParser.Handle(tokens, left, tokenParser.Power);

        while (minPower < GetNextPrecedence(tokens))
        {
            token = tokens.Next();

            tokenParser = mInfixLookup[token.mType];
            if (tokenParser.Handle != nullptr)
                left = tokenParser.Handle(tokens, left, tokenParser.Power);
        }

        return left;
    }

    void AbstractSyntaxTree::RegisterPrefix(SourceToken::Type tokenType, PreInPostFixHandle handle)
    {
        mPrefixLookup[tokenType] = PreInPostFixTokenParser(BindingPower::None, handle);
    }

    void AbstractSyntaxTree::RegisterInfix(SourceToken::Type tokenType, BindingPower power, PreInPostFixHandle handle)
    {
        mInfixLookup[tokenType] = PreInPostFixTokenParser(power, handle);
    }

    void AbstractSyntaxTree::RegisterPostfix(SourceToken::Type tokenType, BindingPower power, PreInPostFixHandle handle)
    {
        RegisterInfix(tokenType, power, handle);
    }

    int64_t AbstractSyntaxTree::GetOperationIndex(const SourceToken& token, SourceToken::Type& operationType, bool& wasSpecialChar)
    {
        return GetOperationIndex((SourceToken&)token, operationType, wasSpecialChar);
    }

    int64_t AbstractSyntaxTree::GetOperationIndex(SourceToken& token, SourceToken::Type& operationType, bool& wasSpecialChar)
    {
        auto operationIterator = std::find_if(Language::SpecialOperators.begin(), Language::SpecialOperators.end(), [token](Language::SpecialOperator& a)
        {
            return a.TokenType == token.mType;
        });
        auto specialCharIterator = std::find_if(Language::SpecialOperationCharacters.begin(), Language::SpecialOperationCharacters.end(), [token](Language::SpecialCharacter& a)
        {
            return a.TokenType == token.mType;
        });

        int64_t operationIndex = -1;
        int64_t specialCharIndex = -1;
        operationType = SourceToken::Type::Invalid;

        if (operationIterator != Language::SpecialOperators.end())
        {
            operationType = (*operationIterator).TokenType;
            operationIndex = operationIterator - Language::SpecialOperators.begin();
        }
        if (specialCharIterator != Language::SpecialOperationCharacters.end())
        {
            operationType = (*specialCharIterator).TokenType;
            specialCharIndex = specialCharIterator - Language::SpecialOperationCharacters.begin();
        }
        wasSpecialChar = specialCharIndex != -1;

        return std::max(operationIndex, specialCharIndex);
    }

    power_t AbstractSyntaxTree::GetNextPrecedence(SourceTokenIterator& tokens)
    {
        auto token = tokens.Peek();
        if (token == SourceToken::empty)
            return 0;

        auto infixIterator = mInfixLookup.find(token.mType);
        if (infixIterator == mInfixLookup.end())
            return 0;

        return infixIterator->second.Power;
    }

}
