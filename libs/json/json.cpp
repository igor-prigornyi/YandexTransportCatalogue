#include <array>
#include "json.h"
using namespace std;

namespace json {

Node::Node()             : value_(nullptr)     { }
Node::Node(nullptr_t)    : value_(nullptr)     { }
Node::Node(Array array)  : value_(move(array)) { }
Node::Node(Dict map)     : value_(move(map))   { }
Node::Node(bool value)   : value_(value)       { }
Node::Node(int value)    : value_(value)       { }
Node::Node(double value) : value_(value)       { }
Node::Node(string value) : value_(move(value)) { }

const Node::Value& Node::GetValue() const {
	return value_;
}

bool Node::IsNull() const {
	return holds_alternative<nullptr_t>(value_);
}

bool Node::IsArray() const {
	return holds_alternative<Array>(value_);
}

bool Node::IsMap() const {
	return holds_alternative<Dict>(value_);
}

bool Node::IsBool() const {
	return holds_alternative<bool>(value_);
}

bool Node::IsInt() const {
	return holds_alternative<int>(value_);
}

bool Node::IsDouble() const {
	return holds_alternative<int>(value_) || holds_alternative<double>(value_); 
}

bool Node::IsPureDouble() const {
	return holds_alternative<double>(value_); 
}

bool Node::IsString() const {
	return holds_alternative<string>(value_); 
}

const Array& Node::AsArray() const {
	if (!IsArray()) throw logic_error("Node is not an Array"s);
	return get<Array>(value_);
}

const Dict& Node::AsMap() const {
	if (!IsMap()) throw logic_error("Node is not a Map"s);
	return get<Dict>(value_);
}

bool Node::AsBool() const {
	if (!IsBool()) throw logic_error("Node is not a Bool"s);
	return get<bool>(value_);
}

int Node::AsInt() const {
	if (!IsInt()) throw logic_error("Node is not an Int"s);
	return get<int>(value_);
}

double Node::AsDouble() const {
	if (!IsDouble()) throw logic_error("Node is not a Double (and not is Int)"s);

	if (IsInt()) return static_cast<double>(get<int>(value_));
	else         return get<double>(value_);
}

const string& Node::AsString() const {
	if (!IsString()) throw logic_error("Node is not a String"s);
	return get<string>(value_);
}

bool operator == (const Node& lhs, const Node& rhs) {
	return lhs.GetValue() == rhs.GetValue();
}

bool operator != (const Node& lhs, const Node& rhs) {
	return !(lhs == rhs);
}

Document::Document(Node root) : root_(move(root)) { }

const Node& Document::GetRoot() const { return root_; }

bool operator == (const Document& lhs, const Document& rhs) {
	return lhs.GetRoot() == rhs.GetRoot();
}

bool operator != (const Document& lhs, const Document& rhs) {
	return !(lhs == rhs);
}

namespace detail {

enum class LexemeType { NOTHING, KEY, VALUE, COMMA, COLON };

char PeekFirstNonSpaceCharFromStream(istream& input) {
	char ch = input.peek();

	while(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
		input.get();
		ch = input.peek();
	}

	return ch;
}

Node LoadNode(istream& input);

Node LoadNumber(istream& input) {
	string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) throw ParsingError("Failed to read number from stream"s);
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!isdigit(input.peek())) throw ParsingError("A digit is expected"s);

        while (isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') read_char();

    // Парсим целую часть числа
    // После 0 в JSON не могут идти другие цифры
    if (input.peek() == '0') read_char();
    else read_digits();

    bool is_int = true;

    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(stoi(parsed_num));
            }
			catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }

        return Node(stod(parsed_num));
    }
	catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(istream& input) {

	if (input.get() != '\"') throw ParsingError("String parsing error"s);

	auto it  = istreambuf_iterator<char>(input);
	auto end = istreambuf_iterator<char>();
	string parsed_str;

	while (true) {

		// Поток закончился до того, как встретили закрывающую кавычку?
		if (it == end)  throw ParsingError("String parsing error"s);

		const char ch = *it;

		// Встретили закрывающую кавычку
		if (ch == '\"') { ++it; break; }
		
		// Встретили начало escape-последовательности
		else if (ch == '\\') {
			++it;

			// Поток завершился сразу после символа обратной косой черты
			if (it == end) throw ParsingError("String parsing error"s);

			const char escaped_char = *(it);

			// Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
			switch (escaped_char) {
				case 'n':  parsed_str.push_back('\n'); break;
				case 't':  parsed_str.push_back('\t'); break;
				case 'r':  parsed_str.push_back('\r'); break;
				case '\"': parsed_str.push_back('\"'); break;
				case '\\': parsed_str.push_back('\\'); break;

				// Встретили неизвестную escape-последовательность
				default: throw ParsingError("Unrecognized escape sequence \\"s + escaped_char); break;
			}
		}

		// Строковый литерал внутри- JSON не может прерываться символами \r или \n
		else if (ch == '\n' || ch == '\r') throw ParsingError("Unexpected end of line"s);

		// Просто считываем очередной символ и помещаем его в результирующую строку
		else parsed_str.push_back(ch);

		++it;
	}

	return Node(move(parsed_str));
}

Node LoadArray(istream& input) {

	// Ожидается открывающая скобка "["
	if (input.get() != '[') throw ParsingError("Scope \"[\" in the array is not opened"s);

	vector<Node> result;
	LexemeType last_read = LexemeType::NOTHING;

	while(true) {
		char ch = PeekFirstNonSpaceCharFromStream(input);

		if(ch == ']') {
			// Закрывающая скобка "]" может быть после элемента массива либо сразу после открывающей скобки "["
			if(last_read == LexemeType::VALUE || last_read == LexemeType::NOTHING) {
				input.get();
				break;
			}
			// Закрывающая скобка "]" после запятой, ожидался элемент массива
			else throw ParsingError("Expected value after comma before scope \"]\" in array"s);
		}
		else if(ch == ',') {
			// Запятая может быть только после элемента массива
			if(last_read == LexemeType::VALUE) {
				input.get();
				last_read = LexemeType::COMMA;
			}
			// Запятая после запятой, ожидался элемент массива
			else if (last_read == LexemeType::COMMA) throw ParsingError("Expected value between two commas in array"s);
			// Запятая после открывающей скобки "[", ожидался элемент массива
			else                                     throw ParsingError("Expected value before comma in array"s);
		}
		else {
			// Элемент массива может быть после запятой либо после открывающей скобки "["
			if(last_read == LexemeType::COMMA || last_read == LexemeType::NOTHING) {
				result.push_back(move(LoadNode(input)));
				last_read = LexemeType::VALUE;
			}
			// Элемент массива после элемента массива без запятой, ожидалась запятая
			else throw ParsingError("Comma expected after value in array"s);
		}
	}

	return Node(move(result));
}

Node LoadDict(istream& input) {

	// Ожидается открывающая скобка "{"
	if (input.get() != '{') throw ParsingError("Scope \"{\" in the dictionary is not opened"s);

	map<string, Node> result;
	string tmp_key;
	Node   tmp_value;

	LexemeType last_read = LexemeType::NOTHING;

	while(true) {
		char ch = PeekFirstNonSpaceCharFromStream(input);

		if(ch == '}') {
			// Закрывающая скобка "}" может быть после значения либо сразу после открывающей скобки "{"
			if(last_read == LexemeType::VALUE || last_read == LexemeType::NOTHING) {
				input.get();
				break;
			}
			// Закрывающая скобка "}" после ключа, ожидалось двоеточие и значение
			else if(last_read == LexemeType::KEY)   throw ParsingError("Expected colon and value after key before scope \"}\" in dictionary"s);
			// Закрывающая скобка "}" после запятой, ожидалась пара ключ-значение
			else if(last_read == LexemeType::COMMA) throw ParsingError("Expected key-value pair after comma before scope \"}\" in dictionary"s);
			// Закрывающая скобка "}" после двоеточия, ожидалось значение
			else                                    throw ParsingError("Expected value after colon before scope \"}\" in dictionary"s);
		}
		else if(ch == ',') {
			// Запятая может быть только после значения (пары ключ-значение)
			if(last_read == LexemeType::VALUE) {
				input.get();
				last_read = LexemeType::COMMA;
			}
			// Запятая после открывающей скобки "{", ожидалась пара ключ-значение
			else if(last_read == LexemeType::NOTHING) throw ParsingError("Expected key-value pair before comma in dictionary"s);
			// Запятая после ключа, ожидалось двоеточие
			else if(last_read == LexemeType::KEY)     throw ParsingError("Expected colon after key in dictionary"s);
			// Запятая после запятой, ожидалась пара ключ-значение
			else if(last_read == LexemeType::COMMA)   throw ParsingError("Expected key-value pair between two commas in dictionary"s);
			// Запятая после двоеточия, ожидалось значение 
			else                                      throw ParsingError("Expected value after colon in dictionary"s);

		}
		else if(ch == ':') {
			// Двоеточие может быть только после ключа
			if(last_read == LexemeType::KEY) {
				input.get();
				last_read = LexemeType::COLON;
			}
			// Двоеточие после открывающей скобки "{", ожидалась пара ключ-значение или закрывающая скобка "}"
			else if(last_read == LexemeType::NOTHING) throw ParsingError("Expected key-value pair or scope \"}\" after scope \"{\" in dictionary"s);
			// Двоеточие после значения, ожидалась запятая или закрывающая скобка "}"
			else if(last_read == LexemeType::VALUE)   throw ParsingError("Expected comma or scope \"}\" after key-value pair in dictionary"s);
			// Двоеточие после запятой, ожидалась пара ключ-значение
			else if(last_read == LexemeType::COMMA)   throw ParsingError("Expected key-value pair after comma in dictionary"s);
			// Двоеточие после двоеточия, ожидалось значение
			else                                      throw ParsingError("Expected value after colon in dictionary"s);
		}
		else {
			// Если последний прочитанный тип - запятая или открывающая скобка "{", читаем ключ
			if(last_read == LexemeType::COMMA || last_read == LexemeType::NOTHING) {
				tmp_key = LoadString(input).AsString();
				last_read = LexemeType::KEY;
			}
			// Если последний прочитанный тип - двоеточие, читаем значение
			else if(last_read == LexemeType::COLON) {
				tmp_value = LoadNode(input);
				last_read = LexemeType::VALUE;
				result.insert({ move(tmp_key), move(tmp_value)});
			}
			// Ключ или значение после ключа, ожидалось двоеточие
			else if(last_read == LexemeType::KEY) throw ParsingError("Expected colon after key in dictionary"s);
			// Ключ или значение после значения, ожидалась запятая или закрывающая скобка "}"
			else                                  throw ParsingError("Expected comma or scope \"}\" after key-value pair in dictionary"s);
		}
	}
	return Node(move(result));
}

Node LoadNull(istream& input) {
	array<char, 4> chars;
	for(char& ch : chars) ch = input.get();

	if (chars[0] == 'n' && chars[1] == 'u' && chars[2] == 'l' && chars[3] == 'l') return Node(nullptr);
	else throw ParsingError("Null parsing error"s);
}

Node LoadBool(istream& input) {
	array<char, 4> chars;
	for(char& ch : chars) ch = input.get();

	if      (chars[0] == 't' && chars[1] == 'r' && chars[2] == 'u' && chars[3] == 'e')                       return Node(true);
	else if (chars[0] == 'f' && chars[1] == 'a' && chars[2] == 'l' && chars[3] == 's' && input.get() == 'e') return Node(false);
	else throw ParsingError("Bool parsing error"s);
}

Node LoadNode(istream& input) {

	char ch = PeekFirstNonSpaceCharFromStream(input);

	switch (ch) {
		case '[':  return LoadArray (input); break;
		case '{':  return LoadDict  (input); break;
		case '\"': return LoadString(input); break;
		case 'n':  return LoadNull  (input); break;
		case 't':  return LoadBool  (input); break;
		case 'f':  return LoadBool  (input); break;
		default:   return LoadNumber(input); break;
	}
}

}

Document::Document(istream& input) : Document(detail::LoadNode(input)) { }

namespace detail {

struct PrintContext {
	ostream& out;
	size_t indent_step = 4u;
	size_t indent = 0u;
	string prefix = ""s;

	void PrintIndentWithPrefix() const {
		for (size_t i = 0; i < indent; ++i) out.put(' ');
		out << prefix;
	}

	void PrintIndentWithSpacedPrefix() const {
		for (size_t i = 0; i < indent + prefix.size(); ++i) out.put(' ');
	}

	PrintContext Indented() const {
		return { out, indent_step, indent + indent_step + prefix.size()};
	}

	PrintContext Indented(const string& new_prefix) const {
		return { out, indent_step, indent + indent_step + prefix.size(), new_prefix};
	}
};

void PrintNode(const Node& node, PrintContext ctx);

struct NodePrinter {

	const PrintContext& ctx;

	void operator() (nullptr_t) {
		ctx.PrintIndentWithPrefix();
		ctx.out << "null"sv;
	}

	void operator() (const Array& array) {
		ctx.PrintIndentWithPrefix();
		ctx.out << "["sv << endl;

    	bool first = true;
    	for (const Node& node : array) {
        	if (!first) ctx.out << ","sv << endl;
        	else        first = false;

			PrintNode(node, ctx.Indented());
    	}

		ctx.out << endl;
		ctx.PrintIndentWithSpacedPrefix();
		ctx.out << "]"sv;
	}

	void operator() (const Dict& map) {
		ctx.PrintIndentWithPrefix();
		ctx.out << "{"sv << endl;

    	bool first = true;
    	for (const auto& [key, value] : map) {
        	if (!first) ctx.out << ", "sv << endl;
        	else        first = false;

			const string key_prefix = "\""s + key + "\": "s;
			PrintNode(value, ctx.Indented(key_prefix));
    	}

		ctx.out << endl;
		ctx.PrintIndentWithSpacedPrefix();
		ctx.out << "}"sv;
	}

	void operator() (bool value) {
		ctx.PrintIndentWithPrefix();
		ctx.out << (value ? "true"sv : "false"sv);
	}

	void operator() (int value) {
		ctx.PrintIndentWithPrefix();
		ctx.out << value;
	}

	void operator() (double value) {
		ctx.PrintIndentWithPrefix();
		ctx.out << value;
	}

	void operator() (const string& value) {
		ctx.PrintIndentWithPrefix();
		ctx.out << "\""sv;

		for (const char& c : value) {
			if      (c == '\n') ctx.out << "\\n"sv;
			else if (c == '\r') ctx.out << "\\r"sv;
			else if (c == '\"') ctx.out << "\\\""sv;
			else if (c == '\\') ctx.out << "\\\\"sv;
			else ctx.out << c;
		}

		ctx.out << "\""sv;
	}
};

void PrintNode(const Node& node, const PrintContext ctx) {
	visit(NodePrinter{ctx}, node.GetValue());
}

}

void Document::Print(ostream& output) const {
	detail::PrintNode(GetRoot(), { output, 4, 0 });
}

}