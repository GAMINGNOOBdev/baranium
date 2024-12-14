#ifndef __ARGUMENTPARSER_H_
#define __ARGUMENTPARSER_H_ 1

#include <string>
#include <vector>

#define ARGUMENT_TYPE_DEF(arg) arg,

#define _ARGUMENT_TYPES \
    ARGUMENT_TYPE_DEF(Invalid) \
    ARGUMENT_TYPE_DEF(Value) \
    ARGUMENT_TYPE_DEF(Flag)

/**
 * @brief Type of the argument, i.e. a flag or something that has a value
 */
enum class ArgumentType
{
    #define ARGUMENT_TYPE_DEF(arg) arg,

    _ARGUMENT_TYPES

    #undef ARGUMENT_TYPE_DEF
};

/**
 * @brief An argument, either a flag or containing a value
 */
struct Argument
{
    /**
     * @brief An empty argument object for comparing if a return value is valid or not
     */
    static Argument empty;

    /**
     * @brief Type of argument, either flag, value or invalid
     */
    ArgumentType Type;

    /**
     * @brief Name of the argument, mostly an abbreviation
     */
    std::string Name;

    /**
     * @brief Second name of the argument, mostly the written out form of the abbreviation
     */
    std::string SecondName;

    /**
     * @brief The value that this `Argument` object holds
     */
    std::string Value;

    /**
     * @brief Construct a new emtpy `Argument`
     */
    Argument();

    /**
     * @brief Construct a new `Argument` object
     * 
     * @param other The other `Argument` object that will be copied
     */
    Argument(Argument& other);

    /**
     * @brief Construct a new `Argument` object
     * 
     * @param other The other `Argument` object that will be copied
     */
    Argument(const Argument& other);

    /**
     * @returns The string representation of this `Argument` object
     */
    std::string ToString();

    /**
     * @brief Checks if this arguments matches the given name
     * 
     * @param name The desired argument name
     * @return `true` if this is the desired argument, 
     * @return `false` if this argument doesn't match or is empty
     */
    bool Matches(std::string name);

    /**
     * @brief Compare two `Argument` objects if they are the same
     * 
     * @param other The other `Argument` object
     * @return `true` if both are equal,
     * @return `false` if both are not equal
     */
    bool operator==(Argument other);

    /**
     * @brief Compare two `Argument` objects if they are not the same
     * 
     * @param other The other `Argument` object
     * @return `true` if both are not equal,
     * @return `false` if both are equal
     */
    bool operator!=(Argument other);
};

/**
 * @brief Small abbreviation to not write long type names everytime
*/
using ArgumentList = std::vector<Argument>;

/**
 * @brief An object that parses arguments from the command line
 */
struct ArgumentParser
{
    /**
     * @brief Construct a new `ArgumentParser` object
     */
    ArgumentParser();

    /**
     * @brief Add an argument to parse
     * 
     * @param type Type of the argument
     * @param name Name of the argument, mostly the simple abbreviation
     * @param alternateName An alternative name of the argument, mostly the full name of the argument
     */
    void AddArgument(ArgumentType type, const char* name, const char* alternateName);

    /**
     * @brief Parse the arguments from the command line
     * 
     * @note Calling `AddArgument()` beforehand is advised
     *       since this function will only parse arguments
     *       that have been added to the list of
     *       available/desired arguments
     * 
     * @param argC Argument count
     * @param argV Argument value array
     */
    void ParseArguments(int argC, const char** argV);

    /**
     * @brief Get a specific argument
     * 
     * @note Only parsed arguments will be checked
     * 
     * @param name Name of the desired argument
     * 
     * @return The specific argument
     */
    Argument GetArgument(std::string name);

    /**
     * @brief Check if a specific argument is present
     * 
     * @note Only parsed arguments will be checked
     * 
     * @param name Name of the desired argument
     * 
     * @return `true` if the desired argument is present,
     * @return `false` if it isn't
     */
    bool HasArgument(std::string name);

    /**
     * @brief Get the list of parsed arguments
     * 
     * @return The list of parsed arguments
     */
    ArgumentList& GetArguments();

    /**
     * @brief Get the list of unparsed arguments
     * 
     * @note This will in most cases contain filenames
     *       or the like, but it is advised to check
     *       for invalid data yourself
     * 
     * @return The list of unparsed arguments
     */
    ArgumentList& GetUnparsedArguments();

private:
    void AddToParsed(Argument arg);
    void AddToUnparsed(Argument arg);

    ArgumentList mUnparsedArgs;
    ArgumentList mLookupTable;
    ArgumentList mParsedArgs;
};

#endif
