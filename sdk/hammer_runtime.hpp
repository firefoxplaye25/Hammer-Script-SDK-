#ifndef HAMMER_RUNTIME_HPP
#define HAMMER_RUNTIME_HPP

#include <winsock2.h>
#include <windows.h>
#include <wininet.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cmath>
#include <chrono>
#include <thread>
#include <random>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <algorithm>
#include <filesystem>
#include <array>

namespace HammerRuntime {

enum class Type { NOTHING, NUMBER, STRING, BOOLEAN, LIST, MAP };

class Var;

class Var {
public:
    Type type = Type::NOTHING;
    double num = 0.0;
    std::string str;
    bool b = false;
    std::shared_ptr<std::vector<Var>> list;
    std::shared_ptr<std::map<std::string, Var>> map;

    Var() : type(Type::NOTHING) {}
    Var(double n) : type(Type::NUMBER), num(n) {}
    Var(int n) : type(Type::NUMBER), num(static_cast<double>(n)) {}
    Var(long long n) : type(Type::NUMBER), num(static_cast<double>(n)) {}
    Var(const char* s) : type(Type::STRING), str(s) {}
    Var(std::string s) : type(Type::STRING), str(std::move(s)) {}
    Var(bool val) : type(Type::BOOLEAN), b(val) {}
    Var(std::vector<Var> items) : type(Type::LIST), list(std::make_shared<std::vector<Var>>(std::move(items))) {}
    Var(std::map<std::string, Var> entries) : type(Type::MAP), map(std::make_shared<std::map<std::string, Var>>(std::move(entries))) {}

    bool isTruthy() const {
        switch (type) {
            case Type::NOTHING: return false;
            case Type::BOOLEAN: return b;
            case Type::NUMBER: return num != 0.0;
            case Type::STRING: return !str.empty();
            case Type::LIST: return list && !list->empty();
            case Type::MAP: return map && !map->empty();
            default: return true;
        }
    }

    std::string toString() const {
        switch (type) {
            case Type::NOTHING: return "nothing";
            case Type::BOOLEAN: return b ? "true" : "false";
            case Type::NUMBER: {
                if (std::floor(num) == num && !std::isinf(num) && !std::isnan(num)) {
                    return std::to_string(static_cast<long long>(num));
                }
                std::string s = std::to_string(num);
                s.erase(s.find_last_not_of('0') + 1, std::string::npos);
                if (s.back() == '.') s.pop_back();
                return s;
            }
            case Type::STRING: return str;
            case Type::LIST: {
                if (!list) return "[]";
                std::string res = "[";
                for (size_t i = 0; i < list->size(); ++i) {
                    if (i > 0) res += ", ";
                    res += (*list)[i].toString();
                }
                res += "]";
                return res;
            }
            case Type::MAP: {
                if (!map) return "{}";
                std::string res = "{";
                bool first = true;
                for (const auto& pair : *map) {
                    if (!first) res += ", ";
                    first = false;
                    res += "\"" + pair.first + "\": " + pair.second.toString();
                }
                res += "}";
                return res;
            }
        }
        return "";
    }

    Var operator+(const Var& o) const {
        if (type == Type::NUMBER && o.type == Type::NUMBER) return Var(num + o.num);
        if (type == Type::STRING || o.type == Type::STRING) return Var(toString() + o.toString());
        if (type == Type::LIST && o.type == Type::LIST) {
            std::vector<Var> merged;
            if (list) merged.insert(merged.end(), list->begin(), list->end());
            if (o.list) merged.insert(merged.end(), o.list->begin(), o.list->end());
            return Var(merged);
        }
        return Var(num + o.num);
    }

    Var operator-() const { return Var(-num); }
    Var operator-(const Var& o) const { return Var(num - o.num); }
    Var operator*(const Var& o) const {
        if (type == Type::STRING && o.type == Type::NUMBER) {
            std::string res = "";
            for (int i = 0; i < (int)o.num; ++i) res += str;
            return Var(res);
        }
        return Var(num * o.num);
    }
    Var operator/(const Var& o) const { return Var(o.num != 0 ? num / o.num : 0.0); }
    Var operator%(const Var& o) const { return Var(static_cast<double>((long long)num % (long long)o.num)); }
    Var power(const Var& o) const { return Var(std::pow(num, o.num)); }

    Var operator==(const Var& o) const {
        if (type != o.type) return Var(false);
        if (type == Type::NUMBER) return Var(num == o.num);
        if (type == Type::STRING) return Var(str == o.str);
        if (type == Type::BOOLEAN) return Var(b == o.b);
        return Var(false);
    }
    Var operator!=(const Var& o) const { return Var(!(*this == o).b); }
    Var operator<(const Var& o) const {
        if (type == Type::STRING && o.type == Type::STRING) return Var(str < o.str);
        return Var(num < o.num);
    }
    Var operator<=(const Var& o) const {
        if (type == Type::STRING && o.type == Type::STRING) return Var(str <= o.str);
        return Var(num <= o.num);
    }
    Var operator>(const Var& o) const {
        if (type == Type::STRING && o.type == Type::STRING) return Var(str > o.str);
        return Var(num > o.num);
    }
    Var operator>=(const Var& o) const {
        if (type == Type::STRING && o.type == Type::STRING) return Var(str >= o.str);
        return Var(num >= o.num);
    }
    Var operator!() const { return Var(!isTruthy()); }

    Var& operator[](const Var& idx) {
        if (type == Type::LIST && list) {
            int i = static_cast<int>(idx.num);
            if (i >= 0 && i < (int)list->size()) return (*list)[i];
        }
        if (type == Type::MAP && map) {
            return (*map)[idx.toString()];
        }
        static Var dummy;
        return dummy;
    }

    Var get(const Var& idx) const {
        if (type == Type::LIST && list) {
            int i = static_cast<int>(idx.num);
            if (i >= 0 && i < (int)list->size()) return (*list)[i];
        }
        if (type == Type::MAP && map) {
            auto it = map->find(idx.toString());
            if (it != map->end()) return it->second;
        }
        if (type == Type::STRING) {
            int i = static_cast<int>(idx.num);
            if (i >= 0 && i < (int)str.size()) return Var(std::string(1, str[i]));
        }
        return Var();
    }

    Var slice(const Var& startVar, const Var& endVar) const {
        if (type == Type::LIST && list) {
            int len = static_cast<int>(list->size());
            int start = static_cast<int>(startVar.num);
            int end = static_cast<int>(endVar.num);
            if (start < 0) start = std::max(0, len + start);
            if (end < 0) end = std::max(0, len + end);
            start = std::clamp(start, 0, len);
            end = std::clamp(end, 0, len);
            std::vector<Var> sub;
            for (int i = start; i <= end && i < len; ++i) {
                sub.push_back((*list)[i]);
            }
            return Var(sub);
        }
        if (type == Type::STRING) {
            int len = static_cast<int>(str.size());
            int start = static_cast<int>(startVar.num);
            int end = static_cast<int>(endVar.num);
            if (start < 0) start = std::max(0, len + start);
            if (end < 0) end = std::max(0, len + end);
            start = std::clamp(start, 0, len);
            end = std::clamp(end, 0, len);
            if (start <= end && start < len) {
                int count = std::min(end - start + 1, len - start);
                return Var(str.substr(start, count));
            }
            return Var("");
        }
        return Var();
    }
};

inline void show(const Var& v) {
    std::cout << v.toString() << "\n";
}

inline Var ask(const Var& prompt) {
    std::cout << prompt.toString();
    std::cout.flush();
    std::string s;
    if (std::getline(std::cin, s)) {
        if (!s.empty() && s.back() == '\r') s.pop_back();
        return Var(s);
    }
    return Var("");
}

inline Var length(const Var& v) {
    if (v.type == Type::STRING) return Var(static_cast<double>(v.str.size()));
    if (v.type == Type::LIST && v.list) return Var(static_cast<double>(v.list->size()));
    if (v.type == Type::MAP && v.map) return Var(static_cast<double>(v.map->size()));
    return Var(0);
}

inline Var push(Var& l, const Var& item) {
    if (l.type == Type::LIST && l.list) l.list->push_back(item);
    return l;
}

inline Var pop(Var& l) {
    if (l.type == Type::LIST && l.list && !l.list->empty()) {
        Var last = l.list->back();
        l.list->pop_back();
        return last;
    }
    return Var();
}

inline Var random(const Var& minV, const Var& maxV) {
    static std::mt19937 rng(1337);
    long long mi = (long long)minV.num;
    long long ma = (long long)maxV.num;
    if (mi > ma) std::swap(mi, ma);
    std::uniform_int_distribution<long long> dist(mi, ma);
    return Var(static_cast<double>(dist(rng)));
}

inline Var round(const Var& v) { return Var(std::round(v.num)); }
inline Var sqrt(const Var& v) { return Var(std::sqrt(v.num)); }
inline Var abs(const Var& v) { return Var(std::abs(v.num)); }
inline Var to_number(const Var& v) {
    try { return Var(std::stod(v.toString())); } catch(...) { return Var(0); }
}
inline Var to_string(const Var& v) { return Var(v.toString()); }

inline Var read_file(const Var& path) {
    std::ifstream f(path.str);
    if (!f.is_open()) throw std::runtime_error("Cannot open file '" + path.str + "' for reading.");
    std::stringstream buffer;
    buffer << f.rdbuf();
    return Var(buffer.str());
}

inline Var write_file(const Var& path, const Var& content) {
    std::ofstream f(path.str);
    if (!f.is_open()) throw std::runtime_error("Cannot open file '" + path.str + "' for writing.");
    f << content.str;
    return Var(true);
}

inline Var file_exists(const Var& path) {
    std::ifstream f(path.str);
    return Var(f.good());
}

inline Var sleep(const Var& ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(ms.num)));
    return Var();
}

inline Var time() {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return Var(std::chrono::duration<double>(now).count());
}

// hamfs functions
inline Var hamfs_read(const Var& path) { return read_file(path); }
inline Var hamfs_write(const Var& path, const Var& content) { return write_file(path, content); }
inline Var hamfs_append(const Var& path, const Var& content) {
    std::ofstream f(path.str, std::ios::app | std::ios::binary);
    if (!f.is_open()) return Var(false);
    f << content.str;
    return Var(true);
}
inline Var hamfs_delete(const Var& path) {
    std::error_code ec;
    return Var(std::filesystem::remove_all(path.str, ec) > 0);
}
inline Var hamfs_make_dir(const Var& path) {
    std::error_code ec;
    return Var(std::filesystem::create_directories(path.str, ec));
}
inline Var hamfs_list(const Var& path) {
    std::string p = path.str.empty() ? "." : path.str;
    std::vector<Var> items;
    std::error_code ec;
    if (!std::filesystem::exists(p, ec) || !std::filesystem::is_directory(p, ec)) return Var(items);
    for (const auto& entry : std::filesystem::directory_iterator(p, ec)) {
        items.push_back(Var(entry.path().filename().string()));
    }
    return Var(items);
}
inline Var hamfs_exists(const Var& path) { return file_exists(path); }
inline Var hamfs_is_folder(const Var& path) {
    std::error_code ec;
    return Var(std::filesystem::is_directory(path.str, ec));
}
inline Var hamfs_is_file(const Var& path) {
    std::error_code ec;
    return Var(std::filesystem::is_regular_file(path.str, ec));
}
inline Var hamfs_size(const Var& path) {
    std::error_code ec;
    auto sz = std::filesystem::file_size(path.str, ec);
    return Var(ec ? 0.0 : static_cast<double>(sz));
}
inline Var hamfs_copy(const Var& src, const Var& dst) {
    std::error_code ec;
    std::filesystem::copy(src.str, dst.str, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing, ec);
    return Var(!ec);
}
inline Var hamfs_move(const Var& src, const Var& dst) {
    std::error_code ec;
    std::filesystem::rename(src.str, dst.str, ec);
    return Var(!ec);
}
inline Var hamfs_cwd() {
    std::error_code ec;
    return Var(std::filesystem::current_path(ec).string());
}
inline Var hamfs_exec(const Var& cmd) {
    std::array<char, 256> buf;
    std::string out;
    FILE* p = _popen(cmd.str.c_str(), "r");
    if (!p) return Var(std::map<std::string, Var>{{"exit_code", Var(-1)}, {"output", Var("Failed to run")}});
    while (fgets(buf.data(), static_cast<int>(buf.size()), p) != nullptr) out += buf.data();
    int code = _pclose(p);
    return Var(std::map<std::string, Var>{{"exit_code", Var(code)}, {"output", Var(out)}});
}

// asock & hmhookwww functions
inline Var asock_get(const Var& urlVar) {
    std::string url = urlVar.str;
    HINTERNET hInternet = InternetOpenA("HammerScript-ASock/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) return Var(std::map<std::string, Var>{{"ok", Var(false)}, {"status", Var(0)}, {"body", Var("Failed")}, {"url", Var(url)}});

    DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
    if (url.rfind("https://", 0) == 0) flags |= INTERNET_FLAG_SECURE;

    HINTERNET hFile = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, flags, 0);
    if (!hFile) {
        InternetCloseHandle(hInternet);
        return Var(std::map<std::string, Var>{{"ok", Var(false)}, {"status", Var(404)}, {"body", Var("Unreachable")}, {"url", Var(url)}});
    }

    DWORD status = 200;
    DWORD sz = sizeof(status);
    DWORD idx = 0;
    HttpQueryInfoA(hFile, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &sz, &idx);

    std::string body;
    char buf[4096];
    DWORD readBytes = 0;
    while (InternetReadFile(hFile, buf, sizeof(buf) - 1, &readBytes) && readBytes > 0) {
        buf[readBytes] = '\0';
        body.append(buf, readBytes);
    }
    InternetCloseHandle(hFile);
    InternetCloseHandle(hInternet);

    return Var(std::map<std::string, Var>{
        {"ok", Var(status >= 200 && status < 400)},
        {"status", Var(static_cast<double>(status))},
        {"body", Var(body)},
        {"url", Var(url)}
    });
}
inline Var hmhookwww_get(const Var& u) { return asock_get(u); }

inline Var asock_post(const Var& urlVar, const Var& bodyVar = Var(""), const Var& headersVar = Var()) {
    std::string url = urlVar.str;
    std::string postData = bodyVar.str;
    std::string headersStr = "Content-Type: application/json\r\n";
    if (headersVar.type == Type::MAP && headersVar.map) {
        headersStr.clear();
        for (const auto& pair : *headersVar.map) {
            headersStr += pair.first + ": " + pair.second.toString() + "\r\n";
        }
    }

    URL_COMPONENTSA urlComp;
    memset(&urlComp, 0, sizeof(urlComp));
    urlComp.dwStructSize = sizeof(urlComp);
    char host[512] = {0};
    char path[2048] = {0};
    urlComp.lpszHostName = host;
    urlComp.dwHostNameLength = sizeof(host);
    urlComp.lpszUrlPath = path;
    urlComp.dwUrlPathLength = sizeof(path);

    if (!InternetCrackUrlA(url.c_str(), static_cast<DWORD>(url.length()), 0, &urlComp)) {
        return Var(std::map<std::string, Var>{{"ok", Var(false)}, {"status", Var(0)}, {"body", Var("Invalid URL format")}, {"url", Var(url)}});
    }

    HINTERNET hInternet = InternetOpenA("HammerScript-ASock/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) return Var(std::map<std::string, Var>{{"ok", Var(false)}, {"status", Var(0)}, {"body", Var("Failed InternetOpen")}, {"url", Var(url)}});

    INTERNET_PORT port = urlComp.nPort;
    if (port == 0) port = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;

    HINTERNET hConnect = InternetConnectA(hInternet, host, port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return Var(std::map<std::string, Var>{{"ok", Var(false)}, {"status", Var(0)}, {"body", Var("Failed InternetConnect")}, {"url", Var(url)}});
    }

    DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
    if (urlComp.nScheme == INTERNET_SCHEME_HTTPS) flags |= INTERNET_FLAG_SECURE;

    const char* acceptTypes[] = { "*/*", NULL };
    std::string reqPath = path[0] == '\0' ? "/" : path;
    HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", reqPath.c_str(), NULL, NULL, acceptTypes, flags, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return Var(std::map<std::string, Var>{{"ok", Var(false)}, {"status", Var(0)}, {"body", Var("Failed HttpOpenRequest")}, {"url", Var(url)}});
    }

    HttpSendRequestA(hRequest, headersStr.c_str(), static_cast<DWORD>(headersStr.length()), (LPVOID)postData.c_str(), static_cast<DWORD>(postData.length()));

    DWORD status = 0;
    DWORD sz = sizeof(status);
    DWORD idx = 0;
    HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &sz, &idx);

    std::string body;
    char buf[4096];
    DWORD readBytes = 0;
    while (InternetReadFile(hRequest, buf, sizeof(buf) - 1, &readBytes) && readBytes > 0) {
        buf[readBytes] = '\0';
        body.append(buf, readBytes);
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return Var(std::map<std::string, Var>{
        {"ok", Var(status >= 200 && status < 400)},
        {"status", Var(static_cast<double>(status))},
        {"body", Var(body)},
        {"url", Var(url)}
    });
}
inline Var hmhookwww_post(const Var& u, const Var& b = Var(""), const Var& h = Var()) { return asock_post(u, b, h); }
inline Var fetch(const Var& u, const Var& b = Var(""), const Var& h = Var()) {
    if (!b.str.empty()) return asock_post(u, b, h);
    return asock_get(u);
}

inline Var asock_download(const Var& urlVar, const Var& destVar) {
    std::string url = urlVar.str;
    std::string dest = destVar.str;
    HINTERNET hInternet = InternetOpenA("HammerScript-ASock/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) return Var(false);

    DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
    if (url.rfind("https://", 0) == 0) flags |= INTERNET_FLAG_SECURE;

    HINTERNET hFile = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, flags, 0);
    if (!hFile) { InternetCloseHandle(hInternet); return Var(false); }

    std::ofstream out(dest, std::ios::binary);
    if (!out.is_open()) { InternetCloseHandle(hFile); InternetCloseHandle(hInternet); return Var(false); }

    char buf[8192];
    DWORD readBytes = 0;
    while (InternetReadFile(hFile, buf, sizeof(buf), &readBytes) && readBytes > 0) {
        out.write(buf, readBytes);
    }
    out.close();
    InternetCloseHandle(hFile);
    InternetCloseHandle(hInternet);
    return Var(true);
}
inline Var hmhookwww_download(const Var& u, const Var& d) { return asock_download(u, d); }

// hmripa primitive file encryptor / decryptor
inline std::vector<uint8_t> hmripa_keystream(const std::string& key, size_t length) {
    std::vector<uint8_t> s(256);
    for (int i = 0; i < 256; ++i) s[i] = static_cast<uint8_t>(i);
    std::string safeKey = key.empty() ? "hmripa_default_key" : key;
    int j = 0;
    for (int i = 0; i < 256; ++i) {
        j = (j + s[i] + static_cast<uint8_t>(safeKey[i % safeKey.size()])) % 256;
        std::swap(s[i], s[j]);
    }
    std::vector<uint8_t> stream(length);
    int i = 0; j = 0;
    for (size_t k = 0; k < length; ++k) {
        i = (i + 1) % 256;
        j = (j + s[i]) % 256;
        std::swap(s[i], s[j]);
        stream[k] = s[(s[i] + s[j]) % 256];
    }
    return stream;
}

inline uint32_t hmripa_compute_hash(const std::string& key, const std::vector<uint8_t>& data) {
    uint32_t hash = 2166136261u;
    for (char c : key) { hash ^= static_cast<uint8_t>(c); hash *= 16777619u; }
    for (size_t i = 0; i < std::min<size_t>(data.size(), 64); ++i) { hash ^= data[i]; hash *= 16777619u; }
    return hash;
}

inline Var hmripa_encrypt(const Var& textVar, const Var& keyVar) {
    std::string plain = textVar.str;
    std::string key = keyVar.str;
    if (plain.empty()) return Var("");
    std::vector<uint8_t> data(plain.begin(), plain.end());
    auto stream = hmripa_keystream(key, data.size());
    std::ostringstream oss;
    for (size_t i = 0; i < data.size(); ++i) {
        uint8_t enc = data[i] ^ stream[i];
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(enc);
    }
    return Var(oss.str());
}

inline Var hmripa_decrypt(const Var& cipherVar, const Var& keyVar) {
    std::string cipherHex = cipherVar.str;
    std::string key = keyVar.str;
    if (cipherHex.empty() || cipherHex.size() % 2 != 0) return Var("");
    std::vector<uint8_t> data;
    data.reserve(cipherHex.size() / 2);
    for (size_t i = 0; i < cipherHex.size(); i += 2) {
        std::string byteStr = cipherHex.substr(i, 2);
        char* endPtr = nullptr;
        unsigned long val = std::strtoul(byteStr.c_str(), &endPtr, 16);
        if (endPtr != byteStr.c_str() + 2) return Var("");
        data.push_back(static_cast<uint8_t>(val));
    }
    auto stream = hmripa_keystream(key, data.size());
    std::string result;
    result.resize(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        result[i] = static_cast<char>(data[i] ^ stream[i]);
    }
    return Var(result);
}

inline Var hmripa_encrypt_file(const Var& srcVar, const Var& dstVar, const Var& keyVar) {
    std::ifstream in(srcVar.str, std::ios::binary);
    if (!in.is_open()) return Var(false);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    uint32_t checkHash = hmripa_compute_hash(keyVar.str, data);
    auto stream = hmripa_keystream(keyVar.str, data.size());
    for (size_t i = 0; i < data.size(); ++i) data[i] ^= stream[i];

    std::ofstream out(dstVar.str, std::ios::binary);
    if (!out.is_open()) return Var(false);
    const char magic[7] = {'H', 'M', 'R', 'I', 'P', 'A', '1'};
    out.write(magic, sizeof(magic));
    out.write(reinterpret_cast<const char*>(&checkHash), sizeof(checkHash));
    if (!data.empty()) out.write(reinterpret_cast<const char*>(data.data()), data.size());
    out.close();
    return Var(true);
}

inline Var hmripa_decrypt_file(const Var& srcVar, const Var& dstVar, const Var& keyVar) {
    std::ifstream in(srcVar.str, std::ios::binary);
    if (!in.is_open()) return Var(false);
    char magic[7];
    if (!in.read(magic, sizeof(magic)) || std::memcmp(magic, "HMRIPA1", 7) != 0) {
        in.close();
        return Var(false);
    }
    uint32_t expectedHash = 0;
    if (!in.read(reinterpret_cast<char*>(&expectedHash), sizeof(expectedHash))) {
        in.close();
        return Var(false);
    }
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    auto stream = hmripa_keystream(keyVar.str, data.size());
    for (size_t i = 0; i < data.size(); ++i) data[i] ^= stream[i];

    uint32_t actualHash = hmripa_compute_hash(keyVar.str, data);
    if (actualHash != expectedHash) return Var(false);

    std::ofstream out(dstVar.str, std::ios::binary);
    if (!out.is_open()) return Var(false);
    if (!data.empty()) out.write(reinterpret_cast<const char*>(data.data()), data.size());
    out.close();
    return Var(true);
}

inline Var hmripa_hash(const Var& inputVar) {
    std::string input = inputVar.str;
    uint64_t hash = 14695981039346656037ull;
    for (char c : input) {
        hash ^= static_cast<uint8_t>(c);
        hash *= 1099511628211ull;
    }
    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << hash;
    return Var(oss.str());
}

// split, join, replace, to_upper, to_lower, trim, contains, starts_with, ends_with
inline Var split(const Var& sVar, const Var& delimVar = Var(" ")) {
    std::string s = sVar.str;
    std::string delim = delimVar.str.empty() ? " " : delimVar.str;
    std::vector<Var> result;
    if (delim.empty()) {
        for (char c : s) result.push_back(Var(std::string(1, c)));
        return Var(result);
    }
    size_t start = 0;
    size_t end = s.find(delim);
    while (end != std::string::npos) {
        result.push_back(Var(s.substr(start, end - start)));
        start = end + delim.length();
        end = s.find(delim, start);
    }
    result.push_back(Var(s.substr(start)));
    return Var(result);
}

inline Var join(const Var& listVar, const Var& delimVar = Var("")) {
    if (listVar.type != Type::LIST || !listVar.list) return Var("");
    std::string res;
    for (size_t i = 0; i < listVar.list->size(); ++i) {
        if (i > 0) res += delimVar.str;
        res += (*listVar.list)[i].toString();
    }
    return Var(res);
}

inline Var replace(const Var& sVar, const Var& targetVar, const Var& replVar) {
    std::string s = sVar.str;
    std::string target = targetVar.str;
    std::string repl = replVar.str;
    if (target.empty()) return Var(s);
    size_t pos = 0;
    while ((pos = s.find(target, pos)) != std::string::npos) {
        s.replace(pos, target.length(), repl);
        pos += repl.length();
    }
    return Var(s);
}

inline Var to_upper(const Var& sVar) {
    std::string s = sVar.toString();
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
    return Var(s);
}

inline Var to_lower(const Var& sVar) {
    std::string s = sVar.toString();
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return Var(s);
}

inline Var trim(const Var& sVar) {
    std::string s = sVar.toString();
    size_t first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return Var("");
    size_t last = s.find_last_not_of(" \t\r\n");
    return Var(s.substr(first, (last - first + 1)));
}

inline Var contains(const Var& target, const Var& query) {
    if (target.type == Type::STRING) {
        return Var(target.str.find(query.str) != std::string::npos);
    }
    if (target.type == Type::LIST && target.list) {
        for (const auto& item : *target.list) {
            if (item.toString() == query.toString()) return Var(true);
        }
        return Var(false);
    }
    if (target.type == Type::MAP && target.map) {
        return Var(target.map->find(query.str) != target.map->end());
    }
    return Var(false);
}

inline Var starts_with(const Var& sVar, const Var& prefixVar) {
    return Var(sVar.str.rfind(prefixVar.str, 0) == 0);
}

inline Var ends_with(const Var& sVar, const Var& suffixVar) {
    std::string s = sVar.str;
    std::string suffix = suffixVar.str;
    if (suffix.size() > s.size()) return Var(false);
    return Var(std::equal(suffix.rbegin(), suffix.rend(), s.rbegin()));
}

inline Var insert(Var& listVar, const Var& indexVar, const Var& item) {
    if (listVar.type == Type::LIST && listVar.list) {
        int idx = static_cast<int>(indexVar.num);
        idx = std::clamp(idx, 0, static_cast<int>(listVar.list->size()));
        listVar.list->insert(listVar.list->begin() + idx, item);
    }
    return listVar;
}

inline Var remove_at(Var& listVar, const Var& indexVar) {
    if (listVar.type == Type::LIST && listVar.list) {
        int idx = static_cast<int>(indexVar.num);
        if (idx >= 0 && idx < static_cast<int>(listVar.list->size())) {
            Var rem = (*listVar.list)[idx];
            listVar.list->erase(listVar.list->begin() + idx);
            return rem;
        }
    }
    return Var();
}

inline Var sort(Var& listVar) {
    if (listVar.type == Type::LIST && listVar.list) {
        std::sort(listVar.list->begin(), listVar.list->end(), [](const Var& a, const Var& b) {
            if (a.type == Type::NUMBER && b.type == Type::NUMBER) return a.num < b.num;
            return a.toString() < b.toString();
        });
    }
    return listVar;
}

inline Var reverse(Var& listVar) {
    if (listVar.type == Type::LIST && listVar.list) {
        std::reverse(listVar.list->begin(), listVar.list->end());
    }
    return listVar;
}

inline Var keys(const Var& mapVar) {
    std::vector<Var> res;
    if (mapVar.type == Type::MAP && mapVar.map) {
        for (const auto& pair : *mapVar.map) res.push_back(Var(pair.first));
    }
    return Var(res);
}

inline Var values(const Var& mapVar) {
    std::vector<Var> res;
    if (mapVar.type == Type::MAP && mapVar.map) {
        for (const auto& pair : *mapVar.map) res.push_back(pair.second);
    }
    return Var(res);
}

inline Var has_key(const Var& mapVar, const Var& keyVar) {
    if (mapVar.type == Type::MAP && mapVar.map) {
        return Var(mapVar.map->find(keyVar.str) != mapVar.map->end());
    }
    return Var(false);
}

// JSON Parser & Serializer in HammerRuntime
class RuntimeJsonParser {
    std::string s;
    size_t pos = 0;
public:
    explicit RuntimeJsonParser(std::string str) : s(std::move(str)) {}
    void skipWs() { while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\r' || s[pos] == '\n')) pos++; }
    Var parse() {
        skipWs();
        if (pos >= s.size()) return Var();
        char c = s[pos];
        if (c == '{') return parseObj();
        if (c == '[') return parseArr();
        if (c == '"') return parseStr();
        if (c == 't' || c == 'f') return parseBool();
        if (c == 'n') return parseNull();
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parseNum();
        return Var();
    }
    Var parseObj() {
        pos++;
        std::map<std::string, Var> m;
        skipWs();
        if (pos < s.size() && s[pos] == '}') { pos++; return Var(m); }
        while (pos < s.size()) {
            skipWs();
            if (pos >= s.size() || s[pos] != '"') break;
            Var k = parseStr();
            skipWs();
            if (pos < s.size() && s[pos] == ':') pos++;
            skipWs();
            Var v = parse();
            m[k.str] = v;
            skipWs();
            if (pos < s.size() && s[pos] == ',') { pos++; continue; }
            if (pos < s.size() && s[pos] == '}') { pos++; break; }
            break;
        }
        return Var(m);
    }
    Var parseArr() {
        pos++;
        std::vector<Var> items;
        skipWs();
        if (pos < s.size() && s[pos] == ']') { pos++; return Var(items); }
        while (pos < s.size()) {
            skipWs();
            items.push_back(parse());
            skipWs();
            if (pos < s.size() && s[pos] == ',') { pos++; continue; }
            if (pos < s.size() && s[pos] == ']') { pos++; break; }
            break;
        }
        return Var(items);
    }
    Var parseStr() {
        pos++;
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
            } else res += s[pos++];
        }
        if (pos < s.size() && s[pos] == '"') pos++;
        return Var(res);
    }
    Var parseNum() {
        size_t start = pos;
        if (s[pos] == '-') pos++;
        while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) pos++;
        if (pos < s.size() && s[pos] == '.') {
            pos++;
            while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) pos++;
        }
        double n = 0.0;
        try { n = std::stod(s.substr(start, pos - start)); } catch (...) {}
        return Var(n);
    }
    Var parseBool() {
        if (s.substr(pos, 4) == "true") { pos += 4; return Var(true); }
        if (s.substr(pos, 5) == "false") { pos += 5; return Var(false); }
        return Var(false);
    }
    Var parseNull() {
        if (s.substr(pos, 4) == "null") pos += 4;
        return Var();
    }
};

inline std::string runtimeJsonEncode(const Var& v) {
    switch (v.type) {
        case Type::NOTHING: return "null";
        case Type::BOOLEAN: return v.b ? "true" : "false";
        case Type::NUMBER: {
            if (std::floor(v.num) == v.num && !std::isinf(v.num) && !std::isnan(v.num)) {
                return std::to_string(static_cast<long long>(v.num));
            }
            std::string s = std::to_string(v.num);
            s.erase(s.find_last_not_of('0') + 1, std::string::npos);
            if (s.back() == '.') s.pop_back();
            return s;
        }
        case Type::STRING: {
            std::string esc = "\"";
            for (char c : v.str) {
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
        case Type::LIST: {
            if (!v.list) return "[]";
            std::string res = "[";
            for (size_t i = 0; i < v.list->size(); ++i) {
                if (i > 0) res += ", ";
                res += runtimeJsonEncode((*v.list)[i]);
            }
            res += "]";
            return res;
        }
        case Type::MAP: {
            if (!v.map) return "{}";
            std::string res = "{";
            bool first = true;
            for (const auto& pair : *v.map) {
                if (!first) res += ", ";
                first = false;
                res += "\"" + pair.first + "\": " + runtimeJsonEncode(pair.second);
            }
            res += "}";
            return res;
        }
    }
    return "null";
}

inline Var json_parse(const Var& sVar) {
    RuntimeJsonParser p(sVar.str);
    return p.parse();
}

inline Var json_encode(const Var& v) {
    return Var(runtimeJsonEncode(v));
}

// hamsys functions
inline Var hamsys_now() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    auto now = std::chrono::system_clock::now();
    auto epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    std::map<std::string, Var> m;
    m["year"] = Var(static_cast<double>(st.wYear));
    m["month"] = Var(static_cast<double>(st.wMonth));
    m["day"] = Var(static_cast<double>(st.wDay));
    m["hour"] = Var(static_cast<double>(st.wHour));
    m["minute"] = Var(static_cast<double>(st.wMinute));
    m["second"] = Var(static_cast<double>(st.wSecond));
    m["millisecond"] = Var(static_cast<double>(st.wMilliseconds));
    m["weekday"] = Var(static_cast<double>(st.wDayOfWeek));
    m["timestamp"] = Var(static_cast<double>(epoch));
    return Var(m);
}

inline Var hamsys_date() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
    return Var(buf);
}

inline Var hamsys_time_str() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
    return Var(buf);
}

inline Var hamsys_datetime() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                  st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    return Var(buf);
}

inline Var hamsys_uptime() {
    return Var(static_cast<double>(GetTickCount64()) / 1000.0);
}

inline Var hamsys_beep(const Var& freqVar = Var(750), const Var& durVar = Var(200)) {
    DWORD freq = static_cast<DWORD>(freqVar.num > 0 ? freqVar.num : 750);
    DWORD dur = static_cast<DWORD>(durVar.num > 0 ? durVar.num : 200);
    return Var(Beep(freq, dur) ? true : false);
}

inline Var hamsys_play_sound(const Var& pathVar, const Var& asyncVar = Var(true)) {
    DWORD flags = SND_FILENAME | (asyncVar.isTruthy() ? SND_ASYNC : SND_SYNC);
    return Var(PlaySoundA(pathVar.str.c_str(), NULL, flags) ? true : false);
}

inline Var hamsys_system_sound(const Var& typeVar = Var("ok")) {
    UINT uType = MB_OK;
    std::string t = typeVar.str;
    if (t == "error" || t == "hand") uType = MB_ICONHAND;
    else if (t == "warning" || t == "exclamation") uType = MB_ICONEXCLAMATION;
    else if (t == "info" || t == "asterisk") uType = MB_ICONASTERISK;
    else if (t == "question") uType = MB_ICONQUESTION;
    return Var(MessageBeep(uType) ? true : false);
}

inline Var hamsys_os() {
    #if defined(_WIN64)
        return Var("Windows (64-bit)");
    #elif defined(_WIN32)
        return Var("Windows (32-bit)");
    #else
        return Var("Windows");
    #endif
}

inline Var hamsys_cpu_count() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return Var(static_cast<double>(si.dwNumberOfProcessors));
}

inline Var hamsys_memory() {
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(ms);
    std::map<std::string, Var> m;
    if (GlobalMemoryStatusEx(&ms)) {
        m["total_mb"] = Var(static_cast<double>(ms.ullTotalPhys / (1024 * 1024)));
        m["free_mb"] = Var(static_cast<double>(ms.ullAvailPhys / (1024 * 1024)));
        m["used_percent"] = Var(static_cast<double>(ms.dwMemoryLoad));
    }
    return Var(m);
}

inline Var hamsys_user() {
    char buf[256];
    DWORD size = sizeof(buf);
    if (GetUserNameA(buf, &size)) return Var(buf);
    return Var("");
}

inline Var hamsys_hostname() {
    char buf[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(buf);
    if (GetComputerNameA(buf, &size)) return Var(buf);
    return Var("");
}

inline Var hamsys_env(const Var& nameVar) {
    char buf[4096];
    DWORD ret = GetEnvironmentVariableA(nameVar.str.c_str(), buf, sizeof(buf));
    if (ret > 0 && ret < sizeof(buf)) return Var(buf);
    return Var("");
}

inline Var hamsys_clipboard_set(const Var& textVar) {
    if (!OpenClipboard(NULL)) return Var(false);
    EmptyClipboard();
    HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, textVar.str.size() + 1);
    if (!hGlob) {
        CloseClipboard();
        return Var(false);
    }
    memcpy(GlobalLock(hGlob), textVar.str.c_str(), textVar.str.size() + 1);
    GlobalUnlock(hGlob);
    SetClipboardData(CF_TEXT, hGlob);
    CloseClipboard();
    return Var(true);
}

inline Var hamsys_clipboard_get() {
    if (!OpenClipboard(NULL)) return Var("");
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (!hData) {
        CloseClipboard();
        return Var("");
    }
    char* pszText = static_cast<char*>(GlobalLock(hData));
    std::string text = pszText ? pszText : "";
    GlobalUnlock(hData);
    CloseClipboard();
    return Var(text);
}

inline Var hamsys_alert(const Var& titleVar, const Var& msgVar = Var("")) {
    std::string title = titleVar.str.empty() ? "Hammer Script Alert" : titleVar.str;
    std::string msg = msgVar.str.empty() ? titleVar.str : msgVar.str;
    if (msgVar.str.empty()) title = "Hammer Script Alert";
    MessageBoxA(NULL, msg.c_str(), title.c_str(), MB_OK | MB_ICONINFORMATION);
    return Var(true);
}

} // namespace HammerRuntime

#endif // HAMMER_RUNTIME_HPP
