#include "../include/stdlib.hpp"
#include "../include/errors.hpp"
#include <cmath>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <random>
#include <algorithm>

namespace Hammer {

static std::mt19937& getRng() {
    static std::mt19937 rng(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
    return rng;
}

void registerStdLib(std::shared_ptr<Environment> env) {
    // length / len
    auto fnLength = [](const std::vector<Value>& args) -> Value {
        if (args.empty()) {
            throw HammerError(0, 0, "Type Error", "length() requires 1 argument.",
                "You called length() without any arguments.",
                "Pass a string, list, or map to get its length.", "length(\"Hello\")\nlength([1, 2, 3])");
        }
        const Value& v = args[0];
        if (v.type == ValueType::VAL_STRING) {
            return Value::makeNumber(static_cast<double>(v.stringVal.size()));
        }
        if (v.type == ValueType::VAL_LIST && v.listVal) {
            return Value::makeNumber(static_cast<double>(v.listVal->size()));
        }
        if (v.type == ValueType::VAL_MAP && v.mapVal) {
            return Value::makeNumber(static_cast<double>(v.mapVal->size()));
        }
        return Value::makeNumber(0.0);
    };
    env->set("length", Value::makeNative(fnLength));
    env->set("len", Value::makeNative(fnLength));

    // push / append
    auto fnPush = [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) {
            throw HammerError(0, 0, "Type Error", "push() requires 2 arguments: push(list, item).",
                "You called push() without providing both the list and the item to add.",
                "Call push with a list and the value you want to append.", "push(my_list, \"new item\")");
        }
        if (args[0].type != ValueType::VAL_LIST || !args[0].listVal) {
            throw HammerError(0, 0, "Type Error", "First argument of push() must be a list.",
                "You passed a " + args[0].typeName() + " instead of a list.",
                "Make sure the first argument is a list.", "push(my_list, 42)");
        }
        args[0].listVal->push_back(args[1]);
        return args[0];
    };
    env->set("push", Value::makeNative(fnPush));
    env->set("append", Value::makeNative(fnPush));

    // pop
    auto fnPop = [](const std::vector<Value>& args) -> Value {
        if (args.empty() || args[0].type != ValueType::VAL_LIST || !args[0].listVal) {
            throw HammerError(0, 0, "Type Error", "pop() requires a list as its argument.",
                "You didn't pass a valid list to pop().",
                "Pass a list to pop the last element.", "last = pop(my_list)");
        }
        if (args[0].listVal->empty()) {
            return Value::makeNothing();
        }
        Value last = args[0].listVal->back();
        args[0].listVal->pop_back();
        return last;
    };
    env->set("pop", Value::makeNative(fnPop));

    // random(min, max)
    auto fnRandom = [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) {
            throw HammerError(0, 0, "Type Error", "random() requires 2 arguments: random(min, max).",
                "You need to provide the minimum and maximum numbers.",
                "Call random with min and max numbers.", "secret = random(1, 100)");
        }
        long long minVal = static_cast<long long>(args[0].numberVal);
        long long maxVal = static_cast<long long>(args[1].numberVal);
        if (minVal > maxVal) std::swap(minVal, maxVal);
        std::uniform_int_distribution<long long> dist(minVal, maxVal);
        return Value::makeNumber(static_cast<double>(dist(getRng())));
    };
    env->set("random", Value::makeNative(fnRandom));

    // round, floor, ceil, abs, sqrt
    env->set("round", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeNumber(0);
        return Value::makeNumber(std::round(args[0].numberVal));
    }));
    env->set("floor", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeNumber(0);
        return Value::makeNumber(std::floor(args[0].numberVal));
    }));
    env->set("ceil", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeNumber(0);
        return Value::makeNumber(std::ceil(args[0].numberVal));
    }));
    env->set("abs", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeNumber(0);
        return Value::makeNumber(std::abs(args[0].numberVal));
    }));
    env->set("sqrt", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeNumber(0);
        return Value::makeNumber(std::sqrt(args[0].numberVal));
    }));

    // uppercase, lowercase, trim
    env->set("uppercase", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeString("");
        std::string s = args[0].toString();
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
        return Value::makeString(s);
    }));
    env->set("lowercase", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeString("");
        std::string s = args[0].toString();
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
        return Value::makeString(s);
    }));
    env->set("trim", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeString("");
        std::string s = args[0].toString();
        size_t first = s.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return Value::makeString("");
        size_t last = s.find_last_not_of(" \t\r\n");
        return Value::makeString(s.substr(first, (last - first + 1)));
    }));

    // type_of
    env->set("type_of", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeString("nothing");
        return Value::makeString(args[0].typeName());
    }));

    // to_number & to_string
    env->set("to_number", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeNumber(0);
        if (args[0].type == ValueType::VAL_NUMBER) return args[0];
        try {
            return Value::makeNumber(std::stod(args[0].toString()));
        } catch (...) {
            return Value::makeNumber(0);
        }
    }));
    env->set("to_string", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeString("");
        return Value::makeString(args[0].toString());
    }));

    // sleep(ms)
    env->set("sleep", Value::makeNative([](const std::vector<Value>& args) -> Value {
        int ms = args.empty() ? 100 : static_cast<int>(args[0].numberVal);
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        return Value::makeNothing();
    }));

    // time() in seconds
    env->set("time", Value::makeNative([](const std::vector<Value>&) -> Value {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        double secs = std::chrono::duration<double>(now).count();
        return Value::makeNumber(secs);
    }));

    // read_file(path)
    env->set("read_file", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeString("");
        std::string path = args[0].toString();
        std::ifstream file(path);
        if (!file.is_open()) {
            throw HammerError(0, 0, "IO Error", "Cannot open file '" + path + "' for reading.",
                "The file path does not exist or you lack read permissions.",
                "Check that the file path is correct.", "content = read_file(\"data.txt\")");
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return Value::makeString(buffer.str());
    }));

    // write_file(path, content)
    env->set("write_file", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) {
            throw HammerError(0, 0, "Type Error", "write_file requires 2 arguments: write_file(path, content).",
                "You called write_file without providing path and content.",
                "Pass both the file path and content string.", "write_file(\"notes.txt\", \"Hello!\")");
        }
        std::string path = args[0].toString();
        std::string content = args[1].toString();
        std::ofstream file(path);
        if (!file.is_open()) {
            throw HammerError(0, 0, "IO Error", "Cannot open file '" + path + "' for writing.",
                "Failed to open or create file for writing.",
                "Verify directory exists and you have write permissions.", "write_file(\"output.txt\", \"data\")");
        }
        file << content;
        return Value::makeBoolean(true);
    }));

    // file_exists(path)
    env->set("file_exists", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeBoolean(false);
        std::ifstream file(args[0].toString());
        return Value::makeBoolean(file.good());
    }));

    // keys(map)
    env->set("keys", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty() || args[0].type != ValueType::VAL_MAP || !args[0].mapVal) {
            return Value::makeList({});
        }
        std::vector<Value> kList;
        for (const auto& pair : *args[0].mapVal) {
            kList.push_back(Value::makeString(pair.first));
        }
        return Value::makeList(kList);
    }));

    // values(map)
    env->set("values", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty() || args[0].type != ValueType::VAL_MAP || !args[0].mapVal) {
            return Value::makeList({});
        }
        std::vector<Value> vList;
        for (const auto& pair : *args[0].mapVal) {
            vList.push_back(pair.second);
        }
        return Value::makeList(vList);
    }));

    // has_key(map, key)
    env->set("has_key", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || args[0].type != ValueType::VAL_MAP || !args[0].mapVal) {
            return Value::makeBoolean(false);
        }
        return Value::makeBoolean(args[0].mapVal->find(args[1].toString()) != args[0].mapVal->end());
    }));

    // split(str, delim)
    env->set("split", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeList({});
        std::string s = args[0].toString();
        std::string delim = args.size() > 1 ? args[1].toString() : " ";
        std::vector<Value> result;
        if (delim.empty()) {
            for (char c : s) result.push_back(Value::makeString(std::string(1, c)));
            return Value::makeList(result);
        }
        size_t start = 0;
        size_t end = s.find(delim);
        while (end != std::string::npos) {
            result.push_back(Value::makeString(s.substr(start, end - start)));
            start = end + delim.length();
            end = s.find(delim, start);
        }
        result.push_back(Value::makeString(s.substr(start)));
        return Value::makeList(result);
    }));

    // join(list, delim)
    env->set("join", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty() || args[0].type != ValueType::VAL_LIST || !args[0].listVal) {
            return Value::makeString("");
        }
        std::string delim = args.size() > 1 ? args[1].toString() : "";
        std::string res;
        for (size_t i = 0; i < args[0].listVal->size(); ++i) {
            if (i > 0) res += delim;
            res += (*args[0].listVal)[i].toString();
        }
        return Value::makeString(res);
    }));

    // replace(str, target, repl)
    env->set("replace", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.size() < 3) return args.empty() ? Value::makeString("") : args[0];
        std::string s = args[0].toString();
        std::string target = args[1].toString();
        std::string repl = args[2].toString();
        if (target.empty()) return Value::makeString(s);
        size_t pos = 0;
        while ((pos = s.find(target, pos)) != std::string::npos) {
            s.replace(pos, target.length(), repl);
            pos += repl.length();
        }
        return Value::makeString(s);
    }));

    // to_upper & to_lower
    env->set("to_upper", env->get("uppercase"));
    env->set("to_lower", env->get("lowercase"));

    // contains(target, query)
    env->set("contains", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::makeBoolean(false);
        const Value& target = args[0];
        const Value& query = args[1];
        if (target.type == ValueType::VAL_STRING) {
            return Value::makeBoolean(target.stringVal.find(query.toString()) != std::string::npos);
        }
        if (target.type == ValueType::VAL_LIST && target.listVal) {
            for (const auto& item : *target.listVal) {
                if (item.toString() == query.toString()) return Value::makeBoolean(true);
            }
            return Value::makeBoolean(false);
        }
        if (target.type == ValueType::VAL_MAP && target.mapVal) {
            return Value::makeBoolean(target.mapVal->find(query.toString()) != target.mapVal->end());
        }
        return Value::makeBoolean(false);
    }));

    // starts_with(str, prefix)
    env->set("starts_with", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::makeBoolean(false);
        std::string s = args[0].toString();
        std::string prefix = args[1].toString();
        return Value::makeBoolean(s.rfind(prefix, 0) == 0);
    }));

    // ends_with(str, suffix)
    env->set("ends_with", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::makeBoolean(false);
        std::string s = args[0].toString();
        std::string suffix = args[1].toString();
        if (suffix.size() > s.size()) return Value::makeBoolean(false);
        return Value::makeBoolean(std::equal(suffix.rbegin(), suffix.rend(), s.rbegin()));
    }));

    // insert(list, index, val)
    env->set("insert", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.size() < 3 || args[0].type != ValueType::VAL_LIST || !args[0].listVal) {
            return args.empty() ? Value::makeNothing() : args[0];
        }
        int idx = static_cast<int>(args[1].numberVal);
        idx = std::clamp(idx, 0, static_cast<int>(args[0].listVal->size()));
        args[0].listVal->insert(args[0].listVal->begin() + idx, args[2]);
        return args[0];
    }));

    // remove_at(list, index)
    env->set("remove_at", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.size() < 2 || args[0].type != ValueType::VAL_LIST || !args[0].listVal) {
            return Value::makeNothing();
        }
        int idx = static_cast<int>(args[1].numberVal);
        if (idx < 0 || idx >= static_cast<int>(args[0].listVal->size())) return Value::makeNothing();
        Value removed = (*args[0].listVal)[idx];
        args[0].listVal->erase(args[0].listVal->begin() + idx);
        return removed;
    }));

    // sort(list)
    env->set("sort", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty() || args[0].type != ValueType::VAL_LIST || !args[0].listVal) {
            return args.empty() ? Value::makeNothing() : args[0];
        }
        std::sort(args[0].listVal->begin(), args[0].listVal->end(), [](const Value& a, const Value& b) {
            if (a.type == ValueType::VAL_NUMBER && b.type == ValueType::VAL_NUMBER) {
                return a.numberVal < b.numberVal;
            }
            return a.toString() < b.toString();
        });
        return args[0];
    }));

    // reverse(list)
    env->set("reverse", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty() || args[0].type != ValueType::VAL_LIST || !args[0].listVal) {
            return args.empty() ? Value::makeNothing() : args[0];
        }
        std::reverse(args[0].listVal->begin(), args[0].listVal->end());
        return args[0];
    }));

    // JSON Parser helper class
    class JsonParser {
        std::string s;
        size_t pos = 0;
    public:
        explicit JsonParser(std::string str) : s(std::move(str)) {}

        void skipWs() {
            while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\r' || s[pos] == '\n')) pos++;
        }

        Value parse() {
            skipWs();
            if (pos >= s.size()) return Value::makeNothing();
            char c = s[pos];
            if (c == '{') return parseObject();
            if (c == '[') return parseArray();
            if (c == '"') return parseString();
            if (c == 't' || c == 'f') return parseBool();
            if (c == 'n') return parseNull();
            if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parseNumber();
            return Value::makeNothing();
        }

        Value parseObject() {
            pos++; // consume '{'
            Value m = Value::makeMap();
            skipWs();
            if (pos < s.size() && s[pos] == '}') { pos++; return m; }
            while (pos < s.size()) {
                skipWs();
                if (pos >= s.size() || s[pos] != '"') break;
                Value keyVal = parseString();
                skipWs();
                if (pos < s.size() && s[pos] == ':') pos++;
                skipWs();
                Value val = parse();
                (*m.mapVal)[keyVal.stringVal] = val;
                skipWs();
                if (pos < s.size() && s[pos] == ',') { pos++; continue; }
                if (pos < s.size() && s[pos] == '}') { pos++; break; }
                break;
            }
            return m;
        }

        Value parseArray() {
            pos++; // consume '['
            std::vector<Value> items;
            skipWs();
            if (pos < s.size() && s[pos] == ']') { pos++; return Value::makeList(items); }
            while (pos < s.size()) {
                skipWs();
                items.push_back(parse());
                skipWs();
                if (pos < s.size() && s[pos] == ',') { pos++; continue; }
                if (pos < s.size() && s[pos] == ']') { pos++; break; }
                break;
            }
            return Value::makeList(items);
        }

        Value parseString() {
            pos++; // consume '"'
            std::string res;
            while (pos < s.size() && s[pos] != '"') {
                if (s[pos] == '\\' && pos + 1 < s.size()) {
                    pos++;
                    char esc = s[pos++];
                    switch (esc) {
                        case 'n': res += '\n'; break;
                        case 't': res += '\t'; break;
                        case 'r': res += '\r'; break;
                        case '"': res += '"'; break;
                        case '\\': res += '\\'; break;
                        default: res += esc; break;
                    }
                } else {
                    res += s[pos++];
                }
            }
            if (pos < s.size() && s[pos] == '"') pos++;
            return Value::makeString(res);
        }

        Value parseNumber() {
            size_t start = pos;
            if (s[pos] == '-') pos++;
            while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) pos++;
            if (pos < s.size() && s[pos] == '.') {
                pos++;
                while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) pos++;
            }
            if (pos < s.size() && (s[pos] == 'e' || s[pos] == 'E')) {
                pos++;
                if (pos < s.size() && (s[pos] == '+' || s[pos] == '-')) pos++;
                while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) pos++;
            }
            double n = 0.0;
            try { n = std::stod(s.substr(start, pos - start)); } catch (...) {}
            return Value::makeNumber(n);
        }

        Value parseBool() {
            if (s.substr(pos, 4) == "true") { pos += 4; return Value::makeBoolean(true); }
            if (s.substr(pos, 5) == "false") { pos += 5; return Value::makeBoolean(false); }
            return Value::makeBoolean(false);
        }

        Value parseNull() {
            if (s.substr(pos, 4) == "null") pos += 4;
            return Value::makeNothing();
        }
    };

    struct JsonEncoder {
        static std::string encode(const Value& v) {
            switch (v.type) {
                case ValueType::VAL_NOTHING: return "null";
                case ValueType::VAL_BOOLEAN: return v.boolVal ? "true" : "false";
                case ValueType::VAL_NUMBER: {
                    if (std::floor(v.numberVal) == v.numberVal && !std::isinf(v.numberVal) && !std::isnan(v.numberVal)) {
                        return std::to_string(static_cast<long long>(v.numberVal));
                    }
                    std::string s = std::to_string(v.numberVal);
                    s.erase(s.find_last_not_of('0') + 1, std::string::npos);
                    if (s.back() == '.') s.pop_back();
                    return s;
                }
                case ValueType::VAL_STRING: {
                    std::string esc = "\"";
                    for (char c : v.stringVal) {
                        if (c == '"') esc += "\\\"";
                        else if (c == '\\') esc += "\\\\";
                        else if (c == '\n') esc += "\\n";
                        else if (c == '\t') esc += "\\t";
                        else if (c == '\r') esc += "\\r";
                        else esc += c;
                    }
                    esc += "\"";
                    return esc;
                }
                case ValueType::VAL_LIST: {
                    if (!v.listVal) return "[]";
                    std::string res = "[";
                    for (size_t i = 0; i < v.listVal->size(); ++i) {
                        if (i > 0) res += ", ";
                        res += encode((*v.listVal)[i]);
                    }
                    res += "]";
                    return res;
                }
                case ValueType::VAL_MAP: {
                    if (!v.mapVal) return "{}";
                    std::string res = "{";
                    bool first = true;
                    for (const auto& pair : *v.mapVal) {
                        if (!first) res += ", ";
                        first = false;
                        res += "\"" + pair.first + "\": " + encode(pair.second);
                    }
                    res += "}";
                    return res;
                }
                default: return "null";
            }
        }
    };

    // json_parse & json_encode
    env->set("json_parse", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeNothing();
        JsonParser parser(args[0].toString());
        return parser.parse();
    }));

    env->set("json_encode", Value::makeNative([](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeString("null");
        return Value::makeString(JsonEncoder::encode(args[0]));
    }));
}

} // namespace Hammer
