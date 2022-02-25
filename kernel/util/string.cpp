#include "string.h"
#include "new.h"

void memcpy(void *dst, const void *src, uint32_t size) {
    uint8_t *pdst = reinterpret_cast<uint8_t *>(dst);
    const uint8_t *psrc = reinterpret_cast<const uint8_t *>(src);
    while (size--) {
        *pdst++ = *psrc++;
    }
}

int memcmp(const void *data1, const void *data2, uint32_t size) {
    const uint8_t *p1 = reinterpret_cast<const uint8_t *>(data1);
    const uint8_t *p2 = reinterpret_cast<const uint8_t *>(data2);
    while (size--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

void memset(void *data, uint8_t value, uint32_t size) {
    uint8_t *p = reinterpret_cast<uint8_t *>(data);
    while (size--) {
        *p++ = value;
    }
}

uint32_t strlen(const char *str) {
    uint32_t l = 0;
    while (*str++) {
        l++;
    }
    return l;
}

void strcpy(char *dst, const char *src) {
    while (*src) {
        *dst++ = *src++;
    }
    *dst = 0;
}

int32_t strcmp(const char *str1, const char *str2) {
    while (*str1 && *str2) {
        if (*str1 != *str2) {
            return *str1 - *str2;
        }
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

String::String(const char *str) {
    _length = strlen(str);
    _buffer = new char[_length + 1];
    strcpy(_buffer, str);
}

String::String(const String &str) {
    _length = str._length;
    _buffer = new char[_length + 1];
    strcpy(_buffer, str._buffer);
}

String::String(String &&str) {
    _length = str._length;
    _buffer = str._buffer;
    str._buffer = nullptr;
}

String::~String() {
    if (_buffer) {
        delete[] _buffer;
    }
}

String &String::operator=(const String &str) {
    if (this == &str) {
        return *this;
    }
    if (_buffer) {
        delete[] _buffer;
    }
    _length = str._length;
    _buffer = new char[_length + 1];
    strcpy(_buffer, str._buffer);
    return *this;
}

String &String::operator=(String &&str) {
    if (this == &str) {
        return *this;
    }
    if (_buffer) {
        delete[] _buffer;
    }
    _length = str._length;
    _buffer = str._buffer;
    str._length = 0;
    str._buffer = nullptr;
    return *this;
}

String String::substr(uint32_t begin, int32_t length) const {
    if (begin >= _length) {
        return "";
    }
    if (length < 0) {
        length = _length - begin;
    }
    char *buf = new char[length + 1];
    memcpy(buf, _buffer + begin, length);
    buf[length] = 0;
    return String(buf, length);
}