#include <string.h>
#include "string"
#include "new.h"

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

bool operator==(const String &s1, const String &s2) {
    return s1._length == s2._length && !memcmp(s1._buffer, s2._buffer, s1._length);
}