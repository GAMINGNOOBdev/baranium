#ifndef __LANGUAGE__ABSTRACTSYNTAXTREE_H_
#define __LANGUAGE__ABSTRACTSYNTAXTREE_H_ 1

#include "../SourceToken.h"
#include <stdint.h>
#include <memory>

namespace Language
{
    
    typedef int8_t power_t;

    /**
     * @brief A class that describes the binding power for operands towards it's left and right to identify expressions
     */
    struct BindingPower
    {
        power_t left;
        power_t right;

    public:
        /**
         * @brief Construct a new `BindingPower` object
         */
        BindingPower();

        /**
         * @brief Construct a new `BindingPower` object
         * 
         * @param l Left binding power
         * @param r Right binding power
         */
        BindingPower(power_t l, power_t r);

        /**
         * @brief Check if binding power is valid
         */
        bool Valid();

        /**
         * @brief Check if `this` and `other` are equal
         */
        bool operator==(BindingPower& other);

        /**
         * @brief Check if `this` and `other` are not equal
         */
        bool operator!=(BindingPower& other);
    };

    /**
     * @brief Get the binding power of a prefix
     * 
     * @param token Prefix token
     */
    BindingPower& GetPrefixPower(SourceToken& token);

    /**
     * @brief Get the binding power of an infix
     * 
     * @param token Infix token
     */
    BindingPower& GetInfixPower(SourceToken& token);

    /**
     * @brief Get the binding power of a postfix
     * 
     * @param token Postfix token
     */
    BindingPower& GetPostfixPower(SourceToken& token);

    /**
     * @brief A node of the abstract syntax tree
     */
    struct TreeNode
    {
        /**
         * @brief The contents of this node
         */
        SourceToken contents;

        /**
         * @brief The operation index, -1 if this node it not an operation
         */
        int operation;

        /**
         * @brief Specifies whether this was a speical character or an operation
         */
        bool specialChar;

        /**
         * @brief The parent node
         */
        std::shared_ptr<TreeNode> parent;

        /**
         * @brief The node to the left
         */
        std::shared_ptr<TreeNode> left;

        /**
         * @brief The node to the right
         */
        std::shared_ptr<TreeNode> right;

    public:
        /**
         * @brief Construct a new `TreeNode` object
         */
        TreeNode();

        /**
         * @brief Construct a new `TreeNode` object
         * 
         * @param parent Parent node
         * @param token The new contents of this token
         * @param opIdx Operation index
         * @param spChr Special char
         */
        TreeNode(std::shared_ptr<TreeNode> pParent, SourceToken& token, int opIdx, bool spChr);

        /**
         * @brief Check if this node is valid
         */
        bool Valid();

        /**
         * @brief set the value of this object
         */
        void operator=(const Language::TreeNode& other);

        /**
         * @brief Yes this is honestly just a contructor but idc it looks better
         */
        static std::shared_ptr<TreeNode> Create(std::shared_ptr<TreeNode> parent, SourceToken& token, int opIdx = -1, bool spChr = false);
    };

    /**
     * @brief An abstract syntax tree that divides expressions into multiple parts/layers
     */
    struct AbstractSyntaxTree
    {
        /**
         * @brief Construct a new `AbstractSyntaxTree` object
         */
        AbstractSyntaxTree();

        /**
         * @brief Parse tokens into this tree
         * 
         * @param tokens The tokens to parse
         */
        void Parse(SourceTokenList tokens);

    private:
        std::shared_ptr<TreeNode> ParseTokens(SourceTokenList& tokens, int& index, power_t minPower);
        int64_t GetOperationIndex(SourceToken& token, SourceToken::Type& operationType, bool& wasSpecialChar);
        void PrintNode(std::shared_ptr<TreeNode> node, int depth);
        void AddNode(SourceToken& token, int opIdx, bool spChr);

    private:
        std::shared_ptr<TreeNode> mRoot;
    };

}

#endif