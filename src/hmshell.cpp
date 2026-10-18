#include "../include/hmshell.hpp"
#include <windows.h>
#include <conio.h>
#include <iostream>
#include <string>
#include <vector>
#include <array>

namespace Hammer {

void registerHmShell(std::shared_ptr<Environment> env) {
    // 1. hmshell_clear()
    auto fnClear = [](const std::vector<Value>& args) -> Value {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD coordScreen = {0, 0};
        DWORD cCharsWritten;
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        DWORD dwConSize;

        if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
            dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
            FillConsoleOutputCharacterA(hConsole, ' ', dwConSize, coordScreen, &cCharsWritten);
            FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);
            SetConsoleCursorPosition(hConsole, coordScreen);
        } else {
            std::system("cls");
        }
        return Value::makeBoolean(true);
    };

    // 2. hmshell_title(text)
    auto fnTitle = [](const std::vector<Value>& args) -> Value {
        if (!args.empty()) {
            SetConsoleTitleA(args[0].toString().c_str());
        }
        return Value::makeBoolean(true);
    };

    // 3. hmshell_cursor(x, y)
    auto fnCursor = [](const std::vector<Value>& args) -> Value {
        if (args.size() >= 2) {
            SHORT x = static_cast<SHORT>(args[0].numberVal);
            SHORT y = static_cast<SHORT>(args[1].numberVal);
            COORD pos = {x, y};
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
            return Value::makeBoolean(true);
        }
        return Value::makeBoolean(false);
    };

    // 4. hmshell_color(colorName)
    auto fnColor = [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeBoolean(false);
        std::string col = args[0].toString();
        HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
        WORD attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; // default white

        if (col == "red") attr = FOREGROUND_RED | FOREGROUND_INTENSITY;
        else if (col == "green") attr = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        else if (col == "blue") attr = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        else if (col == "yellow") attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        else if (col == "cyan") attr = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        else if (col == "magenta") attr = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        else if (col == "white") attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        else if (col == "gray" || col == "grey") attr = FOREGROUND_INTENSITY;
        else if (col == "reset") attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

        SetConsoleTextAttribute(hCon, attr);
        return Value::makeBoolean(true);
    };

    // 5. hmshell_pause(prompt)
    auto fnPause = [](const std::vector<Value>& args) -> Value {
        std::string prompt = (!args.empty()) ? args[0].toString() : "Press any key to continue...";
        std::cout << prompt;
        std::cout.flush();
        int ch = _getch();
        std::cout << "\n";
        return Value::makeNumber(static_cast<double>(ch));
    };

    // 6. hmshell_exec(cmd)
    auto fnExec = [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeString("");
        std::string cmd = args[0].toString();
        std::array<char, 256> buf;
        std::string result;
        FILE* pipe = _popen(cmd.c_str(), "r");
        if (!pipe) return Value::makeString("");
        while (fgets(buf.data(), static_cast<int>(buf.size()), pipe) != nullptr) {
            result += buf.data();
        }
        _pclose(pipe);
        return Value::makeString(result);
    };

    // Register top-level functions
    env->set("hmshell_clear", Value::makeNative(fnClear));
    env->set("hmshell_title", Value::makeNative(fnTitle));
    env->set("hmshell_cursor", Value::makeNative(fnCursor));
    env->set("hmshell_color", Value::makeNative(fnColor));
    env->set("hmshell_pause", Value::makeNative(fnPause));
    env->set("hmshell_exec", Value::makeNative(fnExec));

    // Register namespace object: hmshell
    Value hmshellObj = Value::makeMap();
    (*hmshellObj.mapVal)["clear"] = Value::makeNative(fnClear);
    (*hmshellObj.mapVal)["title"] = Value::makeNative(fnTitle);
    (*hmshellObj.mapVal)["cursor"] = Value::makeNative(fnCursor);
    (*hmshellObj.mapVal)["color"] = Value::makeNative(fnColor);
    (*hmshellObj.mapVal)["pause"] = Value::makeNative(fnPause);
    (*hmshellObj.mapVal)["exec"] = Value::makeNative(fnExec);

    env->set("hmshell", hmshellObj);
}

} // namespace Hammer
