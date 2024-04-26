#include "AbstractSyntaxTree.h"
#include "../TokenParser.h"
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

    BindingPower::BindingPower()
    {
        left = -1;
        right = -1;
    }

    BindingPower::BindingPower(power_t l, power_t r)
    {
        left = l;
        right = r;
    }

    bool BindingPower::Valid()
    {
        return left != -1 && right != -1;
    }

    bool BindingPower::operator==(BindingPower& other)
    {
        return left == other.left && right == other.right;
    }

    bool BindingPower::operator!=(BindingPower& other)
    {
        return !operator==(other);
    }

    BindingPower& GetPrefixPower(SourceToken& token)
    {
        switch (token.mType)
        {
            case SourceToken::Type::Plus:
            case SourceToken::Type::Minus:
            case SourceToken::Type::Not:
            case SourceToken::Type::Tilde:
            case SourceToken::Type::ExclamationPoint:
                return BindingPower(0, 9);
        }

        return BindingPower();
    }

    BindingPower& GetInfixPower(SourceToken& token)
    {
        switch (token.mType)
        {
            case SourceToken::Type::EqualSign:
            case SourceToken::Type::AndEqual:
            case SourceToken::Type::OrEqual:
            case SourceToken::Type::XorEqual:
                return BindingPower(2, 1);
            
            case SourceToken::Type::EqualTo:
            case SourceToken::Type::NotEqual:
            case SourceToken::Type::LessEqual:
            case SourceToken::Type::GreaterEqual:
                return BindingPower(4, 3);
            
            case SourceToken::Type::Plus:
            case SourceToken::Type::Minus:
                return BindingPower(5, 6);
            
            case SourceToken::Type::Times:
            case SourceToken::Type::Divided:
                return BindingPower(7, 8);
            
            case SourceToken::Type::AndAnd:
            case SourceToken::Type::OrOr:
                return BindingPower(10, 11);
        }

        return BindingPower();
    }

    BindingPower& GetPostfixPower(SourceToken& token)
    {
        ///NOTE: this function is currently of no use
        return BindingPower();
    }

    TreeNode::TreeNode()
        : parent(nullptr), left(nullptr), right(nullptr), contents(), operation(-1), specialChar(false)
    {
    }

    TreeNode::TreeNode(std::shared_ptr<TreeNode> pParent, SourceToken& token, int op, bool spChr)
        : parent(pParent), left(nullptr), right(nullptr)
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
    }

    std::shared_ptr<TreeNode> TreeNode::Create(std::shared_ptr<TreeNode> parent, SourceToken& token, int opIdx, bool spChr)
    {
        return std::make_shared<TreeNode>(parent, token, opIdx, spChr);
    }

    AbstractSyntaxTree::AbstractSyntaxTree()
        : mRoot(nullptr)
    {
    }

    void AbstractSyntaxTree::Parse(SourceTokenIterator& tokens)
    {
        int index = 0;
        mRoot = ParseTokens(tokens, 0);
        PrintNode(mRoot, 0);
        nop;
    }

    std::shared_ptr<TreeNode> AbstractSyntaxTree::ParseTokens(SourceTokenIterator& tokens, power_t minPower)
    {
        if (tokens.EndOfList())
            Logging::LogErrorExit("invalid expression, quit before expression was finished");

        auto left = std::make_shared<TreeNode>();
        auto& token = tokens.Next();

        bool wasSpecialChar = false;
        SourceToken::Type type = SourceToken::Type::Invalid;
        int64_t operationIndex = GetOperationIndex(token, type, wasSpecialChar);

        if (type == SourceToken::Type::Invalid)
            return TreeNode::Create(nullptr, token);
        
        BindingPower prefixPower = GetPrefixPower(token);
        prefixPower.Valid() ? nop : Logging::LogErrorExit(stringf("Invalid prefix '%s'", token.Contents.c_str()));

        auto rhs = ParseTokens(tokens, prefixPower.right);
        left->contents = token;
        left->operation = operationIndex;
        left->specialChar = wasSpecialChar;
        left->right = rhs;

        return left;
    }

    int64_t AbstractSyntaxTree::GetOperationIndex(SourceToken& token, SourceToken::Type& operationType, bool& wasSpecialChar)
    {
        auto& operationIterator = std::find_if(Language::SpecialOperators.begin(), Language::SpecialOperators.end(), [token](Language::SpecialOperator& a)
        {
            return a.TokenType == token.mType;
        });
        auto& specialCharIterator = std::find_if(Language::SpecialOperationCharacters.begin(), Language::SpecialOperationCharacters.end(), [token](Language::SpecialCharacter& a)
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

        return std::max(operationIndex, specialCharIndex);
    }

    void AbstractSyntaxTree::PrintNode(std::shared_ptr<TreeNode> node, int depth)
    {
        printf("%s%s\n", std::string((size_t)4*depth, ' ').c_str(), node->contents.Contents.c_str());
        node->left != nullptr ? PrintNode(node->left, depth+1) : nop;
        node->right != nullptr ? PrintNode(node->right, depth+1) : nop;
    }

}