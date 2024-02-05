#include "ArgumentParser.h"
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief An empty argument object for comparing if a return value is valid or not
 */
Argument Argument::empty = Argument();

/**
 * @brief Construct a new emtpy `Argument`
 */
Argument::Argument()
{
    Type = ArgumentType::Invalid;
    Name = "";
    SecondName = "";
    Value = "";
}

/**
 * @brief Construct a new `Argument` object
 * 
 * @param other The other `Argument` object that will be copied
 */
Argument::Argument(Argument& other)
{
    Type = other.Type;
    Name = other.Name;
    SecondName = other.SecondName;
    Value = other.Value;
}

/**
 * @brief Construct a new `Argument` object
 * 
 * @param other The other `Argument` object that will be copied
 */
Argument::Argument(const Argument& other)
{
    Type = other.Type;
    Name = other.Name;
    SecondName = other.SecondName;
    Value = other.Value;
}

/**
 * @returns The string representation of this `Argument` object
 */
std::string Argument::ToString()
{
    std::string asString = std::string("Argument{ ");
    asString += Name;
    asString += "/";
    asString += SecondName;
    asString += ", ";
    std::string argTypeString = "";
    switch (Type)
    {
        #define ARGUMENT_TYPE_DEF(arg) case ArgumentType::arg: argTypeString=#arg; break;
            _ARGUMENT_TYPES
        #undef ARGUMENT_TYPE_DEF
    }
    asString += argTypeString;
    asString += ", ";
    if (!Value.empty())
    {
        asString += "\"";
        asString += Value;
        asString += "\"";
    }
    else asString += "none";
    asString += " }";
    return asString;
}

/**
 * @brief Checks if this arguments matches the given name
 * 
 * @param name The desired argument name
 * @return `true` if this is the desired argument, 
 * @return `false` if this argument doesn't match or is empty
 */
bool Argument::Matches(std::string name)
{
    return Name == name || SecondName == name;
}

/**
 * @brief Compare two `Argument` objects if they are the same
 * 
 * @param other The other `Argument` object
 * @return `true` if both are equal,
 * @return `false` if both are not equal
 */
bool Argument::operator==(Argument other)
{
    return Name == other.Name && SecondName == other.SecondName && Type == other.Type && Value == other.Value;
}

/**
 * @brief Compare two `Argument` objects if they are not the same
 * 
 * @param other The other `Argument` object
 * @return `true` if both are not equal,
 * @return `false` if both are equal
 */
bool Argument::operator!=(Argument other)
{
    return !operator==(other);
}


/**
 * @brief Construct a new `ArgumentParser` object
 */
ArgumentParser::ArgumentParser()
{
    mLookupTable = ArgumentList();
    mParsedArgs = ArgumentList();
    mUnparsedArgs = ArgumentList();
}

/**
 * @brief Add an argument to parse
 * 
 * @param type Type of the argument
 * @param name Name of the argument, mostly the simple abbreviation
 * @param alternateName An alternative name of the argument, mostly the full name of the argument
 */
void ArgumentParser::AddArgument(ArgumentType type, const char* name, const char* alternateName)
{
    if (!name)
        return;

    Argument arg = Argument();
    arg.Type = type;
    arg.Name = name;
    if (alternateName)
        arg.SecondName = alternateName;

    arg.Value = "";
    
    mLookupTable.push_back(arg);
}

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
void ArgumentParser::ParseArguments(int argC, const char** argV)
{
    mParsedArgs.clear();
    mUnparsedArgs.clear();

    for (int argIdx = 0; argIdx < argC; argIdx++)
    {
        std::string argStr = argV[argIdx];

        if (argStr[0] != '-')
        {
            Argument arg0 = Argument();
            arg0.Name = arg0.SecondName = arg0.Value = std::string(argStr);
            AddToUnparsed(arg0);
            continue;
        }

        for (auto& arg : mLookupTable)
        {
            if (!arg.Matches(argStr))
                continue;

            if (arg.Type == ArgumentType::Value)
            {
                if (argIdx == argC - 1)
                    continue;

                Argument arg0 = Argument();
                arg0.Type = ArgumentType::Value;
                arg0.Name = arg.Name;
                arg0.SecondName = arg.SecondName;
                arg0.Value = argV[argIdx + 1];
                AddToParsed(arg0);
                argIdx++;

                continue;
            }

            Argument arg0 = Argument();
            arg0.Type = ArgumentType::Flag;
            arg0.Name = arg.Name;
            arg0.SecondName = arg.SecondName;
            AddToParsed(arg0);
        }
    }
}

/**
 * @brief Get a specific argument
 * 
 * @note Only parsed arguments will be checked
 * 
 * @param name Name of the desired argument
 * 
 * @return The specific argument
 */
Argument ArgumentParser::GetArgument(std::string name)
{
    auto iterator = std::find_if(mParsedArgs.begin(), mParsedArgs.end(),
        [name](Argument& argument)
        {
            return argument.Matches(name);
        }
    );
    if (iterator == mParsedArgs.end())
        return Argument::empty;

    return *iterator;
}

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
bool ArgumentParser::HasArgument(std::string name)
{
    return GetArgument(name) != Argument::empty;
}

/**
 * @brief Get the list of parsed arguments
 * 
 * @return The list of parsed arguments
 */
ArgumentList& ArgumentParser::GetArguments()
{
    return mParsedArgs;
}

/**
 * @brief Get the list of unparsed arguments
 * 
 * @note This will in most cases contain filenames
 *       or the like, but it is advised to check
 *       for invalid data yourself
 * 
 * @return The list of unparsed arguments
 */
ArgumentList& ArgumentParser::GetUnparsedArguments()
{
    return mUnparsedArgs;
}

void ArgumentParser::AddToParsed(Argument arg)
{
    auto iterator = std::find(mParsedArgs.begin(), mParsedArgs.end(), arg);
    if (iterator != mParsedArgs.end())
        return;

    mParsedArgs.push_back(arg);
}

void ArgumentParser::AddToUnparsed(Argument arg)
{
    auto iterator = std::find(mUnparsedArgs.begin(), mUnparsedArgs.end(), arg);
    if (iterator != mUnparsedArgs.end())
        return;

    mUnparsedArgs.push_back(arg);
}
