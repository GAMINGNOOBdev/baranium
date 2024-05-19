#include "ArgumentParser.h"
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>

Argument Argument::empty = Argument();

Argument::Argument()
{
    Type = ArgumentType::Invalid;
    Name = "";
    SecondName = "";
    Value = "";
}

Argument::Argument(Argument& other)
{
    Type = other.Type;
    Name = other.Name;
    SecondName = other.SecondName;
    Value = other.Value;
}

Argument::Argument(const Argument& other)
{
    Type = other.Type;
    Name = other.Name;
    SecondName = other.SecondName;
    Value = other.Value;
}

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

bool Argument::Matches(std::string name)
{
    return Name == name || SecondName == name;
}

bool Argument::operator==(Argument other)
{
    return Name == other.Name && SecondName == other.SecondName && Type == other.Type && Value == other.Value;
}

bool Argument::operator!=(Argument other)
{
    return !operator==(other);
}


ArgumentParser::ArgumentParser()
{
    mLookupTable = ArgumentList();
    mParsedArgs = ArgumentList();
    mUnparsedArgs = ArgumentList();
}

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

bool ArgumentParser::HasArgument(std::string name)
{
    return GetArgument(name) != Argument::empty;
}

ArgumentList& ArgumentParser::GetArguments()
{
    return mParsedArgs;
}

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
