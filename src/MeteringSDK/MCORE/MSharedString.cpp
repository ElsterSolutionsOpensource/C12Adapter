// File MCORE/MSharedString.cpp

#include "MCOREExtern.h"
#include "MSharedString.h"
#include "MCriticalSection.h"
#include "MTime.h"

#if !M_NO_VARIANT

namespace shared {

// Substantial part of this file is a derivation of freeware code.
// Used with permission in accordance of the following MIT license.
//
// Copyright (c) 2012 Intelligent Design Bureau
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

string::_buffer_type string::_empty_string_buffer = {0, 16, 0, 1}; // Has to be a POD
string string::_empty_string(&string::_empty_string_buffer);

inline sstl_size_type _adjust_capacity(sstl_size_type size)
{
    if (size <= string::_minimum_capacity)
        size = string::_minimum_capacity;
    else // adjust size to the nearest power of two trickery
    {
        --size;
        size |= size >> 1;
        size |= size >> 2;
        size |= size >> 4;
        size |= size >> 8;
        size |= size >> 16;
        ++size;
    }
    return size;
}

string& string::assign(const string& other)
{
    if (this != &other) // otherwise the string might get deleted
    {
        _get_buffer()->_ref_decrement();
        _set_uninitialized(other);
    }
    return *this;
}

string& string::assign(size_type size, char c)
{
    if (size == 0)
        clear();
    else if (is_shared() || capacity() < size)
    {
        _get_buffer()->_ref_decrement();
        _set_uninitialized(size, c);
    }
    else
    {
        _get_buffer()->_size = size;
        memset(_bytes, c, size);
    }
    return *this;
}

string& string::assign(const char* str)
{
    return assign(str, static_cast<size_type>(strlen(str)));
}

string& string::assign(const char* str, size_type size)
{
    if (size == 0)
        clear();
    else if (is_shared() || capacity() < size)
    {
        _get_buffer()->_ref_decrement();
        _set_uninitialized(str, size);
    }
    else
    {
        _get_buffer()->_size = size;
        memcpy(_bytes, str, size);
    }
    return *this;
}

string& string::assign(const string& str, size_type pos, size_type count)
{
    SSTL_ASSERT(str.size() >= pos);
    if (this == &str)
        *this = string(str, pos, count);
    else
    {
        const size_type delta = str.size() - pos;
        if (count == npos || delta < count)
            count = delta;
        assign(str.data() + pos, count);
    }
    return *this;
}

string& string::assign(const_iterator begin, const_iterator end)
{
    SSTL_ASSERT(end >= begin);
    const size_type size = static_cast<size_type>(end - begin);
    return assign(begin, size);
}

string& string::push_back(char c)
{
    char* place = _append_uninitialized(1);
    *place = c;
    return *this;
}

string& string::append(size_type size, char c)
{
    char* place = _append_uninitialized(size);
    memset(place, c, size);
    return *this;
}

string& string::append(const char* str)
{
    const size_type len = static_cast<size_type>(strlen(str));
    return append(str, len);
}

string& string::append(const char* str, size_type size)
{
    if (size != 0)
    {
        char* place = _append_uninitialized(size);
        memcpy(place, str, size);
    }
    return *this;
}

string& string::append(const string& other)
{
    return append(other.data(), other.size());
}

string& string::append(const_iterator begin, const_iterator end)
{
    SSTL_ASSERT(begin <= end);
    const size_type size = static_cast<size_type>(end - begin);
    return append(begin, size);
}

string::iterator string::erase(const_iterator position)
{
    size_type index = static_cast<size_type>(position - _bytes);
    SSTL_ASSERT(index < size());
    erase(index, 1);
    return _bytes + index;
}

string::iterator string::erase(const_iterator first, const_iterator last)
{
    SSTL_ASSERT(_bytes >= first);
    SSTL_ASSERT(last <= _bytes + size());
    SSTL_ASSERT(first < last);
    size_type pos = static_cast<size_type>(first - _bytes);
    size_type count = static_cast<size_type>(last - first);
    erase(pos, count);
    return _bytes + pos;
}

string::iterator string::insert(const_iterator where, char ch)
{
    size_type pos = static_cast<size_type>(where - _bytes);
    SSTL_ASSERT(pos <= size());
    char* buff = _insert_uninitialized(pos, 1);
    buff[0] = ch;
    return buff + 1;
}

string::iterator string::insert(const_iterator where, size_type count, char c)
{
    size_type pos = static_cast<size_type>(where - _bytes);
    insert(pos, count, c);
    return _bytes + pos + count;
}

string& string::insert(size_type pos, size_type count, char c)
{
    SSTL_ASSERT(pos <= size());
    char* buff = _insert_uninitialized(pos, count);
    memset(buff, c, count);
    return *this;
}

string& string::insert(size_type pos, const char* s)
{
    size_type size = static_cast<size_type>(strlen(s));
    return insert(pos, s, size);
}

string& string::insert(size_type pos, const char* s, size_type count)
{
    SSTL_ASSERT(pos <= size());
    char* buff = _insert_uninitialized(pos, count);
    memcpy(buff, s, count);
    return *this;
}

string& string::insert(size_type pos, const string& str)
{
    return insert(pos, str.data(), str.size());
}

string& string::insert(size_type pos, const string& str, size_type str_pos, size_type str_count)
{
    SSTL_ASSERT(str_pos + str_count <= str.size());
    return insert(pos, str.data() + str_pos, str_count);
}

string::iterator string::insert(const_iterator where, const_iterator input_first, const_iterator input_last)
{
    SSTL_ASSERT(input_first <= input_last);
    SSTL_ASSERT(where >= _bytes && where <= _bytes + size());
    size_type pos = static_cast<size_type>(where - _bytes);
    size_type count = static_cast<size_type>(input_last - input_first);
    insert(pos, input_first, count);
    return begin() + pos + count;
}

string& string::replace(size_type pos, size_type count, const string& str)
{
    char* buff = _replace_uninitialized(pos, count, str.size());
    memcpy(buff, str.data(), str.size());
    return *this;
}

string& string::replace(const_iterator first, const_iterator last, const string& str)
{
    SSTL_ASSERT(first >= _bytes);
    SSTL_ASSERT(last <= _bytes + size());
    size_type pos = static_cast<unsigned>(first - _bytes);
    size_type count = static_cast<unsigned>(last - first);
    return replace(pos, count, str);
}

string& string::replace(size_type pos, size_type count, const string& str, size_type strPos, size_type strCount)
{
    SSTL_ASSERT(strPos <= str.size());
    SSTL_ASSERT(strPos + strCount <= str.size());
    return replace(pos, count, str.data() + strPos, strCount);
}

string& string::replace(size_type pos, size_type count, const char* s, size_type sCount)
{
    char* buff = _replace_uninitialized(pos, count, sCount);
    memcpy(buff, s, sCount);
    return *this;
}

string& string::replace(const_iterator first, const_iterator last, const char* s, size_type strCount)
{
    SSTL_ASSERT(first >= _bytes);
    SSTL_ASSERT(last <= _bytes + size());
    SSTL_ASSERT(first <= last);
    size_type pos = static_cast<unsigned>(first - _bytes);
    size_type count = static_cast<unsigned>(last - first);
    return replace(pos, count, s, strCount);
}

string& string::replace(size_type pos, size_type count, const char* s)
{
    size_type sCount = static_cast<size_type>(strlen(s));
    return replace(pos, count, s, sCount);
}

string& string::replace(const_iterator first, const_iterator last, const char* s)
{
    SSTL_ASSERT(first >= _bytes);
    SSTL_ASSERT(last <= _bytes + size());
    SSTL_ASSERT(first <= last);
    size_type pos = static_cast<size_type>(first - _bytes);
    size_type count = static_cast<size_type>(last - first);
    return replace(pos, count, s);
}

string& string::replace(size_type pos, size_type count, size_type c_count, char c)
{
    char* buff = _replace_uninitialized(pos, count, c_count);
    char* buffEnd = buff + c_count;
    for ( ; buff != buffEnd; ++buff )
        *buff = c;
    return *this;
}

string& string::replace(const_iterator first, const_iterator last, size_type cCount, char c)
{
    SSTL_ASSERT(first >= _bytes);
    SSTL_ASSERT(last <= _bytes + size());
    SSTL_ASSERT(first <= last);
    size_type pos = static_cast<size_type>(first - _bytes);
    size_type count = static_cast<size_type>(last - first);
    return replace(pos, count, cCount, c);
}

string& string::replace(const_iterator first, const_iterator last, const_iterator input_first, const_iterator input_last)
{
    SSTL_ASSERT(first >= _bytes);
    SSTL_ASSERT(last <= _bytes + size());
    SSTL_ASSERT(input_first <= input_last);
    size_type pos = static_cast<size_type>(first - _bytes);
    size_type count = static_cast<size_type>(last - first);
    size_type str_count = static_cast<size_type>(input_last - input_first);
    return replace(pos, count, input_first, str_count);
}

void string::reserve(size_type reserved_size)
{
    if (reserved_size > capacity())
        _reallocate(reserved_size);
}

void string::shrink_to_fit()
{
    const size_type new_capacity = _adjust_capacity(size());
    if (new_capacity < capacity())
        _reallocate(new_capacity);
}

const char* string::c_str() const
{
    const size_type s = size();
    if (capacity() == s) // have to reallocate
        _reallocate(s + 1);
    _bytes[s] = '\0'; // if capacity allows, it is safe to do even if there are many references
    return _bytes;
}

// Return the hash value for a zero terminated character string.
//
// The algorithm is loosely based on Jenkins one-at-a-time hash function.
//
unsigned string::static_hash(const char* p, size_type size)
{
    if (size == 0)
        return 1;

    unsigned hash = size;

#define HASH_BYTE   hash += static_cast<unsigned>(static_cast<unsigned char>(*p++)); \
                    hash += hash << 10; \
                    hash ^= hash >> 6;

    unsigned octavas = (size + 7) >> 3;
    switch (size & 7)
    {
    case 0:
        do   // Duff's device super trick in action
        {
            HASH_BYTE
            case 7: HASH_BYTE
            case 6: HASH_BYTE
            case 5: HASH_BYTE
            case 4: HASH_BYTE
            case 3: HASH_BYTE
            case 2: HASH_BYTE
            case 1: HASH_BYTE
        } while ( --octavas != 0 );
    }

#undef HASH_BYTE

    // Final shuffling
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;

    if (hash == 0)
        ++hash; // hash value should never be zero
    return hash;
}

void string::resize(size_type new_size)
{
    size_type old_size = size();
    if (new_size != old_size)
    {
        if (is_shared() || new_size > old_size)
        {
            const size_type diff = new_size - old_size;
            char* buff = _append_uninitialized(diff);
            memset(buff, 0, diff);
        }
        else      // shrink self without a hassle
            _get_buffer()->_size = new_size;
    }
}

string& string::erase(size_type pos, size_type count)
{
    if (count != 0)
    {
        const size_type old_size = size();
        SSTL_ASSERT(pos < old_size); // otherwise pos can be anything
        const size_type delta = old_size - pos;
        if (count == npos || count > delta)
            count = delta;
        const size_type end_pos = pos + count;
        if (is_shared()) // have to reallocate anyway
        {
            const size_type new_size = old_size - count;
            size_type new_capacity = _adjust_capacity(new_size);
            _buffer_type* buff = _new_uninitialized_buffer(new_size, new_capacity);
            memcpy(buff->_bytes, _bytes, pos);
            memcpy(buff->_bytes + pos, _bytes + end_pos, old_size - end_pos);
            _get_buffer()->_ref_decrement();
            _bytes = buff->_bytes;
        }
        else
        {
            memmove(_bytes + pos, _bytes + end_pos, old_size - end_pos);
            _get_buffer()->_size -= count;
        }
    }
    return *this;
}

string string::substr(size_type pos, size_type count) const
{
    string result;
    result.assign(*this, pos, count);  // this will take care of all caveats
    return result;
}

string::size_type string::copy(char* dest, size_type count, size_type pos) const
{
    SSTL_ASSERT(pos <= size());
    const size_type delta = size() - pos;
    if (count == npos || count > delta)
        count = delta;
    memcpy(dest, _bytes + pos, count);
    return count;
}

void string::clear()
{
    if (_bytes != _empty_string_buffer._bytes)
    {
        if ( is_shared() ) // minimize hassle depending on whether or not this is shared
        {
            _get_buffer()->_ref_decrement();
            _clear_uninitizlized();
        }
        else
            _get_buffer()->_size = 0;
    }
}

int string::compare(const string& s) const
{
    if (_bytes == s._bytes) // fast version
        return 0;
    return compare(s._bytes, size());
}

int string::compare(const char* s) const
{
    size_type len = static_cast<size_type>(strlen(s));
    return compare(s, len);
}

int string::compare(const char* s, size_type len) const
{
    size_type size1 = size();
    int result;
    if (size1 == len)
        result = memcmp(_bytes, s, size1);
    else if (size1 < len)
    {
        result = memcmp(_bytes, s, size1);
        if ( result == 0 )
            result = -1;
    }
    else
    {
        SSTL_ASSERT(size1 > len);
        result = memcmp(_bytes, s, len);
        if ( result == 0 )
            result = 1;
    }
    return result;
}

bool string::operator==(const string& s) const
{
    if (_bytes == s._bytes)
        return true;
    const _buffer_type* b1 = _get_buffer();
    const _buffer_type* b2 = s._get_buffer();
    if ( b1->_size != b2->_size )
        return false;
    return memcmp(_bytes, s._bytes, b1->_size) == 0;
}

bool string::operator==(const char* s) const
{
    size_type len = static_cast<size_type>(strlen(s));
    if (len != size())
        return false;
    return memcmp(_bytes, s, len) == 0;
}

string string::operator+(char c) const
{
    return _op_plus_right(&c, 1);
}

string string::operator+(const char* s) const
{
    return _op_plus_right(s, static_cast<size_type>(strlen(s)));
}

string string::operator+(const string& s) const
{
    return _op_plus_right(s.data(), s.size());
}

string operator+(char c, const string& s2)
{
    return s2._op_plus_left(&c, 1);
}

string operator+(const char* s1, const string& s2)
{
    return s2._op_plus_left(s1, static_cast<string::size_type>(strlen(s1)));
}

string::size_type string::find(char ch, size_type pos) const
{
   const_iterator i = cbegin() + pos;
   const_iterator i_end = cend();
   for ( ; i < i_end; ++i) // when pos >= size() return npos, as specified
      if (*i == ch)
         return static_cast<size_type>(i - cbegin());
   return npos;
}

string::size_type string::find(const char* s, size_type pos, size_type count) const
{
    SSTL_ASSERT(s != NULL);
    size_type len = size();
    if (len - pos >= count && pos <= len)
    {
        if (count == 0) // this has to happen after the above if
            return pos;
        const char* d = data();
        const char* d_end = data() + len;
        const char* f = SSTL_NAMESPACE::search(d + pos, d_end, s, s + count);
        if (f < d_end)
            return static_cast<string::size_type>(f - d);
    }
    return npos;
}

string::size_type string::rfind(char ch, size_type pos) const
{
    size_type len = size();
    if (len != 0)
    {
        const char* d = data();
        const char* i = d + (pos < len ? pos : len - 1);
        for (; i >= d; --i)
            if (*i == ch)
                return static_cast<string::size_type>(i - d);
    }
    return npos;
}

string::size_type string::rfind(const char* s, size_type pos, size_type count) const
{
    SSTL_ASSERT(s != NULL);
    size_type len = size();
    if (pos < len && len - pos >= count)
        pos += count;
    else
        pos = len;
    if (count == 0)
        return pos;

    const char* d = data();
    const char* d_end = d + pos;
    const char* f = SSTL_NAMESPACE::find_end(d, d_end, s, s + count);
    if (f < d_end)
        return static_cast<string::size_type>(f - d);
    return npos;
}

void string::_reallocate(size_type new_capacity) const
{
    SSTL_ASSERT(size() <= new_capacity);

    new_capacity = _adjust_capacity(new_capacity);
    _buffer_type* buff = _new_uninitialized_buffer(size(), new_capacity);
    memcpy(buff->_bytes, _bytes, buff->_size);
    _get_buffer()->_ref_decrement();
    _bytes = buff->_bytes;
}

void string::_clear_uninitizlized()
{
    _empty_string_buffer._ref_increment();
    _bytes = _empty_string_buffer._bytes;
}

void string::_set_uninitialized(const char* str)
{
    const size_type len = static_cast<size_type>(strlen(str));
    _set_uninitialized(str, len);
}

void string::_set_uninitialized(const char* str, size_type size)
{
    if (size == 0)
        _clear_uninitizlized();
    else
    {
        _bytes = _new_uninitialized(size);
        memcpy(_bytes, str, size);
    }
}

void string::_set_uninitialized(size_type size, char c)
{
    if (size == 0)
        _clear_uninitizlized();
    else
    {
        _bytes = _new_uninitialized(size);
        memset(_bytes, c, size);
    }
}

void string::_set_uninitialized(const string& other)
{
    SSTL_ASSERT(&other != this);
    other._get_buffer()->_ref_increment();
    _bytes = other._bytes;
}

string::_buffer_type* string::_new_uninitialized_buffer(size_type size, size_type capacity)
{
    SSTL_ASSERT(size <= capacity);
    SSTL_ASSERT(capacity != 0);
    SSTL_ASSERT(capacity >= _minimum_capacity);
    unsigned buffer_sizeof = _buffer_type_header_sizeof + capacity;
    _buffer_type* buff = reinterpret_cast<_buffer_type*>(M_NEW char[buffer_sizeof]);
    buff->_hash = 0;
    buff->_capacity = capacity;
    buff->_size = size;
    buff->_ref_count = 0;
    return buff;
}

char* string::_new_uninitialized(size_type size)
{
    return _new_uninitialized_buffer(size, _adjust_capacity(size))->_bytes;
}

char* string::unshare()
{
    _buffer_type* buff = _get_buffer();
    if (buff != &_empty_string_buffer) // unsharing the empty buffer is not going to change it
    {
        if (buff->_ref_count > 0)
        {
            char* bytes = _new_uninitialized(buff->_size);
            memcpy(bytes, _bytes, buff->_size);
            _get_buffer()->_ref_decrement();
            _bytes = bytes;
            return bytes;
        }
        else
        {
            SSTL_ASSERT(!is_interned()); // attempt to modify a non-referenced interned string is made somehow
        }
    }
    return NULL;
}

char* string::_append_uninitialized(size_type count)
{
    SSTL_ASSERT(!is_interned()); // attempt to modify a read-only interned string is made
    size_type old_size = size();
    size_type new_size = old_size + count;
    if (is_shared() || new_size > capacity())
    {
        char* bytes = _new_uninitialized(new_size);
        memcpy(bytes, _bytes, old_size);
        #if SSTL_DEBUG
            memset(bytes + oldSize, 0, newSize - oldSize);
        #endif
        _get_buffer()->_ref_decrement();
        _bytes = bytes;
    }
    _get_buffer()->_size = new_size;
    return _bytes + old_size;
}

char* string::_insert_uninitialized(size_type index, size_type count)
{
    SSTL_ASSERT(!is_interned()); // attempt to modify a read-only interned string is made
    SSTL_ASSERT(index <= size());
    SSTL_ASSERT(count != npos);
    size_type old_size = size();
    size_type new_size = old_size + count;
    if (!is_shared() && new_size < capacity())
        memmove(_bytes + index + count, _bytes + index, old_size - index);
    else // grow    }
    {
        char* bytes = _new_uninitialized(new_size);
        memcpy(bytes, _bytes, index);
        #if SSTL_DEBUG
            memset(bytes + index, 0, count);
        #endif
        memcpy(bytes + index + count, _bytes + index, old_size - index);
        _get_buffer()->_ref_decrement();
        _bytes = bytes;
    }
    _get_buffer()->_size = new_size;
    return _bytes + index;
}

char* string::_replace_uninitialized(size_type pos, size_type count, size_type new_count)
{
    SSTL_ASSERT(!is_interned()); // attempt to modify a read-only interned string is made
    SSTL_ASSERT(pos <= size());
    SSTL_ASSERT(pos + count <= size());
    if (new_count < count) // shrink
        erase(pos + new_count, count - new_count);
    else if (new_count > count) // grow
        _insert_uninitialized(pos + count, new_count - count);
    return _bytes + pos;
}

string string::_op_plus_right(const char* s, size_type len) const
{
    string result;
    result.reserve(size() + len);
    result.append(*this);
    result.append(s, len);
    return result;
}

string string::_op_plus_left(const char* s, size_type len) const
{
    string result;
    result.reserve(size() + len);
    result.append(s, len);
    result.append(*this);
    return result;
}

// Support for string interning

class _intern_holder
{
public: // Constants:

    // Hash cell index decrement to find the next cell
    //
    static const int hashtable_secondary_shift = 1;

    // Default size of the hash table
    //
    static const int hashtable_default_size = 1024;

public:

    _intern_holder()
        :
          _capacity(0),
          _count(0),
          _buffers(NULL)
    {}

    ~_intern_holder()
    {
        MCriticalSection::Locker lock(_lock);
        string::_buffer_type** it = _buffers;
        string::_buffer_type** itEnd = _buffers + _capacity;
        for ( ; it != itEnd; ++it )
            if ( *it != NULL )
                (*it)->_ref_decrement();
        delete [] _buffers;
    }

    void add(string& str)
    {
        string::_buffer_type* buff = str._get_buffer();
        if (buff->_size == 0)
        {
            if (buff != &string::_empty_string_buffer)
            {
                buff->_ref_decrement();
                str._clear_uninitizlized();
            }
            return; // empty string should not be interned into a hash table
        }
        SSTL_ASSERT(buff->_hash == 0); // otherwise we would not be here
        buff->_hash = string::static_hash(buff->_bytes, buff->_size);

        MCriticalSection::Locker lock(_lock);
        string::_buffer_type** cell = find_cell_for_addition(buff->_hash, buff->_bytes, buff->_size);
        if (*cell != NULL)
        {
            buff->_ref_decrement();
            str._bytes = (*cell)->_bytes;
        }
        else
            *cell = buff;
        (*cell)->_ref_increment();
    }

    string::_buffer_type* add(const char* str, unsigned size)
    {
        if (size == 0)
        {
            string::_empty_string_buffer._ref_increment();
            return &string::_empty_string_buffer; // special value, always interned
        }

        unsigned hash = string::static_hash(str, size);

        MCriticalSection::Locker lock(_lock);
        string::_buffer_type** cell =  find_cell_for_addition(hash, str, size);
        if (*cell == NULL)
        {
            string::_buffer_type* buff = string::_new_uninitialized_buffer(size, _adjust_capacity(size));
            memcpy(buff->_bytes, str, size);
            buff->_hash = hash;
            *cell = buff;
        }

        (*cell)->_ref_increment();
        return *cell;
    }

    void OptimizeAndGarbageCollect()
    {
        if (_count != 0)
        {
            MCriticalSection::Locker lock(_lock);
            resize(_capacity);
        }
    }

    void resize(int new_capacity);

    string::_buffer_type** find_cell_for_addition(unsigned hash, const char* bytes, unsigned size);

    static _intern_holder* get_global()
    {
        static _intern_holder holder;
        return &holder;
    }

private:

    int _capacity; // has to be power of two
    int _count;
    string::_buffer_type** _buffers;
    MCriticalSection _lock;
};

string::_buffer_type** _intern_holder::find_cell_for_addition(unsigned hash, const char* bytes, unsigned size)
{
    if (_capacity <= (_count << 1))
        resize(_capacity == 0 ? hashtable_default_size : _capacity + _capacity);

    int index = static_cast<int>(hash & (_capacity - 1u)); // normalize hash into index
    string::_buffer_type** bb;
    for (;;)
    {
        bb = &_buffers[index];
        const string::_buffer_type* b = *bb;
        if ( b == NULL )
        {
            ++_count;  // we know there will be a new item
            return bb;  // empty place to add the new item
        }
        if ( b->_hash == hash && b->_size == size && memcmp(b->_bytes, bytes, size) == 0 )
            return bb;  // the same item is found

        // Otherwise calculate the second-grade hash value, derivative from one given
        index -= hashtable_secondary_shift;
        if ( index < 0 ) // assume proper overflow behavior...
            index += _capacity;
    }
}

void _intern_holder::resize(int new_capacity)
{
    SSTL_ASSERT(new_capacity >= _capacity);
    SSTL_ASSERT((new_capacity & (new_capacity - 1)) == 0); // newCapacity is the power of two

    string::_buffer_type** new_buffers = M_NEW string::_buffer_type*[new_capacity];

    string::_buffer_type** i = new_buffers;
    string::_buffer_type** i_end = new_buffers + new_capacity;
    for (; i != i_end; ++i)
        *i = NULL;

    if (_count != 0)
    {
        int new_count = 0;
        i = _buffers;
        i_end = _buffers + _capacity;
        for (; i != i_end; ++i)
        {
            string::_buffer_type* buff = *i;
            if (buff != NULL)
            {
                SSTL_ASSERT(buff->_hash != 0);
                if (buff->_ref_count == 0) // orphaned item to garbage collect
                    delete [] buff;
                else
                {
                    SSTL_ASSERT(buff->_ref_count > 0);

                    int index = static_cast<int>(buff->_hash & (_capacity - 1u)); // normalize hash into index
                    string::_buffer_type** bb;
                    for (;;)
                    {
                        bb = &_buffers[index];
                        const string::_buffer_type* b = *bb;
                        if (b == NULL)
                        {
                            *bb = buff; // relocate
                            ++new_count;
                            break;
                        }

                        // Otherwise calculate the second-grade hash value, derivative from one given
                        index -= hashtable_secondary_shift;
                        if ( index < 0 ) // assume proper overflow behavior...
                            index += _capacity;
                    }
                }
            }
        }
        SSTL_ASSERT(_count >= new_count);
        _count = new_count;
    }
    delete [] _buffers;
    _buffers = new_buffers;
    _capacity = new_capacity;
}

void string::intern()
{
    if (!is_interned())
        _intern_holder::get_global()->add(*this);
}

string string::intern_create(const char* s)
{
    return _intern_holder::get_global()->add(s, static_cast<size_type>(strlen(s)));
}

string string::intern_create(const char* s, size_type size)
{
    return _intern_holder::get_global()->add(s, size);
}

void string::intern_cleanup(time_t secondsSincePrevious)
{
    if ( secondsSincePrevious > 0 )
    {
        static time_t s_lastTime = 0; // time in the past

#if !M_NO_TIME
        time_t now = MTime::GetUtcSecondsSince1970();
#else
        time_t now = time(NULL);
#endif
        if ( now - s_lastTime < secondsSincePrevious )
            return;
        s_lastTime = now;
    }
    _intern_holder::get_global()->OptimizeAndGarbageCollect();
}

}

#endif // !M_NO_VARIANT
