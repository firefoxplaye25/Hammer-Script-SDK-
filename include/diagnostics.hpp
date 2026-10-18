#ifndef HAMMER_DIAGNOSTICS_HPP
#define HAMMER_DIAGNOSTICS_HPP

#include <string>
#include <vector>
#include <iostream>

namespace Hammer {

class DiagnosticEngine {
public:
    DiagnosticEngine() = default;
    DiagnosticEngine(std::string filename, std::string source);

    void setSource(std::string filename, std::string source);

    void reportError(
        int line,
        int column,
        const std::string& errorType,
        const std::string& message,
        const std::string& explanation,
        const std::string& suggestion,
        const std::string& codeExample = ""
    ) const;

    static std::string findClosestMatch(const std::string& target, const std::vector<std::string>& candidates);
    static int levenshteinDistance(const std::string& s1, const std::string& s2);

private:
    std::string filename = "input.hmscp";
    std::string source;
    std::vector<std::string> lines;

    void splitLines();
};

} // namespace Hammer

#endif // HAMMER_DIAGNOSTICS_HPP
