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
        : left(nullptr), right(nullptr), contents(), operation(-1), specialChar(false)
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
    }

    AbstractSyntaxTree::AbstractSyntaxTree()
        : mRoot(nullptr)
    {
    }

    void AbstractSyntaxTree::Parse(SourceTokenList tokens)
    {
        int index = 0;
        mRoot = ParseTokens(tokens, index, 0);
        // PrintNode(mRoot, 0);
        nop;
    }

    std::shared_ptr<TreeNode> AbstractSyntaxTree::ParseTokens(SourceTokenList& tokens, int& index, power_t minPower)
    {
        if (index >= tokens.size())
            Logging::LogErrorExit("invalid expression, quit before expression was finished");

        auto lhs = std::make_shared<TreeNode>();
        auto& token = tokens.at(index);
        SourceToken::Type type = SourceToken::Type::Invalid;

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

        if (operationIterator != Language::SpecialOperators.end())
        {
            type = (*operationIterator).TokenType;
            operationIndex = operationIterator - Language::SpecialOperators.begin();
        }
        if (specialCharIterator != Language::SpecialOperationCharacters.end())
        {
            type = (*specialCharIterator).TokenType;
            specialCharIndex = specialCharIterator - Language::SpecialOperationCharacters.begin();
        }

        if (type == SourceToken::Type::Invalid)
        {
            lhs->contents = token;
            index++;
        }
        else
        {
            if (token.mType == SourceToken::Type::ParenthesisOpen)
            {
                // parsing ideas: -use token parsers `ReadContentUsingDepth`
                //                -somehow fix the issue
                index++;
                lhs = ParseTokens(tokens, index, 0);
                if (index >= tokens.size())
                    Logging::LogErrorExit(stringf("Line %d: Invalid content depth", token.LineNumber));

                if (tokens.at(index).mType != SourceToken::Type::ParenthesisClose)
                    Logging::LogErrorExit(stringf("Line %d: Invalid content depth, missing ')'", token.LineNumber));
                goto end;
            }
            else
            {
                BindingPower prefixPower = GetPrefixPower(token);
                prefixPower.Valid() ? Logging::LogErrorExit(stringf("Invalid prefix '%s'", token.Contents.c_str())) : nop;

                index++;
                auto rhs = ParseTokens(tokens, index, prefixPower.right);
                lhs->contents = token;
                lhs->operation = std::max(operationIndex, specialCharIndex);
                lhs->specialChar = specialCharIndex > operationIndex;
                lhs->right = rhs;
            }
        }

        while (index < tokens.size())
        {
            token = tokens.at(index);
            if (token.mType == SourceToken::Type::ParenthesisClose)
                break;

            BindingPower power = GetPostfixPower(token);
            if (power.Valid())
            {
                if (power.left < minPower)
                    break;
                
                index++;
                index >= tokens.size() ? Logging::LogErrorExit(stringf("Line %d: Invalid expression", token.LineNumber)) : nop;
                lhs->left = std::make_shared<TreeNode>(token, std::max(operationIndex, specialCharIndex), false);
                continue;
            }
            power = GetInfixPower(token);
            if (power.Valid())
            {
                if (power.left < minPower)
                    break;

                index++;
                index >= tokens.size() ? Logging::LogErrorExit(stringf("Line %d: Invalid expression", token.LineNumber)) : nop;

                lhs->left = std::make_shared<TreeNode>(token, std::max(operationIndex, specialCharIndex), false);
                lhs->right = ParseTokens(tokens, index, power.right);
                continue;
            }

            Logging::LogErrorExit(stringf("Line %d: Invalid operator token '%s'", token.LineNumber, token.Contents.c_str()));
        }

    end:

        return lhs;
    }

    void AbstractSyntaxTree::PrintNode(std::shared_ptr<TreeNode> node, int depth)
    {
        printf("%s%s\n", std::string((size_t)4*depth, ' ').c_str(), node->contents.Contents.c_str());
        node->left != nullptr ? PrintNode(node->left, depth+1) : nop;
        node->right != nullptr ? PrintNode(node->right, depth+1) : nop;
    }

}