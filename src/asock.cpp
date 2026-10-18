#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <wininet.h>

#include "../include/asock.hpp"
#include "../include/errors.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>

namespace Hammer {

static bool wsaInitialized = false;

static void ensureWSA() {
    if (!wsaInitialized) {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        wsaInitialized = true;
    }
}

static std::unordered_map<int, SOCKET> activeSockets;
static int nextSocketId = 1;

void registerASock(std::shared_ptr<Environment> env) {
    // asock_get(url) / hmhookwww_get(url)
    auto fnGet = [](const std::vector<Value>& args) -> Value {
        if (args.empty()) {
            throw HammerError(0, 0, "ASock Error", "asock_get requires a URL string.",
                "You called asock_get without specifying the web address.",
                "Pass a valid URL string.", "res = asock_get(\"https://api.github.com\")");
        }
        std::string url = args[0].toString();

        HINTERNET hInternet = InternetOpenA("HammerScript-ASock/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (!hInternet) {
            Value res = Value::makeMap();
            (*res.mapVal)["ok"] = Value::makeBoolean(false);
            (*res.mapVal)["status"] = Value::makeNumber(0);
            (*res.mapVal)["body"] = Value::makeString("Failed to initialize Internet handle");
            (*res.mapVal)["url"] = Value::makeString(url);
            return res;
        }

        DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
        if (url.rfind("https://", 0) == 0) {
            flags |= INTERNET_FLAG_SECURE;
        }

        HINTERNET hFile = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, flags, 0);
        if (!hFile) {
            InternetCloseHandle(hInternet);
            Value res = Value::makeMap();
            (*res.mapVal)["ok"] = Value::makeBoolean(false);
            (*res.mapVal)["status"] = Value::makeNumber(404);
            (*res.mapVal)["body"] = Value::makeString("Failed to open URL or host unreachable");
            (*res.mapVal)["url"] = Value::makeString(url);
            return res;
        }

        DWORD statusCode = 200;
        DWORD statusSize = sizeof(statusCode);
        DWORD headerIndex = 0;
        HttpQueryInfoA(hFile, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &statusSize, &headerIndex);

        std::string body;
        char buffer[4096];
        DWORD bytesRead = 0;
        while (InternetReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            body.append(buffer, bytesRead);
        }

        InternetCloseHandle(hFile);
        InternetCloseHandle(hInternet);

        Value res = Value::makeMap();
        (*res.mapVal)["ok"] = Value::makeBoolean(statusCode >= 200 && statusCode < 400);
        (*res.mapVal)["status"] = Value::makeNumber(static_cast<double>(statusCode));
        (*res.mapVal)["body"] = Value::makeString(body);
        (*res.mapVal)["url"] = Value::makeString(url);
        return res;
    };

    // asock_post(url, body, headers) / hmhookwww_post(url, body, headers)
    auto fnPost = [](const std::vector<Value>& args) -> Value {
        if (args.empty()) {
            throw HammerError(0, 0, "ASock Error", "asock_post requires at least a URL string.",
                "You called asock_post without specifying the URL.",
                "Call asock_post(url, body, headers).", "asock_post(\"https://httpbin.org/post\", \"data\")");
        }
        std::string url = args[0].toString();
        std::string postData = args.size() > 1 ? args[1].toString() : "";

        std::string headersStr = "Content-Type: application/json\r\n";
        if (args.size() > 2 && args[2].type == ValueType::VAL_MAP && args[2].mapVal) {
            headersStr.clear();
            for (const auto& pair : *args[2].mapVal) {
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
            Value res = Value::makeMap();
            (*res.mapVal)["ok"] = Value::makeBoolean(false);
            (*res.mapVal)["status"] = Value::makeNumber(0);
            (*res.mapVal)["body"] = Value::makeString("Invalid URL format");
            (*res.mapVal)["url"] = Value::makeString(url);
            return res;
        }

        HINTERNET hInternet = InternetOpenA("HammerScript-ASock/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (!hInternet) {
            Value res = Value::makeMap();
            (*res.mapVal)["ok"] = Value::makeBoolean(false);
            (*res.mapVal)["status"] = Value::makeNumber(0);
            (*res.mapVal)["body"] = Value::makeString("Failed to initialize Internet handle");
            (*res.mapVal)["url"] = Value::makeString(url);
            return res;
        }

        INTERNET_PORT port = urlComp.nPort;
        if (port == 0) {
            port = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
        }

        HINTERNET hConnect = InternetConnectA(hInternet, host, port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
        if (!hConnect) {
            InternetCloseHandle(hInternet);
            Value res = Value::makeMap();
            (*res.mapVal)["ok"] = Value::makeBoolean(false);
            (*res.mapVal)["status"] = Value::makeNumber(0);
            (*res.mapVal)["body"] = Value::makeString("Failed to connect to host");
            (*res.mapVal)["url"] = Value::makeString(url);
            return res;
        }

        DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
        if (urlComp.nScheme == INTERNET_SCHEME_HTTPS) flags |= INTERNET_FLAG_SECURE;

        const char* acceptTypes[] = { "*/*", NULL };
        std::string reqPath = path[0] == '\0' ? "/" : path;
        HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", reqPath.c_str(), NULL, NULL, acceptTypes, flags, 0);
        if (!hRequest) {
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            Value res = Value::makeMap();
            (*res.mapVal)["ok"] = Value::makeBoolean(false);
            (*res.mapVal)["status"] = Value::makeNumber(0);
            (*res.mapVal)["body"] = Value::makeString("Failed to open HTTP request");
            (*res.mapVal)["url"] = Value::makeString(url);
            return res;
        }

        HttpSendRequestA(
            hRequest,
            headersStr.c_str(),
            static_cast<DWORD>(headersStr.length()),
            (LPVOID)postData.c_str(),
            static_cast<DWORD>(postData.length())
        );

        DWORD statusCode = 0;
        DWORD statusSize = sizeof(statusCode);
        DWORD headerIndex = 0;
        HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &statusSize, &headerIndex);

        std::string body;
        char buffer[4096];
        DWORD bytesRead = 0;
        while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            body.append(buffer, bytesRead);
        }

        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);

        Value res = Value::makeMap();
        (*res.mapVal)["ok"] = Value::makeBoolean(statusCode >= 200 && statusCode < 400);
        (*res.mapVal)["status"] = Value::makeNumber(static_cast<double>(statusCode));
        (*res.mapVal)["body"] = Value::makeString(body);
        (*res.mapVal)["url"] = Value::makeString(url);
        return res;
    };

    // asock_download(url, dest_path)
    auto fnDownload = [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) {
            throw HammerError(0, 0, "ASock Error", "asock_download requires URL and destination path.",
                "Specify the remote URL and the local file path to save.",
                "Call asock_download(url, local_path).", "asock_download(\"https://site.com/file.png\", \"image.png\")");
        }
        std::string url = args[0].toString();
        std::string dest = args[1].toString();

        HINTERNET hInternet = InternetOpenA("HammerScript-ASock/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (!hInternet) return Value::makeBoolean(false);

        DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
        if (url.rfind("https://", 0) == 0) flags |= INTERNET_FLAG_SECURE;

        HINTERNET hFile = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, flags, 0);
        if (!hFile) {
            InternetCloseHandle(hInternet);
            return Value::makeBoolean(false);
        }

        std::ofstream out(dest, std::ios::binary);
        if (!out.is_open()) {
            InternetCloseHandle(hFile);
            InternetCloseHandle(hInternet);
            return Value::makeBoolean(false);
        }

        char buffer[8192];
        DWORD bytesRead = 0;
        while (InternetReadFile(hFile, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            out.write(buffer, bytesRead);
        }

        out.close();
        InternetCloseHandle(hFile);
        InternetCloseHandle(hInternet);
        return Value::makeBoolean(true);
    };

    // asock_connect(host, port)
    auto fnConnect = [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::makeNumber(-1);
        ensureWSA();

        std::string host = args[0].toString();
        int port = static_cast<int>(args[1].numberVal);

        SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s == INVALID_SOCKET) return Value::makeNumber(-1);

        struct hostent* he = gethostbyname(host.c_str());
        if (!he) {
            closesocket(s);
            return Value::makeNumber(-1);
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr = *((in_addr*)he->h_addr);

        if (connect(s, (sockaddr*)&addr, sizeof(addr)) != 0) {
            closesocket(s);
            return Value::makeNumber(-1);
        }

        int id = nextSocketId++;
        activeSockets[id] = s;
        return Value::makeNumber(id);
    };

    // asock_send(sock_id, data)
    auto fnSend = [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::makeNumber(-1);
        int id = static_cast<int>(args[0].numberVal);
        std::string data = args[1].toString();

        auto it = activeSockets.find(id);
        if (it == activeSockets.end()) return Value::makeNumber(-1);

        int sent = send(it->second, data.c_str(), static_cast<int>(data.size()), 0);
        return Value::makeNumber(sent);
    };

    // asock_recv(sock_id, max_size)
    auto fnRecv = [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeString("");
        int id = static_cast<int>(args[0].numberVal);
        int maxBytes = args.size() > 1 ? static_cast<int>(args[1].numberVal) : 4096;

        auto it = activeSockets.find(id);
        if (it == activeSockets.end()) return Value::makeString("");

        std::vector<char> buf(maxBytes + 1, 0);
        int recvd = recv(it->second, buf.data(), maxBytes, 0);
        if (recvd > 0) {
            return Value::makeString(std::string(buf.data(), recvd));
        }
        return Value::makeString("");
    };

    // asock_close(sock_id)
    auto fnClose = [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeBoolean(false);
        int id = static_cast<int>(args[0].numberVal);

        auto it = activeSockets.find(id);
        if (it == activeSockets.end()) return Value::makeBoolean(false);

        closesocket(it->second);
        activeSockets.erase(it);
        return Value::makeBoolean(true);
    };

    // Fetch shorthand: does POST if body provided, else GET
    auto fnFetch = [fnGet, fnPost](const std::vector<Value>& args) -> Value {
        if (args.size() > 1 && !args[1].toString().empty()) {
            return fnPost(args);
        }
        return fnGet(args);
    };

    // Top-level aliases for asock
    env->set("asock_get", Value::makeNative(fnGet));
    env->set("asock_post", Value::makeNative(fnPost));
    env->set("asock_download", Value::makeNative(fnDownload));
    env->set("asock_connect", Value::makeNative(fnConnect));
    env->set("asock_send", Value::makeNative(fnSend));
    env->set("asock_recv", Value::makeNative(fnRecv));
    env->set("asock_close", Value::makeNative(fnClose));

    // Top-level aliases for hmhookwww
    env->set("hmhookwww_get", Value::makeNative(fnGet));
    env->set("hmhookwww_post", Value::makeNative(fnPost));
    env->set("hmhookwww_download", Value::makeNative(fnDownload));
    env->set("hmhookwww_connect", Value::makeNative(fnConnect));
    env->set("hmhookwww_send", Value::makeNative(fnSend));
    env->set("hmhookwww_recv", Value::makeNative(fnRecv));
    env->set("hmhookwww_close", Value::makeNative(fnClose));

    // Simple fetch() shorthand
    env->set("fetch", Value::makeNative(fnFetch));

    // Map object: asock
    Value asockObj = Value::makeMap();
    (*asockObj.mapVal)["get"] = Value::makeNative(fnGet);
    (*asockObj.mapVal)["post"] = Value::makeNative(fnPost);
    (*asockObj.mapVal)["download"] = Value::makeNative(fnDownload);
    (*asockObj.mapVal)["connect"] = Value::makeNative(fnConnect);
    (*asockObj.mapVal)["send"] = Value::makeNative(fnSend);
    (*asockObj.mapVal)["recv"] = Value::makeNative(fnRecv);
    (*asockObj.mapVal)["close"] = Value::makeNative(fnClose);
    env->set("asock", asockObj);

    // Map object: hmhookwww (interchangeable twin!)
    Value wwwObj = Value::makeMap();
    (*wwwObj.mapVal)["get"] = Value::makeNative(fnGet);
    (*wwwObj.mapVal)["post"] = Value::makeNative(fnPost);
    (*wwwObj.mapVal)["download"] = Value::makeNative(fnDownload);
    (*wwwObj.mapVal)["connect"] = Value::makeNative(fnConnect);
    (*wwwObj.mapVal)["send"] = Value::makeNative(fnSend);
    (*wwwObj.mapVal)["recv"] = Value::makeNative(fnRecv);
    (*wwwObj.mapVal)["close"] = Value::makeNative(fnClose);
    env->set("hmhookwww", wwwObj);
}

} // namespace Hammer
