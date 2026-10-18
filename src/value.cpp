#include "../include/value.hpp"
#include <sstream>
#include <cmath>

namespace Hammer {

Value::Value() : type(ValueType::VAL_NOTHING) {}

Value Value::makeNothing() {
    Value v;
    v.type = ValueType::VAL_NOTHING;
    return v;
}

Value Value::makeNumber(double num) {
    Value v;
    v.type = ValueType::VAL_NUMBER;
    v.numberVal = num;
    return v;
}

Value Value::makeString(std::string str) {
    Value v;
    v.type = ValueType::VAL_STRING;
    v.stringVal = std::move(str);
    return v;
}

Value Value::makeBoolean(bool b) {
    Value v;
    v.type = ValueType::VAL_BOOLEAN;
    v.boolVal = b;
    return v;
}

Value Value::makeList(std::vector<Value> items) {
    Value v;
    v.type = ValueType::VAL_LIST;
    v.listVal = std::make_shared<std::vector<Value>>(std::move(items));
    return v;
}

Value Value::makeMap() {
    Value v;
    v.type = ValueType::VAL_MAP;
    v.mapVal = std::make_shared<std::map<std::string, Value>>();
    return v;
}

Value Value::makeFunction(std::string name, std::vector<std::string> params, std::vector<std::shared_ptr<Stmt>> body, std::shared_ptr<Environment> closure) {
    Value v;
    v.type = ValueType::VAL_FUNCTION;
    v.funcVal = std::make_shared<FunctionData>();
    v.funcVal->name = std::move(name);
    v.funcVal->params = std::move(params);
    v.funcVal->body = std::move(body);
    v.funcVal->closure = std::move(closure);
    return v;
}

Value Value::makeNative(NativeFn fn) {
    Value v;
    v.type = ValueType::VAL_NATIVE;
    v.nativeVal = std::move(fn);
    return v;
}

bool Value::isTruthy() const {
    switch (type) {
        case ValueType::VAL_NOTHING: return false;
        case ValueType::VAL_BOOLEAN: return boolVal;
        case ValueType::VAL_NUMBER: return numberVal != 0.0;
        case ValueType::VAL_STRING: return !stringVal.empty();
        case ValueType::VAL_LIST: return listVal && !listVal->empty();
        case ValueType::VAL_MAP: return mapVal && !mapVal->empty();
        default: return true;
    }
}

std::string Value::typeName() const {
    switch (type) {
        case ValueType::VAL_NOTHING: return "nothing";
        case ValueType::VAL_NUMBER: return "number";
        case ValueType::VAL_STRING: return "string";
        case ValueType::VAL_BOOLEAN: return "boolean";
        case ValueType::VAL_LIST: return "list";
        case ValueType::VAL_MAP: return "map";
        case ValueType::VAL_FUNCTION: return "action";
        case ValueType::VAL_NATIVE: return "native_action";
        default: return "unknown";
    }
}

std::string Value::toString() const {
    switch (type) {
        case ValueType::VAL_NOTHING:
            return "nothing";
        case ValueType::VAL_BOOLEAN:
            return boolVal ? "true" : "false";
        case ValueType::VAL_NUMBER: {
            if (std::floor(numberVal) == numberVal && !std::isinf(numberVal) && !std::isnan(numberVal)) {
                return std::to_string(static_cast<long long>(numberVal));
            }
            std::string s = std::to_string(numberVal);
            // Trim unnecessary trailing zeros
            s.erase(s.find_last_not_of('0') + 1, std::string::npos);
            if (s.back() == '.') s.pop_back();
            return s;
        }
        case ValueType::VAL_STRING:
            return stringVal;
        case ValueType::VAL_LIST: {
            if (!listVal) return "[]";
            std::string res = "[";
            for (size_t i = 0; i < listVal->size(); ++i) {
                if (i > 0) res += ", ";
                const Value& item = (*listVal)[i];
                if (item.type == ValueType::VAL_STRING) {
                    res += "\"" + item.stringVal + "\"";
                } else {
                    res += item.toString();
                }
            }
            res += "]";
            return res;
        }
        case ValueType::VAL_MAP: {
            if (!mapVal) return "{}";
            std::string res = "{";
            bool first = true;
            for (const auto& pair : *mapVal) {
                if (!first) res += ", ";
                first = false;
                res += "\"" + pair.first + "\": ";
                if (pair.second.type == ValueType::VAL_STRING) {
                    res += "\"" + pair.second.stringVal + "\"";
                } else {
                    res += pair.second.toString();
                }
            }
            res += "}";
            return res;
        }
        case ValueType::VAL_FUNCTION:
            return "<action " + (funcVal ? funcVal->name : "") + ">";
        case ValueType::VAL_NATIVE:
            return "<builtin action>";
        default:
            return "";
    }
}

bool Value::operator==(const Value& other) const {
    if (type != other.type) {
        // Allow comparing number with float/int representation directly
        return false;
    }
    switch (type) {
        case ValueType::VAL_NOTHING: return true;
        case ValueType::VAL_BOOLEAN: return boolVal == other.boolVal;
        case ValueType::VAL_NUMBER: return numberVal == other.numberVal;
        case ValueType::VAL_STRING: return stringVal == other.stringVal;
        case ValueType::VAL_LIST: {
            if (listVal == other.listVal) return true;
            if (!listVal || !other.listVal) return false;
            if (listVal->size() != other.listVal->size()) return false;
            for (size_t i = 0; i < listVal->size(); ++i) {
                if ((*listVal)[i] != (*other.listVal)[i]) return false;
            }
            return true;
        }
        case ValueType::VAL_MAP: {
            if (mapVal == other.mapVal) return true;
            if (!mapVal || !other.mapVal) return false;
            if (mapVal->size() != other.mapVal->size()) return false;
            for (const auto& pair : *mapVal) {
                auto it = other.mapVal->find(pair.first);
                if (it == other.mapVal->end()) return false;
                if (pair.second != it->second) return false;
            }
            return true;
        }
        case ValueType::VAL_FUNCTION: return funcVal == other.funcVal;
        case ValueType::VAL_NATIVE: return false;
        default: return false;
    }
}

bool Value::operator!=(const Value& other) const {
    return !(*this == other);
}

} // namespace Hammer
