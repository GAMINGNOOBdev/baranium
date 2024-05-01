#ifndef __LANGUAGE__ABSTRACTSYNTAXTREE_H_
#define __LANGUAGE__ABSTRACTSYNTAXTREE_H_ 1

#include "../SourceToken.h"
#include <functional>
#include <unordered_map>
#include <stdint.h>
#include <memory>

// quick and dirty name shortcut
#define TreeNodeObject std::shared_ptr<TreeNode>

namespace Language
{

    typedef int8_t power_t;

    /**
     * @brief Binding powers for tokens and token types
     */
    enum class BindingPower : power_t
    {
        Invalid = -1,
        None,
        Assignment,         // variable assignment
        Comparison,         // comparisons like "==", "!=", "<=" or ">="
        PrimaryOperation,   // primary as in very simple operations like + or -
        SecondaryOperation, // secondary as in more advanced/complex operations that should be calculated before the primary ones like * or /
        BitwiseOperation,   // bitwise operations almost always go first, stuff like "|", "^", "&", "~"
        Prefix,             // generic prefix
        Postfix,            // generic postifx
        FunctionCall,       // function call
    };

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
         * @brief The node to the left
         */
        TreeNodeObject left;

        /**
         * @brief The node to the right
         */
        TreeNodeObject right;

        /**
         * @brief Sub nodes of this node
         */
        std::vector<TreeNodeObject> subNodes;

    public:
        /**
         * @brief Construct a new `TreeNode` object
         */
        TreeNode();

        /**
         * @brief Construct a new `TreeNode` object
         *
         * @param token The new contents of this token
         * @param opIdx Operation index
         * @param spChr Special char
         */
        TreeNode(SourceToken& token, int opIdx, bool spChr);

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
        static TreeNodeObject Create();

        /**
         * @brief Yes this is honestly just a contructor but idc it looks better
         */
        static TreeNodeObject Create(SourceToken& token, int opIdx = -1, bool spChr = false);
    };

    using PreInPostFixHandle = std::function<TreeNodeObject(SourceTokenIterator&, TreeNodeObject, power_t power)>;

    /**
     * @brief Small utility class for storing pre-/in-/postfix token parsers
     */
    struct PreInPostFixTokenParser
    {
        // The power of the binding token
        power_t Power;

        // The handler function
        PreInPostFixHandle Handle;

        /**
         * @brief Construct a new `PreInPostFixTokenParser` object
         */
        PreInPostFixTokenParser();

        /**
         * @brief Construct a new `PreInPostFixTokenParser` object
         *
         * @param power The binding power of the token
         * @param handle The parsing function for the token
         */
        PreInPostFixTokenParser(power_t power, PreInPostFixHandle handle);

        /**
         * @brief Construct a new `PreInPostFixTokenParser` object
         *
         * @param power The binding power of the token
         * @param handle The parsing function for the token
         */
        PreInPostFixTokenParser(BindingPower power, PreInPostFixHandle handle);
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
        void Parse(SourceTokenIterator& tokens);

        /**
         * @brief Parse tokens
         *
         * @param tokens The tokens that will be parsed
         * @param minPower Minimum binding power
         * @returns A new tree node
         */
        TreeNodeObject ParseTokens(SourceTokenIterator& tokens, BindingPower minPower);

        /**
         * @brief Parse tokens
         *
         * @param tokens The tokens that will be parsed
         * @param minPower Minimum binding power
         * @returns A new tree node
         */
        TreeNodeObject ParseTokens(SourceTokenIterator& tokens, power_t minPower);

        /**
         * @brief Register a specific token type as a prefix
         *
         * @param tokenType Specific token type
         * @param handle Token parser function pointer
         */
        void RegisterPrefix(SourceToken::Type tokenType, PreInPostFixHandle handle = nullptr);

        /**
         * @brief Register a specific token type as an infix
         *
         * @param tokenType Specific token type
         * @param power The binding power of the token
         * @param handle Token parser function pointer
         */
        void RegisterInfix(SourceToken::Type tokenType, BindingPower power, PreInPostFixHandle handle = nullptr);

        /**
         * @brief Register a specific token type as a postfix
         *
         * @param tokenType Specific token type
         * @param power The binding power of the token
         * @param handle Token parser function pointer
         */
        void RegisterPostfix(SourceToken::Type tokenType, BindingPower power, PreInPostFixHandle handle = nullptr);

        /**
         * @brief Get the operation index of a token
         *
         * @param token The token that will be analyzed
         * @param operationType Output of the operation type
         * @param wasSpecialChar Output of the special type
         * @returns The operation index
         */
        static int64_t GetOperationIndex(SourceToken& token, SourceToken::Type& operationType, bool& wasSpecialChar);

    private:
        power_t GetNextPrecedence(SourceTokenIterator& tokens);

    private:
        std::unordered_map<SourceToken::Type, PreInPostFixTokenParser> mPrefixLookup;
        std::unordered_map<SourceToken::Type, PreInPostFixTokenParser> mInfixLookup;
        TreeNodeObject mRoot;
    };

}

#endif
