#include "AST_TokenParsers.h"
#include "../StringUtil.h"
#include "../Logging.h"

void Language::SetupParserHandles(AbstractSyntaxTree& ast)
{
    ///////////////////////////
    /// Generic item parser ///
    ///////////////////////////

    auto genericParser = [](SourceTokenIterator& tokens, TreeNodeObject parentNode, power_t power){
        auto result = TreeNode::Create();
        result->contents = tokens.Current();
        return result;
    };

    ast.RegisterPrefix(SourceToken::Type::Null, genericParser);
    ast.RegisterPrefix(SourceToken::Type::Text, genericParser);
    ast.RegisterPrefix(SourceToken::Type::Number, genericParser);
    ast.RegisterPrefix(SourceToken::Type::Keyword, genericParser);

    /////////////////////
    /// String parser ///
    /////////////////////

    auto stringParser = [](SourceTokenIterator& tokens, TreeNodeObject parentNode, power_t power)
    {
        auto result = TreeNode::Create();
        result->contents = tokens.Current();
        tokens.Next();
        result->left = TreeNode::Create();
        result->left->contents = tokens.Current();
        if (!tokens.NextMatches(SourceToken::Type::DoubleQuote))
            Logging::LogErrorExit(stringf("Line %d: missing \" at the end of string", tokens.Current().LineNumber));
        return result;
    };

    ast.RegisterPrefix(SourceToken::Type::DoubleQuote, stringParser);

    ////////////////////////
    /// Prefix operators ///
    ////////////////////////

    auto prefixOperatorParser = [&ast](SourceTokenIterator& tokens, TreeNodeObject parentNode, power_t power)
    {
        auto result = TreeNode::Create();
        result->contents = tokens.Current();
        SourceToken::Type type;
        bool wasSpecialOperation = false;
        int64_t operationIndex = AbstractSyntaxTree::GetOperationIndex(tokens.Current(), type, wasSpecialOperation);
        result->operation = operationIndex;
        result->specialChar = wasSpecialOperation;
        result->right = ast.ParseTokens(tokens, BindingPower::Prefix);
        return result;
    };

    ast.RegisterPrefix(SourceToken::Type::Plus, prefixOperatorParser);
    ast.RegisterPrefix(SourceToken::Type::Minus, prefixOperatorParser);
    ast.RegisterPrefix(SourceToken::Type::Tilde, prefixOperatorParser);
    ast.RegisterPrefix(SourceToken::Type::ExclamationPoint, prefixOperatorParser);

    /////////////////////////////////
    /// In-/De-crement operations ///
    /////////////////////////////////

    auto indecrementOperatorParser = [&ast](SourceTokenIterator& tokens, TreeNodeObject parentNode, power_t power)
    {
        if (parentNode == nullptr)
            Logging::LogErrorExit("whar");

        auto result = TreeNode::Create();
        result->contents = tokens.Next();
        SourceToken::Type type;
        bool wasSpecialOperation = false;
        int64_t operationIndex = AbstractSyntaxTree::GetOperationIndex(result->contents, type, wasSpecialOperation);
        result->operation = operationIndex;
        result->specialChar = wasSpecialOperation;
        result->left = TreeNode::Create();
        result->left->contents = tokens.Current();
        operationIndex = AbstractSyntaxTree::GetOperationIndex(result->contents, type, wasSpecialOperation);
        result->left->operation = operationIndex;
        result->left->specialChar = wasSpecialOperation;

        if (result->left->contents.mType != SourceToken::Type::Text)
            Logging::LogErrorExit(stringf("Line %d: Invalid assignment, expected variable name, got '%s'", result->left->contents.LineNumber, result->left->contents.Contents.c_str()));

        return result;
    };

    ast.RegisterPrefix(SourceToken::Type::PlusPlus, indecrementOperatorParser);
    ast.RegisterPrefix(SourceToken::Type::MinusMinus, indecrementOperatorParser);

    /////////////////////////////////
    /// Parenthesis order reading ///
    /////////////////////////////////

    ast.RegisterPrefix(SourceToken::Type::ParenthesisOpen, [&ast](SourceTokenIterator& tokens, TreeNodeObject parentNode, power_t power){
        auto result = ast.ParseTokens(tokens, BindingPower::None);

        tokens.Next();
        auto closingToken = tokens.Current();
        if (closingToken.mType != SourceToken::Type::ParenthesisClose)
            Logging::LogErrorExit(stringf("Line %d: Expected ')', got '%s'", closingToken.LineNumber, closingToken.Contents.c_str()));

        return result;
    });

    /////////////////////////////
    /// Function call reading ///
    /////////////////////////////

    ast.RegisterInfix(SourceToken::Type::ParenthesisOpen, BindingPower::FunctionCall, [&ast](SourceTokenIterator& tokens, TreeNodeObject parentNode, power_t power){
        auto result = TreeNode::Create();
        result->operator=(*parentNode); // copy the parent node, which will probably be the function name, well hopefully
        SourceToken::Type type;
        bool wasSpecialOperation = false;
        int64_t operationIndex = AbstractSyntaxTree::GetOperationIndex(result->contents, type, wasSpecialOperation);
        result->operation = operationIndex;
        result->specialChar = wasSpecialOperation;

        if (!tokens.NextMatches(SourceToken::Type::ParenthesisClose))
        {
            do
            {
                result->subNodes.push_back(ast.ParseTokens(tokens, BindingPower::None));
            }
            while (tokens.NextMatches(SourceToken::Type::Comma));
            if (!tokens.NextMatches(SourceToken::Type::ParenthesisClose))
                Logging::LogErrorExit(stringf("Line %d: Missing ')'", tokens.Current().LineNumber));
        }

        return result;
    });

    ////////////////////////////
    /// Comparison operators ///
    ////////////////////////////

    auto comparisonOperatorParser = [&ast](SourceTokenIterator& tokens, TreeNodeObject parentNode, power_t power)
    {
        auto result = TreeNode::Create();
        result->contents = tokens.Current();
        SourceToken::Type type;
        bool wasSpecialOperation = false;
        int64_t operationIndex = AbstractSyntaxTree::GetOperationIndex(tokens.Current(), type, wasSpecialOperation);
        result->operation = operationIndex;
        result->specialChar = wasSpecialOperation;
        result->right = ast.ParseTokens(tokens, BindingPower::None);
        result->left = std::make_shared<TreeNode>(*parentNode);
        return result;
    };

    ast.RegisterInfix(SourceToken::Type::EqualTo, BindingPower::Comparison, comparisonOperatorParser);
    ast.RegisterInfix(SourceToken::Type::NotEqual, BindingPower::Comparison, comparisonOperatorParser);
    ast.RegisterInfix(SourceToken::Type::LessEqual, BindingPower::Comparison, comparisonOperatorParser);
    ast.RegisterInfix(SourceToken::Type::GreaterEqual, BindingPower::Comparison, comparisonOperatorParser);
    ast.RegisterInfix(SourceToken::Type::LessThan, BindingPower::Comparison, comparisonOperatorParser);
    ast.RegisterInfix(SourceToken::Type::GreaterThan, BindingPower::Comparison, comparisonOperatorParser);
    ast.RegisterInfix(SourceToken::Type::AndAnd, BindingPower::Comparison, comparisonOperatorParser);
    ast.RegisterInfix(SourceToken::Type::OrOr, BindingPower::Comparison, comparisonOperatorParser);

    ///////////////////////
    /// Infix operators ///
    ///////////////////////

    auto infixOperatorParser = [&ast](SourceTokenIterator& tokens, TreeNodeObject parentNode, power_t power)
    {
        auto result = TreeNode::Create();
        result->contents = tokens.Current();
        SourceToken::Type type;
        bool wasSpecialOperation = false;
        int64_t operationIndex = AbstractSyntaxTree::GetOperationIndex(tokens.Current(), type, wasSpecialOperation);
        result->operation = operationIndex;
        result->specialChar = wasSpecialOperation;
        result->right = ast.ParseTokens(tokens, power);
        result->left = std::make_shared<TreeNode>(*parentNode);
        return result;
    };

    ast.RegisterInfix(SourceToken::Type::Plus, BindingPower::PrimaryOperation, infixOperatorParser);
    ast.RegisterInfix(SourceToken::Type::Minus, BindingPower::PrimaryOperation, infixOperatorParser);
    ast.RegisterInfix(SourceToken::Type::Asterisk, BindingPower::SecondaryOperation, infixOperatorParser);
    ast.RegisterInfix(SourceToken::Type::Slash, BindingPower::SecondaryOperation, infixOperatorParser);
    ast.RegisterInfix(SourceToken::Type::Modulo, BindingPower::SecondaryOperation, infixOperatorParser);
    ast.RegisterInfix(SourceToken::Type::Or, BindingPower::BitwiseOperation, infixOperatorParser);
    ast.RegisterInfix(SourceToken::Type::And, BindingPower::BitwiseOperation, infixOperatorParser);
    ast.RegisterInfix(SourceToken::Type::Caret, BindingPower::BitwiseOperation, infixOperatorParser);
    ast.RegisterInfix(SourceToken::Type::BitshiftLeft, BindingPower::BitwiseOperation, infixOperatorParser);
    ast.RegisterInfix(SourceToken::Type::BitshiftRight, BindingPower::BitwiseOperation, infixOperatorParser);

    ////////////////////////////
    /// Assignment operation ///
    ////////////////////////////

    auto assignmentOperatorParser = [&ast](SourceTokenIterator& tokens, TreeNodeObject parentNode, power_t power)
    {
        if (parentNode == nullptr)
            Logging::LogErrorExit("whar");

        if (parentNode->contents.mType != SourceToken::Type::Text)
            Logging::LogErrorExit(stringf("Line %d: Invalid assignment, expected variable name, got '%s'", parentNode->contents.LineNumber, parentNode->contents.Contents.c_str()));

        auto result = TreeNode::Create();
        result->contents = tokens.Current();
        SourceToken::Type type;
        bool wasSpecialOperation = false;
        int64_t operationIndex = AbstractSyntaxTree::GetOperationIndex(tokens.Current(), type, wasSpecialOperation);
        result->operation = operationIndex;
        result->specialChar = wasSpecialOperation;
        result->right = ast.ParseTokens(tokens, BindingPower::None);
        result->left = std::make_shared<TreeNode>(*parentNode);
        return result;
    };

    ast.RegisterInfix(SourceToken::Type::EqualSign, BindingPower::Assignment, assignmentOperatorParser);
    ast.RegisterInfix(SourceToken::Type::AndEqual, BindingPower::Assignment, assignmentOperatorParser);
    ast.RegisterInfix(SourceToken::Type::OrEqual, BindingPower::Assignment, assignmentOperatorParser);
    ast.RegisterInfix(SourceToken::Type::XorEqual, BindingPower::Assignment, assignmentOperatorParser);
    ast.RegisterInfix(SourceToken::Type::PlusEqual, BindingPower::Assignment, assignmentOperatorParser);
    ast.RegisterInfix(SourceToken::Type::MinusEqual, BindingPower::Assignment, assignmentOperatorParser);
    ast.RegisterInfix(SourceToken::Type::MulEqual, BindingPower::Assignment, assignmentOperatorParser);
    ast.RegisterInfix(SourceToken::Type::DivEqual, BindingPower::Assignment, assignmentOperatorParser);
    ast.RegisterInfix(SourceToken::Type::ModEqual, BindingPower::Assignment, assignmentOperatorParser);

    /////////////////////////
    /// Postfix operators ///
    /////////////////////////

    auto postfixOperatorParser = [](SourceTokenIterator& tokens, TreeNodeObject parentNode, power_t power)
    {
        if (parentNode == nullptr)
            Logging::LogErrorExit("whar");

        if (parentNode->contents.mType != SourceToken::Type::Text)
            Logging::LogErrorExit(stringf("Line %d: Invalid assignment, expected variable name, got '%s'", parentNode->contents.LineNumber, parentNode->contents.Contents.c_str()));

        auto result = TreeNode::Create();
        result->contents = tokens.Current();
        SourceToken::Type type;
        bool wasSpecialOperation = false;
        int64_t operationIndex = AbstractSyntaxTree::GetOperationIndex(tokens.Current(), type, wasSpecialOperation);
        result->operation = operationIndex;
        result->specialChar = wasSpecialOperation;
        result->left = std::make_shared<TreeNode>(*parentNode);
        return result;
    };

    ast.RegisterPostfix(SourceToken::Type::PlusPlus, BindingPower::Postfix, postfixOperatorParser);
    ast.RegisterPostfix(SourceToken::Type::MinusMinus, BindingPower::Postfix, postfixOperatorParser);
}
