#include "../include/hamsys.hpp"
#include "../include/errors.hpp"
#include <windows.h>
#include <mmsystem.h>
#include <chrono>
#include <cstdio>
#include <string>
#include <vector>

namespace Hammer {

void registerHamSys(std::shared_ptr<Environment> env) {
    // 1. hamsys_now() -> Map with date & time parts + timestamp
    auto fnNow = [](const std::vector<Value>& args) -> Value {
        SYSTEMTIME st;
        GetLocalTime(&st);

        auto now = std::chrono::system_clock::now();
        auto epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

        Value m = Value::makeMap();
        (*m.mapVal)["year"] = Value::makeNumber(static_cast<double>(st.wYear));
        (*m.mapVal)["month"] = Value::makeNumber(static_cast<double>(st.wMonth));
        (*m.mapVal)["day"] = Value::makeNumber(static_cast<double>(st.wDay));
        (*m.mapVal)["hour"] = Value::makeNumber(static_cast<double>(st.wHour));
        (*m.mapVal)["minute"] = Value::makeNumber(static_cast<double>(st.wMinute));
        (*m.mapVal)["second"] = Value::makeNumber(static_cast<double>(st.wSecond));
        (*m.mapVal)["millisecond"] = Value::makeNumber(static_cast<double>(st.wMilliseconds));
        (*m.mapVal)["weekday"] = Value::makeNumber(static_cast<double>(st.wDayOfWeek));
        (*m.mapVal)["timestamp"] = Value::makeNumber(static_cast<double>(epoch));
        return m;
    };

    // 2. hamsys_date() -> "YYYY-MM-DD"
    auto fnDate = [](const std::vector<Value>& args) -> Value {
        SYSTEMTIME st;
        GetLocalTime(&st);
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
        return Value::makeString(std::string(buf));
    };

    // 3. hamsys_time_str() -> "HH:MM:SS"
    auto fnTimeStr = [](const std::vector<Value>& args) -> Value {
        SYSTEMTIME st;
        GetLocalTime(&st);
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
        return Value::makeString(std::string(buf));
    };

    // 4. hamsys_datetime() -> "YYYY-MM-DD HH:MM:SS"
    auto fnDateTime = [](const std::vector<Value>& args) -> Value {
        SYSTEMTIME st;
        GetLocalTime(&st);
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                      st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        return Value::makeString(std::string(buf));
    };

    // 5. hamsys_uptime() -> seconds
    auto fnUptime = [](const std::vector<Value>& args) -> Value {
        ULONGLONG ms = GetTickCount64();
        return Value::makeNumber(static_cast<double>(ms) / 1000.0);
    };

    // 6. hamsys_beep(freq, duration_ms)
    auto fnBeep = [](const std::vector<Value>& args) -> Value {
        DWORD freq = 750;
        DWORD dur = 200;
        if (!args.empty()) freq = static_cast<DWORD>(args[0].numberVal);
        if (args.size() > 1) dur = static_cast<DWORD>(args[1].numberVal);
        BOOL ok = Beep(freq, dur);
        return Value::makeBoolean(ok ? true : false);
    };

    // 7. hamsys_play_sound(path, async)
    auto fnPlaySound = [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeBoolean(false);
        std::string soundPath = args[0].toString();
        bool async = (args.size() > 1) ? args[1].isTruthy() : true;
        DWORD flags = SND_FILENAME | (async ? SND_ASYNC : SND_SYNC);
        BOOL ok = PlaySoundA(soundPath.c_str(), NULL, flags);
        return Value::makeBoolean(ok ? true : false);
    };

    // 8. hamsys_system_sound(type)
    auto fnSystemSound = [](const std::vector<Value>& args) -> Value {
        UINT uType = MB_OK;
        if (!args.empty()) {
            std::string t = args[0].toString();
            if (t == "error" || t == "hand") uType = MB_ICONHAND;
            else if (t == "warning" || t == "exclamation") uType = MB_ICONEXCLAMATION;
            else if (t == "info" || t == "asterisk") uType = MB_ICONASTERISK;
            else if (t == "question") uType = MB_ICONQUESTION;
            else uType = MB_OK;
        }
        BOOL ok = MessageBeep(uType);
        return Value::makeBoolean(ok ? true : false);
    };

    // 9. hamsys_os()
    auto fnOS = [](const std::vector<Value>& args) -> Value {
        #if defined(_WIN64)
            return Value::makeString("Windows (64-bit)");
        #elif defined(_WIN32)
            return Value::makeString("Windows (32-bit)");
        #else
            return Value::makeString("Windows");
        #endif
    };

    // 10. hamsys_cpu_count()
    auto fnCpuCount = [](const std::vector<Value>& args) -> Value {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        return Value::makeNumber(static_cast<double>(si.dwNumberOfProcessors));
    };

    // 11. hamsys_memory() -> {"total_mb": X, "free_mb": Y, "used_percent": Z}
    auto fnMemory = [](const std::vector<Value>& args) -> Value {
        MEMORYSTATUSEX ms;
        ms.dwLength = sizeof(ms);
        if (!GlobalMemoryStatusEx(&ms)) {
            return Value::makeMap();
        }
        Value m = Value::makeMap();
        (*m.mapVal)["total_mb"] = Value::makeNumber(static_cast<double>(ms.ullTotalPhys / (1024 * 1024)));
        (*m.mapVal)["free_mb"] = Value::makeNumber(static_cast<double>(ms.ullAvailPhys / (1024 * 1024)));
        (*m.mapVal)["used_percent"] = Value::makeNumber(static_cast<double>(ms.dwMemoryLoad));
        return m;
    };

    // 12. hamsys_user()
    auto fnUser = [](const std::vector<Value>& args) -> Value {
        char buf[256];
        DWORD size = sizeof(buf);
        if (GetUserNameA(buf, &size)) {
            return Value::makeString(std::string(buf));
        }
        return Value::makeString("");
    };

    // 13. hamsys_hostname()
    auto fnHostname = [](const std::vector<Value>& args) -> Value {
        char buf[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = sizeof(buf);
        if (GetComputerNameA(buf, &size)) {
            return Value::makeString(std::string(buf));
        }
        return Value::makeString("");
    };

    // 14. hamsys_env(name)
    auto fnEnv = [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeString("");
        std::string varName = args[0].toString();
        char buf[4096];
        DWORD ret = GetEnvironmentVariableA(varName.c_str(), buf, sizeof(buf));
        if (ret > 0 && ret < sizeof(buf)) {
            return Value::makeString(std::string(buf));
        }
        return Value::makeString("");
    };

    // 15. hamsys_clipboard_set(text)
    auto fnClipboardSet = [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeBoolean(false);
        std::string text = args[0].toString();
        if (!OpenClipboard(NULL)) return Value::makeBoolean(false);
        EmptyClipboard();
        HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
        if (!hGlob) {
            CloseClipboard();
            return Value::makeBoolean(false);
        }
        memcpy(GlobalLock(hGlob), text.c_str(), text.size() + 1);
        GlobalUnlock(hGlob);
        SetClipboardData(CF_TEXT, hGlob);
        CloseClipboard();
        return Value::makeBoolean(true);
    };

    // 16. hamsys_clipboard_get()
    auto fnClipboardGet = [](const std::vector<Value>& args) -> Value {
        if (!OpenClipboard(NULL)) return Value::makeString("");
        HANDLE hData = GetClipboardData(CF_TEXT);
        if (!hData) {
            CloseClipboard();
            return Value::makeString("");
        }
        char* pszText = static_cast<char*>(GlobalLock(hData));
        std::string text = pszText ? pszText : "";
        GlobalUnlock(hData);
        CloseClipboard();
        return Value::makeString(text);
    };

    // 17. hamsys_alert(title, message)
    auto fnAlert = [](const std::vector<Value>& args) -> Value {
        std::string title = (args.size() > 0) ? args[0].toString() : "Hammer Script";
        std::string message = (args.size() > 1) ? args[1].toString() : (args.size() > 0 ? args[0].toString() : "");
        if (args.size() == 1) {
            message = title;
            title = "Hammer Script Alert";
        }
        MessageBoxA(NULL, message.c_str(), title.c_str(), MB_OK | MB_ICONINFORMATION);
        return Value::makeBoolean(true);
    };

    // Register global functions
    env->set("hamsys_now", Value::makeNative(fnNow));
    env->set("hamsys_date", Value::makeNative(fnDate));
    env->set("hamsys_time_str", Value::makeNative(fnTimeStr));
    env->set("hamsys_datetime", Value::makeNative(fnDateTime));
    env->set("hamsys_uptime", Value::makeNative(fnUptime));
    env->set("hamsys_beep", Value::makeNative(fnBeep));
    env->set("hamsys_play_sound", Value::makeNative(fnPlaySound));
    env->set("hamsys_system_sound", Value::makeNative(fnSystemSound));
    env->set("hamsys_os", Value::makeNative(fnOS));
    env->set("hamsys_cpu_count", Value::makeNative(fnCpuCount));
    env->set("hamsys_memory", Value::makeNative(fnMemory));
    env->set("hamsys_user", Value::makeNative(fnUser));
    env->set("hamsys_hostname", Value::makeNative(fnHostname));
    env->set("hamsys_env", Value::makeNative(fnEnv));
    env->set("hamsys_clipboard_set", Value::makeNative(fnClipboardSet));
    env->set("hamsys_clipboard_get", Value::makeNative(fnClipboardGet));
    env->set("hamsys_alert", Value::makeNative(fnAlert));

    // Register 'hamsys' map object
    Value hamsysObj = Value::makeMap();
    (*hamsysObj.mapVal)["now"] = Value::makeNative(fnNow);
    (*hamsysObj.mapVal)["date"] = Value::makeNative(fnDate);
    (*hamsysObj.mapVal)["time_str"] = Value::makeNative(fnTimeStr);
    (*hamsysObj.mapVal)["datetime"] = Value::makeNative(fnDateTime);
    (*hamsysObj.mapVal)["uptime"] = Value::makeNative(fnUptime);
    (*hamsysObj.mapVal)["beep"] = Value::makeNative(fnBeep);
    (*hamsysObj.mapVal)["play_sound"] = Value::makeNative(fnPlaySound);
    (*hamsysObj.mapVal)["system_sound"] = Value::makeNative(fnSystemSound);
    (*hamsysObj.mapVal)["os"] = Value::makeNative(fnOS);
    (*hamsysObj.mapVal)["cpu_count"] = Value::makeNative(fnCpuCount);
    (*hamsysObj.mapVal)["memory"] = Value::makeNative(fnMemory);
    (*hamsysObj.mapVal)["user"] = Value::makeNative(fnUser);
    (*hamsysObj.mapVal)["hostname"] = Value::makeNative(fnHostname);
    (*hamsysObj.mapVal)["env"] = Value::makeNative(fnEnv);
    (*hamsysObj.mapVal)["clipboard_set"] = Value::makeNative(fnClipboardSet);
    (*hamsysObj.mapVal)["clipboard_get"] = Value::makeNative(fnClipboardGet);
    (*hamsysObj.mapVal)["alert"] = Value::makeNative(fnAlert);

    env->set("hamsys", hamsysObj);
}

} // namespace Hammer
