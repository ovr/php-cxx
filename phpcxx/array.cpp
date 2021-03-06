#include <stdexcept>
#include "array.h"
#include "helpers.h"
#include "string.h"
#include "value.h"
#include "zendstring.h"

static inline zval* derefAndCheck(zval* z)
{
    if (Z_ISREF_P(z)) {
        z = Z_REFVAL_P(z);
    }

    if (UNEXPECTED(Z_TYPE_P(z) != IS_ARRAY)) {
        throw std::runtime_error("Not an array");
    }

    return z;
}

phpcxx::Array::Array()
{
    array_init(&this->m_z);
}

phpcxx::Array::Array(zval* z)
{
    if (Z_TYPE_P(z) == IS_ARRAY) {
        ZVAL_MAKE_REF(z);
        zend_reference* ref = Z_REF_P(z);
        ++GC_REFCOUNT(ref);
        ZVAL_REF(&this->m_z, ref);
    }
    else {
        ZVAL_COPY(&this->m_z, z);
        convert_to_array(&this->m_z);
    }
}

phpcxx::Array::Array(Value& v)
    : Array(&v.m_z)
{
}

phpcxx::Array::Array(Array& other)
{
    ZVAL_COPY(&this->m_z, &other.m_z);
}

phpcxx::Array::Array(Array&& other)
{
    ZVAL_UNDEF(&this->m_z);
    std::swap(this->m_z, other.m_z);
}

phpcxx::Array::~Array()
{
    i_zval_ptr_dtor(&this->m_z ZEND_FILE_LINE_CC);
#ifdef PHPCXX_DEBUG
    ZVAL_UNDEF(&this->m_z);
#endif
}

phpcxx::Array& phpcxx::Array::operator=(const Array& other)
{
    phpcxx::assign(&this->m_z, &other.m_z);
    return *this;
}

phpcxx::Value& phpcxx::Array::operator[](std::nullptr_t)
{
    zval* z = derefAndCheck(&this->m_z);
    SEPARATE_ARRAY(z);

    zval* var_ptr = zend_hash_next_index_insert(Z_ARRVAL_P(z), &EG(uninitialized_zval));
    if (UNEXPECTED(!var_ptr)) {
        throw std::runtime_error("Cannot add element to the array as the next element is already occupied");
    }

    return *(new(var_ptr) Value(placement_construct));
}

phpcxx::Value& phpcxx::Array::operator[](zend_long idx)
{
    zval* z = derefAndCheck(&this->m_z);
    SEPARATE_ARRAY(z);

    zend_ulong h = static_cast<zend_ulong>(idx);
    zval* retval = zend_hash_index_find(Z_ARRVAL_P(z), h);
    if (!retval) {
        retval = zend_hash_index_add_new(Z_ARRVAL_P(z), h, &EG(uninitialized_zval));
    }

    return *(new(retval) Value(placement_construct));
}

phpcxx::Value& phpcxx::Array::operator[](const Value& key)
{
    zval* z = &key.m_z;

    while (true) {
        switch (key.type()) {
            case Type::String:    return this->operator[](Z_STR(key.m_z));
            case Type::Integer:   return this->operator[](Z_LVAL(this->m_z));
            case Type::Double:    return this->operator[](zend_dval_to_lval(Z_DVAL(this->m_z)));
            case Type::True:      return this->operator[](1l);
            case Type::False:     return this->operator[](0l);
            case Type::Resource:  return this->operator[](Z_RES_HANDLE_P(z));
            case Type::Undefined: return this->operator[](ZSTR_EMPTY_ALLOC());
            case Type::Null:      return this->operator[](ZSTR_EMPTY_ALLOC());
            case Type::Reference:
                z = Z_REFVAL_P(z);
                break;

            default:
                throw std::runtime_error("Illegal offset type");
        }
    }
}

phpcxx::Value& phpcxx::Array::operator[](zend_string* key)
{
    zend_ulong hval;
    if (ZEND_HANDLE_NUMERIC(key, hval)) {
        return this->operator[](static_cast<zend_long>(hval));
    }

    zval* z = derefAndCheck(&this->m_z);
    SEPARATE_ARRAY(z);

    zval* retval = zend_hash_find(Z_ARRVAL_P(z), key);
    if (retval) {
        if (UNEXPECTED(Z_TYPE_P(retval) == IS_INDIRECT)) {
            retval = Z_INDIRECT_P(retval);
            if (UNEXPECTED(Z_TYPE_P(retval) == IS_UNDEF)) {
                ZVAL_NULL(retval);
            }
        }
    }
    else {
        retval = zend_hash_add_new(Z_ARRVAL_P(z), key, &EG(uninitialized_zval));
    }

    return *(new(retval) Value(placement_construct));
}

std::size_t phpcxx::Array::size() const
{
    zval* z = derefAndCheck(&this->m_z);
    return zend_hash_num_elements(Z_ARRVAL_P(z));
}

bool phpcxx::Array::contains(zend_long idx) const
{
    zval* z = derefAndCheck(&this->m_z);
    return zend_hash_index_exists(Z_ARRVAL_P(z), static_cast<zend_ulong>(idx));
}

bool phpcxx::Array::contains(const Value& key) const
{
    zval* z = &key.m_z;

    while (true) {
        switch (key.type()) {
            case Type::String:    return this->contains(Z_STR(key.m_z));
            case Type::Integer:   return this->contains(Z_LVAL(this->m_z));
            case Type::Double:    return this->contains(zend_dval_to_lval(Z_DVAL(this->m_z)));
            case Type::True:      return this->contains(1l);
            case Type::False:     return this->contains(0l);
            case Type::Resource:  return this->contains(Z_RES_HANDLE_P(z));
            case Type::Undefined: return this->contains(ZSTR_EMPTY_ALLOC());
            case Type::Null:      return this->contains(ZSTR_EMPTY_ALLOC());
            case Type::Reference:
                z = Z_REFVAL_P(z);
                break;

            default:
                throw std::runtime_error("Illegal offset type");
        }
    }
}

bool phpcxx::Array::contains(zend_string* key) const
{
    zend_ulong hval;
    if (ZEND_HANDLE_NUMERIC(key, hval)) {
        return this->contains(static_cast<zend_long>(hval));
    }

    zval* z = derefAndCheck(&this->m_z);
    return zend_hash_exists(Z_ARRVAL_P(z), key);
}

void phpcxx::Array::unset(zend_long idx)
{
    zval* z = derefAndCheck(&this->m_z);
    SEPARATE_ARRAY(z);
    zend_hash_index_del(Z_ARRVAL_P(z), static_cast<zend_ulong>(idx));
}

void phpcxx::Array::unset(const Value& key)
{
    zval* z = &key.m_z;

    while (true) {
        switch (key.type()) {
            case Type::String:    return this->unset(Z_STR(key.m_z));
            case Type::Integer:   return this->unset(Z_LVAL(this->m_z));
            case Type::Double:    return this->unset(zend_dval_to_lval(Z_DVAL(this->m_z)));
            case Type::True:      return this->unset(1l);
            case Type::False:     return this->unset(0l);
            case Type::Resource:  return this->unset(Z_RES_HANDLE_P(z));
            case Type::Undefined: return this->unset(ZSTR_EMPTY_ALLOC());
            case Type::Null:      return this->unset(ZSTR_EMPTY_ALLOC());
            case Type::Reference:
                z = Z_REFVAL_P(z);
                break;

            default:
                throw std::runtime_error("Illegal offset type");
        }
    }
}

void phpcxx::Array::unset(zend_string* key)
{
    zend_ulong hval;
    if (ZEND_HANDLE_NUMERIC(key, hval)) {
        return this->unset(static_cast<zend_long>(hval));
    }

    zval* z = derefAndCheck(&this->m_z);
    SEPARATE_ARRAY(z);

    HashTable* ht = Z_ARRVAL_P(z);
    if (UNEXPECTED(ht == &EG(symbol_table))) {
        zend_delete_global_variable(key);
    }
    else {
        zend_hash_del(ht, key);
    }
}

phpcxx::Value& phpcxx::Array::operator[](const char* key)
{
    return this->operator[](ZendString(key).release());
}

phpcxx::Value& phpcxx::Array::operator[](const string& key)
{
    return this->operator[](ZendString(key).release());
}

phpcxx::Value& phpcxx::Array::operator[](const ZendString& key)
{
    return this->operator[](key.get());
}

bool phpcxx::Array::contains(const char* key) const
{
    return this->contains(ZendString(key).release());
}

bool phpcxx::Array::contains(const string& key) const
{
    return this->contains(ZendString(key).release());
}

bool phpcxx::Array::contains(const ZendString& key) const
{
    return this->contains(key.get());
}

void phpcxx::Array::unset(const char* key)
{
    this->unset(ZendString(key).release());
}

void phpcxx::Array::unset(const string& key)
{
    this->unset(ZendString(key).release());
}

void phpcxx::Array::unset(const ZendString& key)
{
    this->unset(key.get());
}
