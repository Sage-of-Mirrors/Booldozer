#include "GenUtil.hpp"
#include <iconv.h>

std::string LGenUtility::Utf8ToSjis(const std::string& value) {
    iconv_t conv = iconv_open("SHIFT-JIS", "UTF-8");
    if (conv == (iconv_t)(-1)) {
        throw std::runtime_error("Error opening iconv for UTF-8 to Shift-JIS conversion");
    }

    size_t inbytesleft = value.size();
    char* inbuf = const_cast<char*>(value.data());

    size_t outbytesleft = value.size() * 2;
    std::string sjis(outbytesleft, '\0');
    char* outbuf = &sjis[0];

    if (iconv(conv, &inbuf, &inbytesleft, &outbuf, &outbytesleft) == (size_t)(-1)) {
        throw std::runtime_error("Error converting from UTF-8 to Shift-JIS");
    }

    sjis.resize(sjis.size() - outbytesleft);
    iconv_close(conv);
    return sjis;
}

std::string LGenUtility::SjisToUtf8(const std::string& value) {
    iconv_t conv = iconv_open("UTF-8", "SHIFT-JIS");
    if (conv == (iconv_t)(-1)) {
        throw std::runtime_error("Error opening iconv for Shift-JIS to UTF-8 conversion");
    }

    size_t inbytesleft = value.size();
    char* inbuf = const_cast<char*>(value.data());

    size_t outbytesleft = value.size() * 3;
    std::string utf8(outbytesleft, '\0');
    char* outbuf = &utf8[0];

    if (iconv(conv, &inbuf, &inbytesleft, &outbuf, &outbytesleft) == (size_t)(-1)) {
        throw std::runtime_error("Error converting from Shift-JIS to UTF-8");
    }

    utf8.resize(utf8.size() - outbytesleft);
    iconv_close(conv);
    return utf8;
}