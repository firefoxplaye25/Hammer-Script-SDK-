#include "../include/hmcanvas.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

namespace Hammer {

static int canvasWidth = 60;
static int canvasHeight = 20;
static std::vector<std::string> canvasBuffer(20, std::string(60, ' '));

void registerHmCanvas(std::shared_ptr<Environment> env) {
    // 1. hmcanvas_init(w, h)
    auto fnInit = [](const std::vector<Value>& args) -> Value {
        int w = (args.size() > 0) ? static_cast<int>(args[0].numberVal) : 60;
        int h = (args.size() > 1) ? static_cast<int>(args[1].numberVal) : 20;
        if (w < 1) w = 10;
        if (h < 1) h = 5;
        canvasWidth = w;
        canvasHeight = h;
        canvasBuffer.assign(canvasHeight, std::string(canvasWidth, ' '));
        return Value::makeBoolean(true);
    };

    // 2. hmcanvas_clear(fill_char)
    auto fnClear = [](const std::vector<Value>& args) -> Value {
        char fill = ' ';
        if (!args.empty() && !args[0].toString().empty()) {
            fill = args[0].toString()[0];
        }
        canvasBuffer.assign(canvasHeight, std::string(canvasWidth, fill));
        return Value::makeBoolean(true);
    };

    // 3. hmcanvas_set(x, y, char_str)
    auto fnSet = [](const std::vector<Value>& args) -> Value {
        if (args.size() >= 3) {
            int x = static_cast<int>(args[0].numberVal);
            int y = static_cast<int>(args[1].numberVal);
            std::string ch = args[2].toString();
            if (y >= 0 && y < canvasHeight && x >= 0 && x < canvasWidth && !ch.empty()) {
                canvasBuffer[y][x] = ch[0];
                return Value::makeBoolean(true);
            }
        }
        return Value::makeBoolean(false);
    };

    // 4. hmcanvas_text(x, y, text)
    auto fnText = [](const std::vector<Value>& args) -> Value {
        if (args.size() >= 3) {
            int x = static_cast<int>(args[0].numberVal);
            int y = static_cast<int>(args[1].numberVal);
            std::string txt = args[2].toString();
            if (y >= 0 && y < canvasHeight) {
                for (size_t i = 0; i < txt.size(); ++i) {
                    int px = x + static_cast<int>(i);
                    if (px >= 0 && px < canvasWidth) {
                        canvasBuffer[y][px] = txt[i];
                    }
                }
                return Value::makeBoolean(true);
            }
        }
        return Value::makeBoolean(false);
    };

    // 5. hmcanvas_box(x, y, w, h, border_char)
    auto fnBox = [](const std::vector<Value>& args) -> Value {
        if (args.size() >= 4) {
            int x = static_cast<int>(args[0].numberVal);
            int y = static_cast<int>(args[1].numberVal);
            int w = static_cast<int>(args[2].numberVal);
            int h = static_cast<int>(args[3].numberVal);
            char b = '+';
            if (args.size() >= 5 && !args[4].toString().empty()) b = args[4].toString()[0];

            for (int cy = y; cy < y + h; ++cy) {
                if (cy < 0 || cy >= canvasHeight) continue;
                for (int cx = x; cx < x + w; ++cx) {
                    if (cx < 0 || cx >= canvasWidth) continue;
                    if (cy == y || cy == y + h - 1 || cx == x || cx == x + w - 1) {
                        canvasBuffer[cy][cx] = b;
                    }
                }
            }
            return Value::makeBoolean(true);
        }
        return Value::makeBoolean(false);
    };

    // 6. hmcanvas_render()
    auto fnRender = [](const std::vector<Value>& args) -> Value {
        for (int y = 0; y < canvasHeight; ++y) {
            std::cout << canvasBuffer[y] << "\n";
        }
        return Value::makeBoolean(true);
    };

    // 7. hmcanvas_to_string()
    auto fnToString = [](const std::vector<Value>& args) -> Value {
        std::string res;
        for (int y = 0; y < canvasHeight; ++y) {
            res += canvasBuffer[y];
            if (y + 1 < canvasHeight) res += "\n";
        }
        return Value::makeString(res);
    };

    // Register global functions
    env->set("hmcanvas_init", Value::makeNative(fnInit));
    env->set("hmcanvas_clear", Value::makeNative(fnClear));
    env->set("hmcanvas_set", Value::makeNative(fnSet));
    env->set("hmcanvas_text", Value::makeNative(fnText));
    env->set("hmcanvas_box", Value::makeNative(fnBox));
    env->set("hmcanvas_render", Value::makeNative(fnRender));
    env->set("hmcanvas_to_string", Value::makeNative(fnToString));

    // Register namespace object: hmcanvas
    Value obj = Value::makeMap();
    (*obj.mapVal)["init"] = Value::makeNative(fnInit);
    (*obj.mapVal)["clear"] = Value::makeNative(fnClear);
    (*obj.mapVal)["set"] = Value::makeNative(fnSet);
    (*obj.mapVal)["text"] = Value::makeNative(fnText);
    (*obj.mapVal)["box"] = Value::makeNative(fnBox);
    (*obj.mapVal)["render"] = Value::makeNative(fnRender);
    (*obj.mapVal)["to_string"] = Value::makeNative(fnToString);

    env->set("hmcanvas", obj);
}

} // namespace Hammer
