#include <unicode/unistr.h>
#include <unicode/ustring.h>
#include <unicode/ucnv.h>

#include "GenUtil.hpp"

// The following was copied from https://gist.github.com/kilfu0701/e279e35372066ae1832850c438d5611e
std::string LGenUtility::Utf8ToSjis(const std::string& value)
{
    icu::UnicodeString src(value.c_str(), "utf8");
    int length = src.extract(0, src.length(), NULL, "shift_jis");

    std::vector<char> result(length + 1);
    src.extract(0, src.length(), &result[0], "shift_jis");

    return std::string(result.begin(), result.end() - 1);
}

std::string LGenUtility::SjisToUtf8(const std::string& value)
{
    icu::UnicodeString src(value.c_str(), "shift_jis");
    int length = src.extract(0, src.length(), NULL, "utf8");

    std::vector<char> result(length + 1);
    src.extract(0, src.length(), &result[0], "utf8");

    return std::string(result.begin(), result.end() - 1);
}