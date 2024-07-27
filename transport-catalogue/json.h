#pragma once
 
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

using namespace std::literals;

namespace json 
{
    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error 
    {
        public:

            using runtime_error::runtime_error;
    };
 
class Node final 
{
    public:
        /* Реализуйте Node, используя std::variant */
        // Поместив nullptr_t в начале списка типов, он становится типом по умолчанию для этого variant
        using Value =  std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    
        Node() = default;
        Node(bool value);
        Node(Array array);
        Node(Dict map);
        Node(int value);
        Node(std::string value);
        Node(std::nullptr_t);
        Node(double value);
        
        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        double AsDouble() const;
        bool AsBool() const;
        const std::string& AsString() const;
    
        bool IsNull() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsArray() const;
        bool IsMap() const;
    
        const Value& GetValue() const 
        { 
            return value_; 
        }

        bool operator==(const Node& rhs) const 
        {
            return value_ == rhs.value_;
        }

        bool operator!=(const Node& rhs) const 
        {
            return !(value_ == rhs.value_);
        }
            
    private:

        Value value_;
    };
    
    class Document 
    {
        public:

            Document() = default;
            
            explicit Document(Node root)
                : root_(std::move(root)) 
                {}
            
            const Node& GetRoot() const 
            {
                return root_;
            }

            bool operator==(const Document& rhs) const 
            {
                return root_ == rhs.root_;
            }

            bool operator!=(const Document& rhs) const 
            {
                return !(root_ == rhs.root_);
            }
        
        private:

            Node root_;
    };
 
    Document Load(std::istream& input);   
    void Print(const Document& doc, std::ostream& output);
} // end namespace json 