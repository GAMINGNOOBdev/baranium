#ifndef __LANGUAGE__GENERICTYPEHOLDER_H_
#define __LANGUAGE__GENERICTYPEHOLDER_H_ 1

#include <typeinfo>
#include <string>
#include <vector>

namespace Language
{

    /**
     * @brief Types of values that can be stored
     */
    enum class GenericValueType
    {
        /**
         * @brief Invalid state
         */
        Invalid,

        /**
         * @brief A string object
         */
        String,

        /**
         * @brief An integer (signed!)
         */
        Integer,

        /**
         * @brief A Floating-point number
         */
        Float,

        /**
         * @brief A boolean
         */
        Bool,

        /**
         * @brief An array of objects, strings, numbers or anything that can be stored
         */
        Array,

        /**
         * @brief Another Generic object
         */
        Object
    };

    /**
     * @brief A handy little struct for handling different types of values
     * 
     * @tparam T Type
     */
    template<typename T>
    struct GenericValueTemplate
    {
        typedef T Type;
    };

    /**
     * @brief An entry in a Generic object which can also store objects and values
     */
    struct GenericEntry
    {
        template<typename T>
        GenericEntry(std::string name, T value)
        {
            Name = name;
            SetValue(value, GenericValueTemplate<T>());
        }

        /**
         * @brief Name of this entry
         */
        std::string Name;

        /**
         * @brief The type of value that is stored in this entry
         */
        GenericValueType ValueType;

        /**
         * @brief Set the value of this entry
         * 
         * @tparam The type of value that this entry should have
         * @param value The value of this entry
         */
        template<typename T>
        void SetValue(T value)
        {
            ValueType = GenericValueType::Invalid;
            SetValue(value, GenericValueTemplate<T>());
        }

        /**
         * @brief Set the value of this entry
         * 
         * @tparam The type of value that this entry should have
         * @param value The value of this entry
         */
        template<typename T>
        void SetValue(T value, GenericValueTemplate<T>)
        {
            ValueType = GenericValueType::Invalid;
        }

        /**
         * @brief Set the value of this entry
         * 
         * @tparam The type of value that this entry should have
         * @param value The value of this entry
         */
        void SetValue(std::string value, GenericValueTemplate<std::string>)
        {
            ValueType = GenericValueType::String;
            StringValue = value;
        }

        /**
         * @brief Set the value of this entry
         * 
         * @tparam The type of value that this entry should have
         * @param value The value of this entry
         */
        void SetValue(int value, GenericValueTemplate<int>)
        {
            ValueType = GenericValueType::Integer;
            IntegerValue = value;
        }

        /**
         * @brief Set the value of this entry
         * 
         * @tparam The type of value that this entry should have
         * @param value The value of this entry
         */
        void SetValue(float value, GenericValueTemplate<float>)
        {
            ValueType = GenericValueType::Float;
            FloatValue = value;
        }

        /**
         * @brief Set the value of this entry
         * 
         * @tparam The type of value that this entry should have
         * @param value The value of this entry
         */
        void SetValue(bool value, GenericValueTemplate<bool>)
        {
            ValueType = GenericValueType::Bool;
            BoolValue = value;
        }

        /**
         * @brief Get the value of this entry
         * 
         * @tparam The type of value reqeusted by the user
         * @returns The value of this entry
         */
        template<typename T>
        T GetValue()
        {
            if (GetTypeInfoFromValueType(ValueType) != typeid(T))
                return T();

            return GetValue(GenericValueTemplate<T>());
        }

        /**
         * @brief Get the value of this entry
         * 
         * @tparam The type of value reqeusted by the user
         * @returns The value of this entry
         */
        std::string GetValue(GenericValueTemplate<std::string>)
        {
            if (GetTypeInfoFromValueType(ValueType) != typeid(std::string))
                return std::string();
            
            return StringValue;
        }

        /**
         * @brief Get the value of this entry
         * 
         * @tparam The type of value reqeusted by the user
         * @returns The value of this entry
         */
        int GetValue(GenericValueTemplate<int>)
        {
            if (GetTypeInfoFromValueType(ValueType) != typeid(int))
                return int();
            
            return IntegerValue;
        }

        /**
         * @brief Get the value of this entry
         * 
         * @tparam The type of value reqeusted by the user
         * @returns The value of this entry
         */
        float GetValue(GenericValueTemplate<float>)
        {
            if (GetTypeInfoFromValueType(ValueType) != typeid(float))
                return float();
            
            return FloatValue;
        }

        /**
         * @brief Get the value of this entry
         * 
         * @tparam The type of value reqeusted by the user
         * @returns The value of this entry
         */
        bool GetValue(GenericValueTemplate<bool>)
        {
            if (GetTypeInfoFromValueType(ValueType) != typeid(bool))
                return bool();
            
            return BoolValue;
        }

        /**
         * @returns The string representation of this `GenericEntry`
         */
        std::string ToString()
        {
            return std::string("GenericEntry{ ")
                        .append("Type: ")
                        .append(ValueTypeToString())
                        .append(" }");
        }

        /**
         * @returns Convert The string representation of the current value type
         */
        std::string ValueTypeToString()
        {
            switch (ValueType)
            {
                case GenericValueType::String:
                    return "Type::String";

                case GenericValueType::Integer:
                    return "Type::Integer";

                case GenericValueType::Float:
                    return "Type::Float";
                
                case GenericValueType::Bool:
                    return "Type::Bool";

                case GenericValueType::Array:
                    return "Type::Array";

                case GenericValueType::Object:
                    return "Type::Object";
                
                default:
                case GenericValueType::Invalid:
                    return "Type::Invalid";
            }
        }

        /**
         * @brief Get the value type from the type info
         * 
         * @param type Type info
         * 
         * @returns The specific value type
         */
        const GenericValueType GetValueTypeFromTypeInfo(const std::type_info& type)
        {
            if (type == typeid(std::string))
                return GenericValueType::String;

            if (type == typeid(int))
                return GenericValueType::Integer;

            if (type == typeid(float))
                return GenericValueType::Float;

            if (type == typeid(bool))
                return GenericValueType::Bool;

            return GenericValueType::Invalid;
        }

        /**
         * @brief Get the type info from the value type
         * 
         * @param type Value type
         * 
         * @returns The type info 
         */
        const std::type_info& GetTypeInfoFromValueType(GenericValueType type)
        {
            switch (type)
            {
                case GenericValueType::String:
                    return typeid(std::string);

                case GenericValueType::Integer:
                    return typeid(int);

                case GenericValueType::Float:
                    return typeid(float);
                
                case GenericValueType::Bool:
                    return typeid(bool);

                case GenericValueType::Invalid:
                default:
                    return typeid(void*);
            }
        }

    private:
        bool            BoolValue;
        float           FloatValue;
        std::string     StringValue;
        int             IntegerValue;
    };

}

#endif