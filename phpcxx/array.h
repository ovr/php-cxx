#ifndef PHPCXX_ARRAY_H
#define PHPCXX_ARRAY_H

#include "phpcxx.h"

#include <type_traits>
#include <Zend/zend.h>
#include <Zend/zend_long.h>
#include <Zend/zend_string.h>
#include "helpers.h"
#include "string.h"
#include "map.h"
#include "vector.h"

namespace phpcxx {

class Value;
class ZendString;

class PHPCXX_EXPORT Array {
public:
    Array();
    [[gnu::nonnull]] Array(zval* z);
    Array(Value& v);
    Array(Array& other);
    Array(Array&& other);

    ~Array();

    Array& operator=(const Array& other);

    template<typename T>
    Array& operator=(const vector<T>& v);

    template<typename K, typename V, enable_if_t<std::is_integral<K>::value>* = nullptr>
    Array& operator=(const map<K, V>& v);

    template<typename K, typename V, enable_if_t<is_pchar<K>::value || is_string<K>::value>* = nullptr>
    Array& operator=(const map<K, V>& v);

    Value& operator[](std::nullptr_t);
    Value& operator[](zend_long idx);
    Value& operator[](const Value& key);
    [[gnu::nonnull]] Value& operator[](zend_string* key);

    std::size_t size() const;

    bool contains(zend_long idx) const;
    bool contains(const Value& key) const;
    [[gnu::nonnull]] bool contains(zend_string* key) const;

    void unset(zend_long idx);
    void unset(const Value& key);
    [[gnu::nonnull]] void unset(zend_string* key);

    Value& operator[](const char* key);
    Value& operator[](const string& key);
    Value& operator[](const ZendString& key);

    bool contains(const char* key) const;
    bool contains(const string& key) const;
    bool contains(const ZendString& key) const;

    void unset(const char* key);
    void unset(const string& key);
    void unset(const ZendString& key);

    zval* pzval() const { return &this->m_z; }
private:
    mutable zval m_z;

    Array(std::nullptr_t) {} // for placement new

    friend class Value;
};

}

#include "array.tcc"

#endif /* PHPCXX_ARRAY_H */
