#pragma once

#include <vector>
#include <algorithm>

std::vector<std::string> explode(const std::string &sText, const std::string &sDelims) {
    std::vector<std::string> arsTokens;
    size_t beg, pos = 0;
    while ((beg = sText.find_first_not_of(sDelims, pos)) != std::string::npos) {
        pos = sText.find_first_of(sDelims, beg + 1);
        arsTokens.push_back(sText.substr(beg, pos - beg));
    }
    return arsTokens;
}

std::string& ltrim(std::string &s)
{
    auto it = std::find_if(s.begin(), s.end(),
                    [](char c) {
                        return !std::isspace<char>(c, std::locale::classic());
                    });
    s.erase(s.begin(), it);
    return s;
}

std::string& rtrim(std::string &s)
{
    auto it = std::find_if(s.rbegin(), s.rend(),
                    [](char c) {
                        return !std::isspace<char>(c, std::locale::classic());
                    });
    s.erase(it.base(), s.end());
    return s;
}

std::string& trim(std::string &s) {
    return ltrim(rtrim(s));
}
