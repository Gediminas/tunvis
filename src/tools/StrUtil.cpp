#include "StrUtil.h"

std::vector<std::string> explode(const std::string &sText, const std::string &sDelims) {
    std::vector<std::string> arsTokens;
    size_t beg, pos = 0;
    while ((beg = sText.find_first_not_of(sDelims, pos)) != std::string::npos) {
        pos = sText.find_first_of(sDelims, beg + 1);
        arsTokens.push_back(sText.substr(beg, pos - beg));
    }
    return arsTokens;
}

std::string& ltrim(std::string &s) {
    const auto it = std::find_if(s.begin(), s.end(), [](char c) {
            return !std::isspace<char>(c, std::locale::classic());
        });
    s.erase(s.begin(), it);
    return s;
}

std::string& rtrim(std::string &s) {
    const auto it = std::find_if(s.rbegin(), s.rend(), [](char c) {
            return !std::isspace<char>(c, std::locale::classic());
        });
    s.erase(it.base(), s.end());
    return s;
}

std::string& trim(std::string &s) {
    return ltrim(rtrim(s));
}

std::string str_format(const std::string fmt_str, ...) {
    va_list ap;
    char *fp = NULL;
    va_start(ap, fmt_str);
    vasprintf(&fp, fmt_str.c_str(), ap);
    va_end(ap);
    std::unique_ptr<char[]> formatted(fp);
    return std::string(formatted.get());
}
