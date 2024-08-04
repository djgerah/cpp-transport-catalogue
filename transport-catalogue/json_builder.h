#pragma once

#include <string>
#include <vector>
#include "json.h"

namespace json 
{
    class Builder 
    {
        class BaseContext;
        class DictValueContext;
        class DictItemContext;
        class ArrayItemContext;

        public:

            Builder()
                : root_()
                , nodes_stack_{&root_}
                {}
                
            DictValueContext Key(std::string key);
            BaseContext Value(Node::Value value);
            DictItemContext StartDict();
            ArrayItemContext StartArray();
            BaseContext EndDict();
            BaseContext EndArray();
            Node Build();

        private:

            void AddObject(Node::Value value, bool one_shot);
            Node* GetTopNode();
            Node::Value& GetCurrentValue();
            const Node::Value& GetCurrentValue() const;
            
            Node root_;
            std::vector<Node*> nodes_stack_;
            
        // При таком подходе останутся ошибки, которые продолжат выявляться лишь на этапе запуска:
        // Вызов некорректного метода сразу после создания json::Builder.
        // Вызов некорректного метода после End*.
        class BaseContext 
        {
            public:

                BaseContext(Builder& builder) 
                    : builder_(builder) 
                    {}
                
                DictValueContext Key(std::string key);
                BaseContext Value(Node::Value value);
                DictItemContext StartDict();
                ArrayItemContext StartArray();
                BaseContext EndDict();
                BaseContext EndArray();
                Node Build();           

            private:

                Builder& builder_;
        };

        // Гарантирует, что за вызовом StartDict следует Key или EndDict.
        // StartDict() → Key(), EndDict() 
        // Гарантирует, что после вызова Value, последовавшего за вызовом Key, вызван Key или EndDict
        // Key() → Value() → Key(), EndDict()
        class DictItemContext : public BaseContext 
        {
            public:

                DictItemContext(BaseContext base) 
                    : BaseContext(base) 
                    {}
                
                // => Поддерживает только методы Key и EndDict, делегируемые в Builder
                BaseContext Value(Node::Value value) = delete;
                DictItemContext StartDict() = delete;
                ArrayItemContext StartArray() = delete;
                BaseContext EndArray() = delete;
                Node Build() = delete;
        };

        // Гарантирует, что за вызовом StartArray следует Value, StartDict, StartArray или EndArray
        // StartArray() → Value(), StartDict(), StartArray(), EndArray() 

        // Гарантирует, что после вызова StartArray и серии Value следует Value, StartDict, StartArray или EndArray
        // StartArray() → Value() → Value(), StartDict(), StartArray(), EndArray()
        class ArrayItemContext : public BaseContext 
        {
            public:
        
                ArrayItemContext(BaseContext base) 
                    : BaseContext(base) 
                    {}
                
                ArrayItemContext Value(Node::Value value) 
                { 
                    return BaseContext::Value(std::move(value)); 
                }

                // => Поддерживает только методы Value, StartDict, StartArray и EndArray, делегируемые в Builder
                DictValueContext Key(std::string key) = delete;
                BaseContext EndDict() = delete;
                Node Build() = delete;
        };
        
        // Гарантирует, что непосредственно после Key вызван Value, StartDict или StartArray.
        // Key() → Value(), StartDict(), StartArray()
        class DictValueContext : public BaseContext 
        {
            public:

                DictValueContext(BaseContext base) 
                    : BaseContext(base) 
                    {}
                
                DictItemContext Value(Node::Value value) 
                { 
                    return BaseContext::Value(std::move(value)); 
                }
                
                // => Поддерживает только методы Value, StartDict и StartArray, делегируемые в Builder
                DictValueContext Key(std::string key) = delete;
                BaseContext EndDict() = delete;
                BaseContext EndArray() = delete;
                Node Build() = delete;
        };
    };
}  // namespace json