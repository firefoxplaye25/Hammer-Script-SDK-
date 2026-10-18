#ifndef HAMMER_HMRIPA_HPP
#define HAMMER_HMRIPA_HPP

#include "value.hpp"
#include "environment.hpp"
#include <string>
#include <vector>
#include <cstdint>
#include <memory>

namespace Hammer {

class HMRipa {
public:
    // String encryption/decryption
    static std::string encryptString(const std::string& plainText, const std::string& key);
    static std::string decryptString(const std::string& cipherHex, const std::string& key, bool& ok);

    // File encryption/decryption
    static bool encryptFile(const std::string& srcPath, const std::string& dstPath, const std::string& key);
    static bool decryptFile(const std::string& srcPath, const std::string& dstPath, const std::string& key);

    // Hash/checksum helper
    static std::string hashString(const std::string& input);

    // Keystream generator helper
    static std::vector<uint8_t> generateKeyStream(const std::string& key, size_t length);
    static uint32_t computeCheckHash(const std::string& key, const std::vector<uint8_t>& data);
};

void registerHMRipa(std::shared_ptr<Environment> env);

} // namespace Hammer

#endif // HAMMER_HMRIPA_HPP
