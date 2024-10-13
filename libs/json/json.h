#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <variant>

namespace json {

class Node;

using Array = std::vector<Node>;
using Dict  = std::map<std::string, Node>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    Node();
    Node(std::nullptr_t);
    Node(Array array);
    Node(Dict map);
    Node(bool value);
    Node(int value);
    Node(double value);
    Node(std::string value);

    const Value& GetValue() const;

    bool IsNull()       const;
    bool IsArray()      const;
    bool IsMap()        const;
    bool IsBool()       const;
    bool IsInt()        const;
    bool IsDouble()     const;
    bool IsPureDouble() const;
    bool IsString()     const;
    
    const Array&       AsArray()  const;
    const Dict&        AsMap()    const;
    bool               AsBool()   const;
    int                AsInt()    const;
    double             AsDouble() const;
    const std::string& AsString() const;

private:
    Value value_;
};

bool operator == (const Node& lhs, const Node& rhs);
bool operator != (const Node& lhs, const Node& rhs);

class Document {
public:
    explicit Document(Node root);
    explicit Document(std::istream& input);
    const Node& GetRoot() const;
    void Print(std::ostream& output) const;

private:
    Node root_;
};

bool operator == (const Document& lhs, const Document& rhs);
bool operator != (const Document& lhs, const Document& rhs);


}