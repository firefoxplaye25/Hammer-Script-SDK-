#include "../include/diagnostics.hpp"
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace Hammer {

DiagnosticEngine::DiagnosticEngine(std::string filename, std::string source)
    : filename(std::move(filename)), source(std::move(source)) {
    splitLines();
}

void DiagnosticEngine::setSource(std::string file, std::string src) {
    filename = std::move(file);
    source = std::move(src);
    splitLines();
}

void DiagnosticEngine::splitLines() {
    lines.clear();
    std::stringstream ss(source);
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
}

int DiagnosticEngine::levenshteinDistance(const std::string& s1, const std::string& s2) {
    const size_t m = s1.size();
    const size_t n = s2.size();
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1));

    for (size_t i = 0; i <= m; ++i) dp[i][0] = static_cast<int>(i);
    for (size_t j = 0; j <= n; ++j) dp[0][j] = static_cast<int>(j);

    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            if (std::tolower(s1[i - 1]) == std::tolower(s2[j - 1])) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = 1 + std::min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
            }
        }
    }
    return dp[m][n];
}

std::string DiagnosticEngine::findClosestMatch(const std::string& target, const std::vector<std::string>& candidates) {
    if (candidates.empty()) return "";
    int bestDist = 999;
    std::string bestMatch;

    for (const auto& cand : candidates) {
        int dist = levenshteinDistance(target, cand);
        if (dist < bestDist && dist <= 3) {
            bestDist = dist;
            bestMatch = cand;
        }
    }
    return bestMatch;
}

void DiagnosticEngine::reportError(
    int line,
    int column,
    const std::string& errorType,
    const std::string& message,
    const std::string& explanation,
    const std::string& suggestion,
    const std::string& codeExample
) const {
    // ANSI color codes
    const std::string RED = "\033[1;31m";
    const std::string YELLOW = "\033[1;33m";
    const std::string CYAN = "\033[1;36m";
    const std::string GREEN = "\033[1;32m";
    const std::string BOLD = "\033[1m";
    const std::string DIM = "\033[2m";
    const std::string RESET = "\033[0m";

    std::cerr << "\n" << RED << "================================================================================" << RESET << "\n";
    std::cerr << RED << "[Hammer Script " << errorType << "]" << RESET << " " << BOLD << message << RESET << "\n";
    std::cerr << DIM << "--> " << filename << ":" << line << ":" << column << RESET << "\n";
    std::cerr << DIM << "    |" << RESET << "\n";

    // Show source context (previous line, offending line, next line)
    int startLine = std::max(1, line - 1);
    int endLine = std::min(static_cast<int>(lines.size()), line + 1);

    for (int l = startLine; l <= endLine; ++l) {
        std::string lineContent = (l - 1 < (int)lines.size()) ? lines[l - 1] : "";
        if (l == line) {
            std::cerr << RED << " " << std::setw(2) << l << " | " << RESET << lineContent << "\n";
            std::cerr << DIM << "    | " << RESET;
            for (int c = 1; c < column; ++c) {
                std::cerr << " ";
            }
            std::cerr << RED << "^" << RESET;
            std::cerr << RED << "--- Here is where the error occurred" << RESET << "\n";
        } else {
            std::cerr << DIM << " " << std::setw(2) << l << " | " << lineContent << RESET << "\n";
        }
    }
    std::cerr << DIM << "    |" << RESET << "\n";

    // Detailed explanation
    if (!explanation.empty()) {
        std::cerr << CYAN << "[Why did this happen?]" << RESET << "\n";
        std::cerr << "  " << explanation << "\n\n";
    }

    // How to fix it
    if (!suggestion.empty()) {
        std::cerr << YELLOW << "[How to fix this]" << RESET << "\n";
        std::cerr << "  " << suggestion << "\n\n";
    }

    // Code Example
    if (!codeExample.empty()) {
        std::cerr << GREEN << "[Correct Example]" << RESET << "\n";
        std::stringstream ss(codeExample);
        std::string exLine;
        while (std::getline(ss, exLine)) {
            std::cerr << "  " << exLine << "\n";
        }
        std::cerr << "\n";
    }
    std::cerr << RED << "================================================================================" << RESET << "\n\n";
}

} // namespace Hammer
