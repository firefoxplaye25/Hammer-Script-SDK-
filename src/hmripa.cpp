#include "../include/hmripa.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace Hammer {

static const char HMRIPA_MAGIC[7] = {'H', 'M', 'R', 'I', 'P', 'A', '1'};

uint32_t HMRipa::computeCheckHash(const std::string& key, const std::vector<uint8_t>& data) {
    uint32_t hash = 2166136261u;
    for (char c : key) {
        hash ^= static_cast<uint8_t>(c);
        hash *= 16777619u;
    }
    for (size_t i = 0; i < std::min<size_t>(data.size(), 64); ++i) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    return hash;
}

std::vector<uint8_t> HMRipa::generateKeyStream(const std::string& key, size_t length) {
    std::vector<uint8_t> s(256);
    for (int i = 0; i < 256; ++i) s[i] = static_cast<uint8_t>(i);

    std::string safeKey = key.empty() ? "hmripa_default_key" : key;
    int j = 0;
    for (int i = 0; i < 256; ++i) {
        j = (j + s[i] + static_cast<uint8_t>(safeKey[i % safeKey.size()])) % 256;
        std::swap(s[i], s[j]);
    }

    std::vector<uint8_t> stream(length);
    int i = 0;
    j = 0;
    for (size_t k = 0; k < length; ++k) {
        i = (i + 1) % 256;
        j = (j + s[i]) % 256;
        std::swap(s[i], s[j]);
        stream[k] = s[(s[i] + s[j]) % 256];
    }
    return stream;
}

std::string HMRipa::encryptString(const std::string& plainText, const std::string& key) {
    if (plainText.empty()) return "";
    std::vector<uint8_t> data(plainText.begin(), plainText.end());
    auto stream = generateKeyStream(key, data.size());

    std::ostringstream oss;
    for (size_t i = 0; i < data.size(); ++i) {
        uint8_t enc = data[i] ^ stream[i];
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(enc);
    }
    return oss.str();
}

std::string HMRipa::decryptString(const std::string& cipherHex, const std::string& key, bool& ok) {
    ok = true;
    if (cipherHex.empty()) return "";
    if (cipherHex.size() % 2 != 0) {
        ok = false;
        return "";
    }

    std::vector<uint8_t> data;
    data.reserve(cipherHex.size() / 2);
    for (size_t i = 0; i < cipherHex.size(); i += 2) {
        std::string byteStr = cipherHex.substr(i, 2);
        char* endPtr = nullptr;
        unsigned long val = std::strtoul(byteStr.c_str(), &endPtr, 16);
        if (endPtr != byteStr.c_str() + 2) {
            ok = false;
            return "";
        }
        data.push_back(static_cast<uint8_t>(val));
    }

    auto stream = generateKeyStream(key, data.size());
    std::string result;
    result.resize(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        result[i] = static_cast<char>(data[i] ^ stream[i]);
    }
    return result;
}

bool HMRipa::encryptFile(const std::string& srcPath, const std::string& dstPath, const std::string& key) {
    std::ifstream in(srcPath, std::ios::binary);
    if (!in.is_open()) return false;

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    uint32_t checkHash = computeCheckHash(key, data);
    auto stream = generateKeyStream(key, data.size());

    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= stream[i];
    }

    std::ofstream out(dstPath, std::ios::binary);
    if (!out.is_open()) return false;

    // Write magic header: 7 bytes
    out.write(HMRIPA_MAGIC, sizeof(HMRIPA_MAGIC));
    // Write verification hash: 4 bytes
    out.write(reinterpret_cast<const char*>(&checkHash), sizeof(checkHash));
    // Write encrypted payload
    if (!data.empty()) {
        out.write(reinterpret_cast<const char*>(data.data()), data.size());
    }
    out.close();
    return true;
}

bool HMRipa::decryptFile(const std::string& srcPath, const std::string& dstPath, const std::string& key) {
    std::ifstream in(srcPath, std::ios::binary);
    if (!in.is_open()) return false;

    // Read magic header
    char magic[7];
    if (!in.read(magic, sizeof(magic)) || std::memcmp(magic, HMRIPA_MAGIC, 7) != 0) {
        in.close();
        return false; // Not a valid hmripa encrypted file
    }

    // Read verification hash
    uint32_t expectedHash = 0;
    if (!in.read(reinterpret_cast<char*>(&expectedHash), sizeof(expectedHash))) {
        in.close();
        return false;
    }

    // Read encrypted payload
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    auto stream = generateKeyStream(key, data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= stream[i];
    }

    // Check verification hash
    uint32_t actualHash = computeCheckHash(key, data);
    if (actualHash != expectedHash) {
        return false; // Wrong key / corrupted file
    }

    std::ofstream out(dstPath, std::ios::binary);
    if (!out.is_open()) return false;

    if (!data.empty()) {
        out.write(reinterpret_cast<const char*>(data.data()), data.size());
    }
    out.close();
    return true;
}

std::string HMRipa::hashString(const std::string& input) {
    uint64_t hash = 14695981039346656037ull;
    for (char c : input) {
        hash ^= static_cast<uint8_t>(c);
        hash *= 1099511628211ull;
    }
    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << hash;
    return oss.str();
}

void registerHMRipa(std::shared_ptr<Environment> env) {
    auto fnEncrypt = [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::makeString("");
        std::string text = args[0].toString();
        std::string key = args[1].toString();
        return Value::makeString(HMRipa::encryptString(text, key));
    };

    auto fnDecrypt = [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::makeString("");
        std::string cipher = args[0].toString();
        std::string key = args[1].toString();
        bool ok = false;
        std::string res = HMRipa::decryptString(cipher, key, ok);
        return Value::makeString(res);
    };

    auto fnEncryptFile = [](const std::vector<Value>& args) -> Value {
        if (args.size() < 3) return Value::makeBoolean(false);
        std::string src = args[0].toString();
        std::string dst = args[1].toString();
        std::string key = args[2].toString();
        return Value::makeBoolean(HMRipa::encryptFile(src, dst, key));
    };

    auto fnDecryptFile = [](const std::vector<Value>& args) -> Value {
        if (args.size() < 3) return Value::makeBoolean(false);
        std::string src = args[0].toString();
        std::string dst = args[1].toString();
        std::string key = args[2].toString();
        return Value::makeBoolean(HMRipa::decryptFile(src, dst, key));
    };

    auto fnHash = [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeString("");
        return Value::makeString(HMRipa::hashString(args[0].toString()));
    };

    // Register top level functions
    env->set("hmripa_encrypt", Value::makeNative(fnEncrypt));
    env->set("hmripa_decrypt", Value::makeNative(fnDecrypt));
    env->set("hmripa_encrypt_file", Value::makeNative(fnEncryptFile));
    env->set("hmripa_decrypt_file", Value::makeNative(fnDecryptFile));
    env->set("hmripa_hash", Value::makeNative(fnHash));

    // Map object namespace: hmripa
    Value hmripaObj = Value::makeMap();
    (*hmripaObj.mapVal)["encrypt"] = Value::makeNative(fnEncrypt);
    (*hmripaObj.mapVal)["decrypt"] = Value::makeNative(fnDecrypt);
    (*hmripaObj.mapVal)["encrypt_file"] = Value::makeNative(fnEncryptFile);
    (*hmripaObj.mapVal)["decrypt_file"] = Value::makeNative(fnDecryptFile);
    (*hmripaObj.mapVal)["hash"] = Value::makeNative(fnHash);
    env->set("hmripa", hmripaObj);
}

} // namespace Hammer
