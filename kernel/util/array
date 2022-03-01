#pragma once

#include <stdint.h>
#include "../util/new.h"

template <typename Type>
class Array {
public:
    Array() {
        _data = nullptr;
        _size = _capa = 0;
    }

    explicit Array(uint32_t capa) {
        _data = reinterpret_cast<Type *>(Memory::alloc(sizeof (Type) * capa));
        _size = 0;
        _capa = capa;
    }

    Array(const Array &array) {
        _data = reinterpret_cast<Type *>(Memory::alloc(sizeof (Type) * array._size));
        _size = _capa = array._size;
        for (uint32_t i = 0; i < _size; i++) {
            new (_data + i)Type(array._data[i]);
        }
    }

    Array(Array &&array) {
        _data = array._data;
        _size = array._size;
        _capa = array._capa;
        array._data = nullptr;
        array._size = array._capa = 0;
    }

    ~Array() {
        if (_data != nullptr) {
            for (uint32_t i = 0; i < _size; i++) {
                _data[i].~Type();
            }
            Memory::free(_data);
        }
    }

    Array &operator=(const Array &array) {
        if (this == &array) {
            return *this;
        }
        if (_capa < array._size) {
            ~Array();
            _data = reinterpret_cast<Type *>(Memory::alloc(sizeof (Type) * array._size));
            _size = _capa = array._size;
        } else {
            for (uint32_t i = 0; i < _size; i++) {
                _data[i].~Type();
            }
            _size = array._size;
        }
        for (uint32_t i = 0; i < _size; i++) {
            new (_data + i)Type(array._data[i]);
        }
        return *this;
    }

    Array &operator=(Array &&array) {
        if (this == &array) {
            return *this;
        }
        ~Array();
        _data = array._data;
        _size = array._size;
        _capa = array._capa;
        array._data = nullptr;
        array._size = array._capa = 0;
        return *this;
    }

    void resize(uint32_t size) {
        if (size > _size) {
            reserve(size);
            for (uint32_t i = _size; i < size; i++) {
                new (_data + i)Type();
            }
        } else if (size < _size) {
            for (uint32_t i = size; i < _size; i++) {
                _data[i].~Type();
            }
        }
        _size = size;
    }

    void reserve(uint32_t capa) {
        if (capa <= _capa) {
            return;
        }
        
        Type *data = reinterpret_cast<Type *>(Memory::alloc(sizeof (Type) * capa));
        for (uint32_t i = 0; i < _size; i++) {
            new (data + i)Type(static_cast<Type &&>(_data[i]));
        }
        if (_data) {
            Memory::free(_data);
        }
        _data = data;
        _capa = capa;
    }

    uint32_t size() const {
        return _size;
    }

    uint32_t capacity() const {
        return _capa;
    }

    Type &operator[](uint32_t index) {
        return _data[index];
    }

    const Type &operator[](uint32_t index) const {
        return _data[index];
    }

    void pushBack(const Type &data) {
        if (_size == _capa) {
            reserve(_capa ? _capa * 2 : 1);
        }
        new (_data + _size++)Type(data);
    }
    
    void pushBack(Type &&data) {
        if (_size == _capa) {
            reserve(_capa ? _capa * 2 : 1);
        }
        new (_data + _size++)Type(static_cast<Type &&>(data));
    }

    Type popBack() {
        if (_size > 0) {
            Type temp(static_cast<Type &&>(_data[--_size]));
            _data[_size].~Type();
            return temp;
        } else {
            return Type();
        }
    }

    int32_t indexOf(const Type &val) {
        for (uint32_t i = 0; i < _size; i++) {
            if (val == _data[i]) {
                return i;
            }
        }
        return -1;
    }

    void eraseAt(uint32_t pos) {
        if (pos >= _size) {
            return;
        }
        _data[pos].~Type();
        while (pos + 1 < _size) {
            new (_data + pos)Type(static_cast<Type &&>(_data[pos + 1]));
            _data[pos + 1].~Type();
            pos++;
        }
        _size--;
    }

    Type *data() {
        return _data;
    }

private:
    Type *_data;
    uint32_t _size, _capa;
};
