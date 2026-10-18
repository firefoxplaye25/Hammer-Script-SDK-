#ifndef HAMMER_VALUE_HPP
#define HAMMER_VALUE_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace Hammer {

class Environment;
struct Stmt;

enum class ValueType {
    VAL_NOTHING,
    VAL_NUMBER,
    VAL_STRING,
    VAL_BOOLEAN,
    VAL_LIST,
    VAL_MAP,
    VAL_FUNCTION,
    VAL_NATIVE
};

class Value;

using NativeFn = std::function<Value(const std::vector<Value>&)>;

struct FunctionData {
    std::string name;
    std::vector<std::string> params;
    std::vector<std::shared_ptr<Stmt>> body;
    std::shared_ptr<Environment> closure;
};

class Value {
public:
    ValueType type;

    double numberVal = 0.0;
    std::string stringVal;
    bool boolVal = false;
    std::shared_ptr<std::vector<Value>> listVal;
    std::shared_ptr<std::map<std::string, Value>> mapVal;
    std::shared_ptr<FunctionData> funcVal;
    NativeFn nativeVal;

    Value(); // creates NOTHING
    static Value makeNothing();
    static Value makeNumber(double num);
    static Value makeString(std::string str);
    static Value makeBoolean(bool b);
    static Value makeList(std::vector<Value> items = {});
    static Value makeMap();
    static Value makeFunction(std::string name, std::vector<std::string> params, std::vector<std::shared_ptr<Stmt>> body, std::shared_ptr<Environment> closure);
    static Value makeNative(NativeFn fn);

    bool isTruthy() const;
    std::string toString() const;
    std::string typeName() const;

    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const;
};

} // namespace Hammer

#endif // HAMMER_VALUE_HPP
