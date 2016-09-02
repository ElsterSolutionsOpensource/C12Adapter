#ifndef MCORE_MSHAREDSTRING_H
#define MCORE_MSHAREDSTRING_H
/// \file MCORE/MSharedString.h
/// \addtogroup MCORE
///@{

#include <MCORE/MInterlocked.h>

#if !M_NO_VARIANT

/// This is still internal
/// \cond SHOW_INTERNAL

namespace shared {

typedef unsigned sstl_size_type;
typedef int sstl_difference_type;
typedef Muint32 sstl_uint32;
typedef Muint64 sstl_uint64;
#define SSTL_NAMESPACE std
#define SSTL_CONSTEXPR
#define SSTL_ASSERT(e) M_ASSERT(e)
#define SSTL_STATIC_ASSERT(e, s) M_COMPILED_ASSERT(e)

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

class M_CLASS string
{
    friend class _intern_holder;

public:
    typedef char value_type;
    typedef sstl_size_type size_type;
    typedef sstl_difference_type difference_type;
    typedef char& reference;
    typedef const char& const_reference;
    typedef char* pointer;
    typedef const char* const_pointer;
    typedef char* iterator;
    typedef const char* const_iterator;
    typedef SSTL_NAMESPACE::reverse_iterator<iterator> reverse_iterator;
    typedef SSTL_NAMESPACE::reverse_iterator<const_iterator> const_reverse_iterator;

    // Private shared constant string buffer.
    //
    // This type has to be a POD because it can be statically initialized
    //
    struct _buffer_type
    {
    public: // Data:

        // Hash value, if the string is interned.
        //
        // Once the hash is computed, the string becomes constant and it cannot change.
        // There is a debug check to verify this.
        //
        unsigned _hash;

        // Capacity of the buffer in bytes
        //
        unsigned _capacity;

        // Size of the string in the buffer
        //
        unsigned _size;

        // Reference counter for this buffer.
        // Zero means one single reference, and negative value is no references.
        // Cannot use atomic_int as not all compilers still support pods with constructors.
        //
        mutable volatile int _ref_count;

        // Buffer with fake size for storing the string.
        //
        union
        {
            char _bytes [ 16 ];
            sstl_uint64 _qwords [ 2 ]; // this ensures the best possible alignment for data
        };

    public:

        void _ref_increment() const
        {
            MInterlocked::FetchAndIncrement(&_ref_count);
        }

        void _ref_decrement() const
        {
            if ( MInterlocked::FetchAndDecrement(&_ref_count) <= 0 )
                delete [] (char*)this;
        }
    };

public: // Constants:

    static const size_type npos = 0xFFFFFFFF;
    static const unsigned _buffer_type_header_sizeof = sizeof(_buffer_type) - sizeof(sstl_uint64) * 2;
    static const unsigned _minimum_capacity = sizeof(sstl_uint64) * 2;

public:

    string()
    {
        _clear_uninitizlized();
    }

    /// Construct the string from the given buffer, give the buffer directly.
    ///
    /// This operation does not change buffer reference counter.
    ///
    /// \param b Buffer from which to construct the string.
    ///
    string(_buffer_type* b)
    {
        SSTL_ASSERT(b != NULL);
        _bytes = b->_bytes;
    }

    string(const char* s)
    {
        _set_uninitialized(s);
    }

    string(const char* s, size_type size)
    {
        _set_uninitialized(s, size);
    }

    string(const string& str, size_type pos, size_type count = string::npos)
    {
        SSTL_ASSERT(str.size() >= pos);
        const size_type max_count = str.size() - pos;
        if (count > max_count)
            count = max_count;
        _set_uninitialized(str.data() + pos, count);
    }

    string(size_type size, char c)
    {
        _set_uninitialized(size, c);
    }

    string(const_iterator begin, const_iterator end)
    {
        SSTL_ASSERT(end >= begin);
        const size_type size = static_cast<size_type>(end - begin);
        _set_uninitialized(begin, size);
    }

    string(iterator begin, iterator end)
    {
        SSTL_ASSERT(end >= begin);
        const size_type size = static_cast<size_type>(end - begin);
        _set_uninitialized(begin, size);
    }

    /// Construct from generic iterator range
    ///
    /// This constructor will be used for every iterator type but const_iterator
    ///
    template
        <class Iter>
    string(Iter begin, Iter end)
    {
        SSTL_ASSERT(end >= begin);
        const size_type size = static_cast<size_type>(end - begin);
        if (size == 0)
            _clear_uninitizlized();
        else
        {
            _bytes = _new_uninitialized(size);
            char* b = _bytes;
            for ( ; begin != end; ++begin, ++b )
                *b = *begin;
        }
    }

    string(const string& other)
    {
        _set_uninitialized(other);
    }

    ~string()
    {
        _get_buffer()->_ref_decrement();
    }

public:

    iterator begin()  {unshare(); return _bytes;}
    iterator end()    {unshare(); return _bytes + size();}

    const_iterator begin() const  {return _bytes;}
    const_iterator end() const    {return _bytes + size();}

    const_iterator cbegin() const {return _bytes;}
    const_iterator cend() const   {return _bytes + size();}

    reverse_iterator rbegin() {return reverse_iterator(end());}
    reverse_iterator rend()   {return reverse_iterator(begin());}

    const_reverse_iterator rbegin() const {return const_reverse_iterator(end());}
    const_reverse_iterator rend() const   {return const_reverse_iterator(begin());}

    const_reverse_iterator crbegin() const {return const_reverse_iterator(cend());}
    const_reverse_iterator crend() const   {return const_reverse_iterator(cbegin());}

    char& front()
    {
        SSTL_ASSERT(!empty());
        return _bytes[0];
    }
    const char& front() const
    {
        SSTL_ASSERT(!empty());
        return _bytes[0];
    }

    char& back()
    {
        SSTL_ASSERT(!empty());
        return _bytes[size() - 1];
    }
    const char& back() const
    {
        SSTL_ASSERT(!empty());
        return _bytes[size() - 1];
    }

    ///@{
    /// Get the number of characters in the string, zero character is not included.
    ///
    size_type size() const
    {
        return _get_buffer()->_size;
    }
    size_type length() const
    {
        return size();
    }
    ///@}

    bool empty() const
    {
        return size() == 0;
    }

    unsigned capacity() const
    {
        return _get_buffer()->_capacity;
    }

    void reserve(size_type reserved_size);

    void shrink_to_fit();

    /// Access the data of string, not zero terminated.
    ///
    const char* data() const
    {
        return _bytes;
    }

    /// Access zero terminated string.
    ///
    /// \attention Calling c_str can invalidate constant iterators to string object.
    ///
    const char* c_str() const;

    string& operator=(char c)
    {
        return assign(1, c);
    }
    string& operator=(const char* str)
    {
        return assign(str);
    }
    string& operator=(const string& other)
    {
        return assign(other);
    }

    string& assign(size_type size, char c);
    string& assign(const char* str);
    string& assign(const char* str, size_type size);
    string& assign(const_iterator begin, const_iterator end);
    string& assign(iterator begin, iterator end)
    {
        return assign(const_cast<const_iterator>(begin), const_cast<const_iterator>(end));
    }

    /// Assign string from a generic iterator range.
    ///
    /// This method is selected by the compiler for reverse iterators and so on,
    /// while for everything else such as const_iterator and iterator there are
    /// more efficient implementations with explicit parameters.
    ///
    template
        <class Iter>
    string& assign(Iter begin, Iter end)
    {
        SSTL_ASSERT(end >= begin);
        _get_buffer()->_ref_decrement();
        const size_type size = static_cast<size_type>(end - begin);
        if (size == 0)
            _clear_uninitizlized();
        else
        {
            _bytes = _new_uninitialized(size);
            char* buff = _bytes;
            for ( ; begin != end; ++begin, ++buff)
                *buff = *begin;
        }
        return *this;
    }

    string& assign(const string& other);

    string& assign(const string& str, size_type pos, size_type count);

    string& operator+=(char c)
    {
        return push_back(c);
    }

    string& operator+=(const char* c)
    {
        return append(c);
    }

    string& operator+=(const string& s)
    {
        return append(s);
    }

    string& push_back(char c);

    /// Removes the last character.
    ///
    /// \since C++11, C++ programming language standard approved by the 
    ///      International Organization for Standardization (ISO) on August 12, 2011.
    ///
    void pop_back()
    {
        SSTL_ASSERT(!empty());
        _get_buffer()->_size--;
    }

    string& append(size_type size, char c);
    string& append(const char* str);
    string& append(const char* str, size_type size);
    string& append(const string& other);
    string& append(const_iterator begin, const_iterator end);
    string& append(iterator begin, iterator end)
    {
        return append(const_cast<const_iterator>(begin), const_cast<const_iterator>(end));
    }

    template
        <class Iter>
    string& append(Iter begin, Iter end)
    {
        SSTL_ASSERT(begin <= end);
        const size_type size = static_cast<size_type>(end - begin);
        if (size > 0)
        {
            char* buff = _append_uninitialized(size);
            for ( ; begin != end; ++begin, ++buff)
                *buff = *begin;
        }
        return *this;
    }

    void resize(size_type new_size);

    string substr(size_type pos = 0, size_type count = npos) const;

    void clear();

    size_type copy(char* dest, size_type count, size_type pos = 0) const;

    static unsigned static_hash(const char* p, size_type size);

    unsigned hash() const
    {
        return static_hash(_bytes, size());
    }

    string& erase(size_type pos, size_type count);
    iterator erase(const_iterator position);
    iterator erase(const_iterator first, const_iterator last);

    iterator insert(const_iterator where, char ch);
    iterator insert(const_iterator where, size_type count, char c);
    string& insert(size_type pos, size_type count, char c);
    string& insert(size_type pos, const char* s);
    string& insert(size_type pos, const char* s, size_type count);
    string& insert(size_type pos, const string& str);
    string& insert(size_type pos, const string& str, size_type str_pos, size_type str_count);
    iterator insert(const_iterator where, const_iterator input_first, const_iterator input_last);

    template
        <class InputIt>
    iterator insert(const_iterator where, InputIt input_first, InputIt input_last)
    {
        SSTL_ASSERT(input_first <= input_last);
        SSTL_ASSERT(where >= _bytes && where <= _bytes + size());
        size_type pos = where - _bytes;
        size_type count = input_last - input_first;
        if (count == 0)
            return const_cast<iterator>(where);
        char* buff = _insert_uninitialized(pos, count);
        for ( ; input_first != input_last; ++input_first, ++buff )
           buff = *input_first;
        return buff;
    }

    string& replace(size_type pos, size_type count, const string& str);
    string& replace(const_iterator first, const_iterator last, const string& str);
    string& replace(size_type pos, size_type count, const string& str, size_type strPos, size_type strCount);
    string& replace(size_type pos, size_type count, const char* s, size_type sCount);
    string& replace(const_iterator first, const_iterator last, const char* s, size_type strCount);
    string& replace(size_type pos, size_type count, const char* s);
    string& replace(const_iterator first, const_iterator last, const char* s);
    string& replace(size_type pos, size_type count, size_type c_count, char c);
    string& replace(const_iterator first, const_iterator last, size_type cCount, char c);
    string& replace(const_iterator first, const_iterator last, const_iterator input_first, const_iterator input_last);

    template
        <class InputIt>
    string& replace(const_iterator first, const_iterator last, InputIt input_first, InputIt input_last)
    {
        SSTL_ASSERT(first >= _bytes);
        SSTL_ASSERT(last <= _bytes + size());
        SSTL_ASSERT(input_first <= input_last);
        size_type pos = first - _bytes;
        size_type count = last - first;
        size_type newCount = input_last - input_first;
        char* buff = _replace_uninitialized(pos, count, newCount);
        for ( ; input_first != input_last; ++input_first, ++buff )
            *buff = *input_first;
        return *this;
    }

    int compare(const string& str) const;
    int compare(const char* str) const;
    int compare(const char* str, size_type size) const;

    bool operator==(const string& s) const;
    bool operator==(const char* s) const;
    friend bool operator==(const char* s1, const string& s2)
    {
        return s2.operator==(s1);
    }

    bool operator!=(const string& s) const
    {
        return !operator==(s);
    }
    bool operator!=(const char* s) const
    {
        return !operator==(s);
    }
    friend bool operator!=(const char* s1, const string& s2)
    {
        return !s2.operator==(s1);
    }

    bool operator<(const string& s) const
    {
        return compare(s) < 0;
    }
    bool operator<(const char* s) const
    {
        return compare(s) < 0;
    }
    friend bool operator<(const char* s1, const string& s2)
    {
        return s2.compare(s1) >= 0;
    }

    bool operator<=(const string& s) const
    {
        return compare(s) <= 0;
    }
    bool operator<=(const char* s) const
    {
        return compare(s) <= 0;
    }
    friend bool operator<=(const char* s1, const string& s2)
    {
        return s2.compare(s1) > 0;
    }

    bool operator>(const string& s) const
    {
        return compare(s) > 0;
    }
    bool operator>(const char* s) const
    {
        return compare(s) > 0;
    }
    friend bool operator>(const char* s1, const string& s2)
    {
        return s2.compare(s1) <= 0;
    }

    bool operator>=(const string& s) const
    {
        return compare(s) >= 0;
    }
    bool operator>=(const char* s) const
    {
        return compare(s) >= 0;
    }
    friend bool operator>=(const char* s1, const string& s2)
    {
        return s2.compare(s1) < 0;
    }

    int compare(const std::string& str) const
    {
       return compare(str.data(), static_cast<size_type>(str.size()));
    }
    bool operator==(const std::string& s) const
    {
       return compare(s.data(), static_cast<size_type>(s.size())) == 0;
    }
    friend bool operator==(const std::string& s1, const string& s2)
    {
       return s2.compare(s1.data(), static_cast<size_type>(s1.size())) == 0;
    }
    bool operator!=(const std::string& s) const
    {
        return !operator==(s);
    }
    friend bool operator!=(const std::string& s1, const string& s2)
    {
       return !s2.operator==(s1);
    }
    bool operator<(const std::string& s) const
    {
       return compare(s.data(), static_cast<size_type>(s.size())) < 0;
    }
    friend bool operator<(const std::string& s1, const string& s2)
    {
        return s2.compare(s1.data(), static_cast<size_type>(s1.size())) >= 0;
    }
    bool operator<=(const std::string& s) const
    {
       return compare(s.data(), static_cast<size_type>(s.size())) <= 0;
    }
    friend bool operator<=(const std::string& s1, const string& s2)
    {
        return s2.compare(s1.data(), static_cast<size_type>(s1.size())) > 0;
    }
    bool operator>(const std::string& s) const
    {
       return compare(s.data(), static_cast<size_type>(s.size())) > 0;
    }
    friend bool operator>(const std::string& s1, const string& s2)
    {
        return s2.compare(s1.data(), static_cast<size_type>(s1.size())) <= 0;
    }
    bool operator>=(const std::string& s) const
    {
       return compare(s.data(), static_cast<size_type>(s.size())) >= 0;
    }
    friend bool operator>=(const std::string& s1, const string& s2)
    {
        return s2.compare(s1.data(), static_cast<size_type>(s1.size())) < 0;
    }

    string operator+(char c) const;
    string operator+(const char* s) const;
    string operator+(const string& s) const;
    friend string operator+(char c, const string& s2);
    friend string operator+(const char* s1, const string& s2);

    /// Nonconstant indexing operator only assumes immediate assignment, no reference shall be stored to assign later.
    ///
    char& operator[](size_type i)
    {
        SSTL_ASSERT(i < size());
        unshare();
        return _bytes[i];
    }
    char operator[](size_type i) const
    {
        SSTL_ASSERT(i < size());
        return _bytes[i];
    }

    /// Nonconstant indexing operator only assumes immediate assignment, no reference shall be stored to assign later.
    ///
    char& at(size_type i)
    {
        SSTL_ASSERT(i < size());
        unshare();
        return _bytes[i];
    }
    char at(size_type i) const
    {
        SSTL_ASSERT(i < size());
        return _bytes[i];
    }

    void swap(string& other)
    {
        char* bytes = _bytes; // works if other is this
        _bytes = other._bytes;
        other._bytes = bytes;
    }

    size_type find(char ch, size_type pos = 0) const;
    size_type find(const char* s, size_type pos, size_type count) const;
    size_type find(const char* s, size_type pos = 0) const     {return find(s, pos, static_cast<size_type>(strlen(s)));}
    size_type find(const string& str, size_type pos = 0) const {return find(str.data(), pos, str.size());}

    size_type rfind(char ch, size_type pos = string::npos) const;
    size_type rfind(const char* s, size_type pos, size_type count) const;
    size_type rfind(const char* s, size_type pos = string::npos) const     {return rfind(s, pos, static_cast<size_type>(strlen(s)));}
    size_type rfind(const string& str, size_type pos = string::npos) const {return rfind(str.data(), pos, str.size());}

    bool is_shared() const
    {
        return _get_buffer()->_ref_count > 0;
    }

    bool is_interned() const
    {
        return _get_buffer()->_hash != 0;
    }

    char* unshare();

    void intern();
    static string intern_create(const char* s);
    static string intern_create(const char* s, size_type size);
    static void intern_cleanup(time_t secondsSincePrevious = 60);

public:


    _buffer_type* _get_buffer()
    {
        return reinterpret_cast<_buffer_type*>(_bytes - _buffer_type_header_sizeof); // offset from header
    }
    const _buffer_type* _get_buffer() const
    {
        return reinterpret_cast<const _buffer_type*>(_bytes - _buffer_type_header_sizeof); // offset from header
    }

    void _clear_uninitizlized();

    void _set_uninitialized(char c)
    {
        _bytes[0] = c;
    }

    void _set_uninitialized(const char* str);

    void _set_uninitialized(const char* str, size_type size);

    void _set_uninitialized(size_type size, char c);

    void _set_uninitialized(const string& other);

    char* _insert_uninitialized(size_type index, size_type count);

    char* _append_uninitialized(size_type count);

    char* _replace_uninitialized(size_type index, size_type count, size_type new_count);

    void _reallocate(size_type new_capacity) const;

    static _buffer_type* _new_uninitialized_buffer(size_type size, size_type capacity);

    static char* _new_uninitialized(size_type size);

    string _op_plus_right(const char* s, size_type len) const;
    string _op_plus_left(const char* s, size_type len) const;

public:

    /// Static empty string that is suitable for context where empty string is necessary.
    ///
    static string _empty_string;

private: // Data:

    // Pointer to bytes field of private Buffer class in the string.
    //
    mutable char* _bytes;

    // Private static member that holds the buffer with string "".
    // The MSharedString object default constructor makes the object
    // point to this buffer.
    //
    static _buffer_type _empty_string_buffer;
};

}

typedef shared::string MSharedString;

/// This is still internal
/// \endcond SHOW_INTERNAL

#endif // !M_NO_VARIANT

///@}
#endif
