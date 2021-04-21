#pragma once

#include <vector>

std::vector<std::string> explode(const std::string &sText, const std::string &sDelims) {
    std::vector<std::string> arsTokens;
    size_t beg, pos = 0;
    while ((beg = sText.find_first_not_of(sDelims, pos)) != std::string::npos) {
        pos = sText.find_first_of(sDelims, beg + 1);
        arsTokens.push_back(sText.substr(beg, pos - beg));
    }
    return arsTokens;
}
