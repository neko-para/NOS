#pragma once

#include <stdint.h>

void memcpy(void *dst, const void *src, uint32_t size);
int memcmp(const void *data1, const void *data2, uint32_t size);
void memset(void *data, uint8_t value, uint32_t size);

uint32_t strlen(const char *str);
void strcpy(char *dst, const char *src);
int32_t strcmp(const char *str1, const char *str2);

class String {
public:
    String(const char *str = "");
    String(const String &str);
    String(String &&str);
    ~String();
    String &operator=(const String &str);
    String &operator=(String &&str);

    const char *c_str() const {
        return _buffer;
    }
    char &operator[](uint32_t index) {
        return _buffer[index];
    }
    const char &operator[](uint32_t index) const {
        return _buffer[index];
    }
    uint32_t length() const {
        return _length;
    }

    int32_t toInt() const {
        int32_t x = 0, f = 0;
        const char *p = _buffer;
        if (*p == '-') {
            p++;
            f = 1;
        }
        while (*p >= '0' && *p <= '9') {
            x = (x * 10) + (*p - '0');
        }
        return f ? -x : x;
    }

    int32_t indexOf(char ch, uint32_t from = 0) const {
        for (uint32_t i = from; i < _length; i++) {
            if (_buffer[i] == ch) {
                return i;
            }
        }
        return -1;
    }

    String substr(uint32_t begin, int32_t length = -1) const;

    friend bool operator==(const String &s1, const String &s2) {
        return s1._length == s2._length && !memcmp(s1._buffer, s2._buffer, s1._length);
    }

private:
    String(char *buf, uint32_t len) : _length(len), _buffer(buf) {}

    uint32_t _length;
    char *_buffer;
};
