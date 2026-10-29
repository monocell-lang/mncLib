#pragma once
#include <cstdio>
#include <cstring>
#include "compare.h"
#include "utility.h"
#include "format.h"


#define _MNC_BEGIN namespace mnc {
#define _MNC_END }
#define _GLOBAL _MNC_END

_MNC_BEGIN

//////////////////////////////////////////////////////////////////////
//////////////////////////// class: Slice ////////////////////////////
//////////////////////////////////////////////////////////////////////
#pragma region class: Slice

class String;

class Slice {
private:

    struct __throw 
    {
        static void STRING_SLICE_Acessing_Outside_String()
        {
            std::fprintf(stderr, "[mnc::Slice] ;; ABORT WARNING ;; accessing outside string\n");
            std::fflush(stderr);
            std::abort();
        }
    };

    friend class Comparator<Slice, Ordering::Order>;
    friend class Comparator<String, Ordering::Order>;
    
private:

    char const* _data;
    ulong len;

public:

    constexpr Slice(): _data(""), len(0) {}
    constexpr explicit Slice(char const* data, ulong len): _data(data), len(len) {}
    constexpr Slice(Slice const&) = default;
    constexpr Slice(Slice&&) = default;
    constexpr Slice& operator=(Slice const&) = default;
    constexpr Slice& operator=(Slice&&) = default;
    constexpr ~Slice() = default;

    template <ulong N>
    constexpr explicit Slice(char const (&data)[N]): _data(data), len(N - 1) {}

    template <ulong N>
    static constexpr Slice make(char const (&data)[N]) { return Slice(data); }

    //////////////////////////////////////////////////////////////////////
    /////////////////////////////// fn: basic ////////////////////////////
    //////////////////////////////////////////////////////////////////////

    constexpr ulong size() const
    {
        return len;
    }

    constexpr bool is_empty() const
    {
        return len == 0;
    }

    constexpr Slice clone() const
    {
        return *this;
    }

    constexpr char const* data() const
    {
        return _data;
    }

    constexpr char const* begin() const
    {
        return _data;
    }

    constexpr char const* end() const
    {
        return _data + len;
    }

    bool overlaps(Slice other) const
    {
        // CASE A
        // s1:    [begin     end]
        // s2:               [begin     end]
        // ------------------------------------------------
        // A = s2.begin >= s1.end
        // !A = s2.begin < s1.end

        // CASE B
        // s1:              [begin     end]
        // s2:   [begin     end]
        // ------------------------------------------------
        // B = s1.begin >= s2.end
        // !B = s1.begin < s2.end

        // CASE A
        // CASE B
        // ------------------------------------------------
        // !A && !B = (s2.begin < s1.end) && (s1.begin < s2.end)

        if (len == 0 || other.len == 0)
            return false;

        return (other.begin() < end()) && (begin() < other.end());
    }

    bool has_subslice(Slice other) const
    {
        if (len == 0 || other.len == 0)
            return false;

        return (begin() <= other.begin()) && (other.end() <= end());
    }

    //////////////////////////////////////////////////////////////////////
    //////////////////////////// ACCESS ELEMENTS /////////////////////////
    //////////////////////////////////////////////////////////////////////

    constexpr char const& operator[](ulong index) const&
    {
        if (index >= len)
            __throw::STRING_SLICE_Acessing_Outside_String();

        return _data[index];
    }

    constexpr char const&& operator[](ulong index) const&&
    {
        if (index >= len)
            __throw::STRING_SLICE_Acessing_Outside_String();

        return static_cast<char const&&>(_data[index]);
    }

    constexpr Option<char const&> front() const
    {
        if (len == 0) 
            return { False{} };
        
        return { True{}, _data[0] };
    }

    constexpr Option<char const&> back() const
    {
        if (len == 0) 
            return { False{} };

        return { True{}, _data[len - 1] };
    }

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: compare ////////////////////////////
    //////////////////////////////////////////////////////////////////////

private:

    static constexpr Order compare(Slice left, Slice right)
    {
        ulong min_len = (left.len < right.len? left.len: right.len);

        if consteval
        {
            for (ulong i = 0; i < min_len; i++)
            {
                if (left._data[i] < right._data[i])
                    return Order::Less;
                else if (left._data[i] > right._data[i])
                    return Order::Greater;
            }
        }
        else
        {
            int first_diff = std::memcmp(left._data, right._data, min_len);
            if (first_diff < 0)
                return Order::Less;
            else if (first_diff > 0)
                return Order::Greater;
        }

        if (left.len < right.len)
            return Order::Less;
        else if (left.len > right.len)
            return Order::Greater;
        else
            return Order::Equal;
    }

public:

    constexpr bool operator==(Slice other) const
    {
        return compare(*this, other) == Order::Equal;
    }

    constexpr bool operator!=(Slice other) const
    {
        return compare(*this, other) != Order::Equal;
    }

    constexpr bool operator<(Slice other) const
    {
        return compare(*this, other) == Order::Less;
    }

    constexpr bool operator>(Slice other) const
    {
        return compare(*this, other) == Order::Greater;
    }

    constexpr bool operator<=(Slice other) const
    {
        return compare(*this, other) != Order::Greater;
    }

    constexpr bool operator>=(Slice other) const
    {
        return compare(*this, other) != Order::Less;
    }

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: slice //////////////////////////////
    //////////////////////////////////////////////////////////////////////

    constexpr Slice slice(ulong start, ulong max_len) const 
    {
        if (start > len || max_len == 0)
        {
            return Slice {};
        }
        else
        {
            return Slice {
                _data + start, 
                (max_len < len - start? max_len: len - start)
            };
        }
    }

    constexpr Pair<Slice, Slice> split(ulong right_start) const
    {
        if (right_start == 0)
        {
            return {
                Slice {},
                Slice { _data, len }
            };
        }
        else if (right_start >= len)
        {
            return {
                Slice { _data, len },
                Slice {}
            };
        }
        else
        {
            return {
                Slice { _data, right_start },
                Slice { _data + right_start, len - right_start }
            };
        }
    }

    constexpr Slice drop_head(ulong max_count) const
    {
        if (max_count > len) 
            return Slice {};

        return Slice { _data + max_count, len - max_count };
    }

    constexpr Slice drop_tail(ulong max_count) const
    {
        if (max_count > len) 
            return Slice {};

        return Slice { _data, len - max_count };
    }

    constexpr String into_str() const;

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: find ///////////////////////////////
    //////////////////////////////////////////////////////////////////////

    constexpr Option<ulong> find(char ch, bool find_not, ulong start_index = 0) const 
    {
        if (start_index >= len) 
            return { False{} };

        if consteval 
        {
            if (!find_not)
            {
                for (ulong i = start_index; i < len; i++) {
                    if (_data[i] == ch) return { True{}, i };
                }
                return { False{} };
            }
            else
            {
                for (ulong i = start_index; i < len; i++) {
                    if (_data[i] != ch) return { True{}, i };
                }
                return { False{} };
            }
        } 
        else 
        {
            if (!find_not)
            {
                char const* ptr = (char const*)std::memchr(_data + start_index, ch, len - start_index);
                if (ptr != nullptr) {
                    return { True{}, ptr - _data };
                }
                return { False{} };
            }
            else
            {
                for (ulong i = start_index; i < len; i++) {
                    if (_data[i] != ch) return { True{}, i };
                }
                return { False{} };
            }
        }
    }

    constexpr Option<ulong> rfind(char ch, bool find_not) const 
    {
        if (len == 0) 
            return { False{} };

        if (!find_not)
        {
            for (ulong i = len; i > 0; i--) {
                if (_data[i - 1] == ch) return { True{}, i - 1 };
            }
            return { False{} };
        }
        else
        {
            for (ulong i = len; i > 0; i--) {
                if (_data[i - 1] != ch) return { True{}, i - 1 };
            }
            return { False{} };
        }

    }
};

_GLOBAL constexpr mnc::Slice operator""_slice(char const* _, mnc::ulong __) 
{
    return mnc::Slice { _, __ };
} 
_MNC_BEGIN

template <>
class Comparator<Slice, Ordering::Order> {
public:

    static constexpr Order compare(Slice left, Slice right)
    {
        return Slice::compare(left, right);
    }
};

template <>
class Formatter<Slice> {
private:

    Formatter<char const*> base;

public:

    consteval Formatter() {}
    consteval Formatter(char const* params, ulong len): base(params, len) {}
    template <ulong N>
    consteval Formatter(char const (&params)[N]): base(params, N - 1) {}

    template <typename Buffer>
    requires (has_buffer_trait<Buffer>)
    constexpr Result<void, FormatError> format(Buffer& buffer, Slice value, FormatIR IR = FormatIR{}) const
    {
        return base.format(buffer, value.data(), value.size(), IR);
    }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// class: String ///////////////////////////
//////////////////////////////////////////////////////////////////////
#pragma region class: String

class String {
private:

    friend class Slice;

    //////////////////////////////////////////////////////////////////////
    //////////////////////////////// helpers /////////////////////////////
    //////////////////////////////////////////////////////////////////////

    struct __throw
    {
        static void STRING_Acessing_Outside_String()
        {
            std::fprintf(stderr, "[mnc::String] ;; ABORT WARNING ;; accessing outside string\n");
            std::fflush(stderr);
            std::abort();
        }

        static void STRING_Writing_From_Null_Character_Stream()
        {
            std::fprintf(stderr, "[mnc::String] ;; ABORT WARNING ;; writing from null character stream\n");
            std::fflush(stderr);
            std::abort();
        }

        static void STRING_Memory_Request_Failed()
        {
            std::fprintf(stderr, "[mnc::String] ;; ABORT WARNING ;; memory request failed\n");
            std::fflush(stderr);
            std::abort();
        }
    };

    struct helper
    {
        static constexpr ulong goodsize(ulong n) 
        {
            return 3 * n / 2 + 1;
        }

        static constexpr char* alloc(ulong size)
        {
            char* result = new(std::nothrow) char[size + 1];
            if (result == nullptr)
                __throw::STRING_Memory_Request_Failed();
            return result;
        }

        static constexpr void memcpy(char* dest, char const* src, ulong cpy_len)
        {
            if consteval 
            {
                for (ulong i = 0; i < cpy_len; i++) {
                    dest[i] = src[i];
                }
            }
            else
            {
                std::memcpy(dest, src, cpy_len);
            }
        }

        static constexpr void memset(char* dest, char ch, ulong count)
        {
            if consteval 
            {
                for (ulong i = 0; i < count; i++) {
                    dest[i] = ch;
                }
            }
            else
            {
                std::memset(dest, ch, count);
            }
        }
    };

    //////////////////////////////////////////////////////////////////////
    //////////////////////////////// MAIN ////////////////////////////////
    //////////////////////////////////////////////////////////////////////

private:

    union {
        struct { char buffer[16]; } stack;
        struct { char* buffer; ulong cap; } heap;
    };
    
    ulong len; // bit 0 = HEAP FLAG | bits 1-63 = actual length
    static constexpr ulong STACK_CAPACITY = 15;

public:

    constexpr String()
    {
        len = 0;
        stack.buffer[0] = '\0';
    }

    constexpr explicit String(Slice other): stack{}
    {
        if (other.size() <= STACK_CAPACITY) // STACK
        {
            helper::memcpy(stack.buffer, other.data(), other.size());
            stack.buffer[other.size()] = '\0';
            len = (other.size() << 1) | 0;
        }
        else // HEAP
        {
            heap.cap = helper::goodsize(other.size());
            heap.buffer = helper::alloc(heap.cap + 1);
            helper::memcpy(heap.buffer, other.data(), other.size());
            heap.buffer[other.size()] = '\0';
            len = (other.size() << 1) | 1;
        }
    }

    constexpr explicit String(String const& other): String(other.as_slice()) {}

    constexpr String(String&& other): stack{}
    {
        if (other.len & 1)
        {
            heap.cap = other.heap.cap;
            heap.buffer = other.heap.buffer;
            len = other.len;

            // make-default other
            other.len = 0;
            other.stack.buffer[0] = '\0';
        }
        else
        {
            if consteval 
            {
                helper::memcpy(stack.buffer, other.stack.buffer, other.size());
                stack.buffer[other.size()] = '\0';
            }
            else
            {
                helper::memcpy(stack.buffer, other.stack.buffer, 16);
            }
            len = other.len;

            // make-default other
            other.len = 0;
            other.stack.buffer[0] = '\0';
        }
    }

    constexpr String& operator=(Slice other)
    {
        String dummy;
        if consteval
        {
            dummy = other.into_str();
            other = dummy.as_slice();
        }
        else
        {
            if (as_slice().overlaps(other))
            {
                return operator=(other.into_str().as_slice());
            }
        }

        if (len & 1)
        {
            if (heap.cap >= other.size())
            {
                helper::memcpy(heap.buffer, other.data(), other.size());
                heap.buffer[other.size()] = '\0';
                len = (other.size() << 1) | 1;
            }
            else
            {
                delete[] heap.buffer;
                heap.cap = helper::goodsize(other.size());
                heap.buffer = helper::alloc(heap.cap + 1);
                helper::memcpy(heap.buffer, other.data(), other.size());
                heap.buffer[other.size()] = '\0';
                len = (other.size() << 1) | 1;
            }
        }
        else
        {
            if (STACK_CAPACITY >= other.size())
            {
                helper::memcpy(stack.buffer, other.data(), other.size());
                stack.buffer[other.size()] = '\0';
                len = (other.size() << 1) | 0;
            }
            else
            {
                heap.cap = helper::goodsize(other.size());
                heap.buffer = helper::alloc(heap.cap + 1);
                helper::memcpy(heap.buffer, other.data(), other.size());
                heap.buffer[other.size()] = '\0';
                len = (other.size() << 1) | 1;
            }
        }

        return *this;
    }

    constexpr String& operator=(String const& other)
    {
        if (this == &other)
            return *this;

        return operator=(other.as_slice());
    }

    constexpr String& operator=(String&& other)
    {
        if (this == &other)
            return *this;

        // DESTRUCTOR
        if (len & 1)
        {
            delete[] heap.buffer;
        }

        // MOVE CONSTRUCTOR
        if (other.len & 1)
        {
            heap.cap = other.heap.cap;
            heap.buffer = other.heap.buffer;
            len = other.len;

            // make-default other
            other.len = 0;
            other.stack.buffer[0] = '\0';
        }
        else
        {
            if consteval 
            {
                helper::memcpy(stack.buffer, other.stack.buffer, other.size());
                stack.buffer[other.size()] = '\0';
            }
            else
            {
                helper::memcpy(stack.buffer, other.stack.buffer, 16);
            }
            len = other.len;

            // make-default other
            other.len = 0;
            other.stack.buffer[0] = '\0';
        }

        return *this;
    }

    static constexpr String with_capacity(ulong capacity)
    {
        if (capacity <= STACK_CAPACITY) 
        {
            return String();
        }
        else
        {
            String result;
            result.heap.cap = capacity;
            result.heap.buffer = helper::alloc(capacity + 1);
            result.heap.buffer[0] = '\0';
            result.len = 1;   // set HEAP flag
            return result;
        }
    }

    constexpr ~String()
    {
        if (len & 1) 
        {
            delete[] heap.buffer;
        }
    }

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////////// BASIC //////////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region

    constexpr ulong size() const
    {
        return len >> 1;
    }

    constexpr bool is_empty() const
    {
        return (len >> 1) == 0;
    }

    constexpr bool is_heap() const
    {
        return (len & 1);
    }

    constexpr ulong capacity() const
    {
        if (len & 1)
            return heap.cap;
        else
            return STACK_CAPACITY;
    }

    constexpr String clone() const
    {
        return String(*this);
    }

    constexpr char* begin()
    {
        if (len & 1)
            return heap.buffer;
        else
            return stack.buffer;
    }

    constexpr char const* begin() const
    {
        if (len & 1)
            return heap.buffer;
        else
            return stack.buffer;
    }

    constexpr char* end()
    {
        if (len & 1)
            return heap.buffer + size();
        else
            return stack.buffer + size();
    }

    constexpr char const* end() const
    {
        if (len & 1)
            return heap.buffer + size();
        else
            return stack.buffer + size();
    }

    constexpr char const* data() const
    {
        if (len & 1)
            return heap.buffer;
        else
            return stack.buffer;
    }

    constexpr Slice as_slice() const
    {
        return Slice { data(), size() };
    }

    constexpr Slice slice(ulong start, ulong max_len) const 
    {
        return as_slice().slice(start, max_len);
    }

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// ACCESS ELEMENTS ////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region

    constexpr char& operator[](ulong index) &
    {
        if (index >= size())
            __throw::STRING_Acessing_Outside_String();

        if (len & 1)
            return heap.buffer[index];
        else
            return stack.buffer[index];
    }

    constexpr char const& operator[](ulong index) const&
    {
        if (index >= size())
            __throw::STRING_Acessing_Outside_String();

        if (len & 1)
            return heap.buffer[index];
        else
            return stack.buffer[index];
    }

    constexpr char&& operator[](ulong index) &&
    {
        if (index >= size())
            __throw::STRING_Acessing_Outside_String();

        if (len & 1)
            return static_cast<char&&>(heap.buffer[index]);
        else
            return static_cast<char&&>(stack.buffer[index]);
    }

    constexpr char const&& operator[](ulong index) const&&
    {
        if (index >= size())
            __throw::STRING_Acessing_Outside_String();

        if (len & 1)
            return static_cast<char const&&>(heap.buffer[index]);
        else
            return static_cast<char const&&>(stack.buffer[index]);
    }

    constexpr Option<char&> front()
    {
        if (size() == 0)
            return { False{} };
        return { True{}, *begin() };
    }

    constexpr Option<char&> back()
    {
        if (size() == 0)
            return { False{} };
        return { True{}, *(begin() + size() - 1) };
    }

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: compare ////////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region

    constexpr bool operator==(String const& other) const
    {
        return Comparator<Slice, Ordering::Order>::compare(
            as_slice(), 
            other.as_slice()
        ) == Order::Equal;
    }

    constexpr bool operator!=(String const& other) const
    {
        return Comparator<Slice, Ordering::Order>::compare(
            as_slice(), 
            other.as_slice()
        ) != Order::Equal;
    }

    constexpr bool operator<(String const& other) const
    {
        return Comparator<Slice, Ordering::Order>::compare(
            as_slice(), 
            other.as_slice()
        ) == Order::Less;
    }

    constexpr bool operator>(String const& other) const
    {
        return Comparator<Slice, Ordering::Order>::compare(
            as_slice(), 
            other.as_slice()
        ) == Order::Greater;
    }

    constexpr bool operator<=(String const& other) const
    {
        return Comparator<Slice, Ordering::Order>::compare(
            as_slice(), 
            other.as_slice()
        ) != Order::Greater;
    }

    constexpr bool operator>=(String const& other) const
    {
        return Comparator<Slice, Ordering::Order>::compare(
            as_slice(), 
            other.as_slice()
        ) != Order::Less;
    }

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: operator+= /////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region

    constexpr String& operator+=(Slice other) &
    {
        write(other.data(), other.size()).map_err(
            [](FormatError) {
                __throw::STRING_Memory_Request_Failed();
            }
        ).unwrap();
        return *this;
    }

    constexpr String& operator+=(String const& other) &
    {
        write(other.data(), other.size()).map_err(
            [](FormatError) {
                __throw::STRING_Memory_Request_Failed();
            }
        ).unwrap();
        return *this;
    }

    constexpr String&& operator+=(Slice other) &&
    {
        return static_cast<String&&>(operator+=(other));
    }

    constexpr String&& operator+=(String const& other) &&
    {
        return static_cast<String&&>(operator+=(other));
    }

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: operator+ //////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region

    constexpr String operator+(Slice other) &&
    {
        return (static_cast<String&&>(*this).operator+=(other));
    }

    constexpr String operator+(String const& other) &&
    {
        return (static_cast<String&&>(*this).operator+=(other));
    }

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: write //////////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region

    constexpr Result<void, FormatError> write(char const* chars, ulong write_len)
    {
        if (write_len == 0)
            return { True{} };
        
        if (chars == nullptr && write_len != 0)
            __throw::STRING_Writing_From_Null_Character_Stream();

        String dummy;
        if consteval 
        {
            dummy = Slice { chars, write_len }.into_str();
            chars = dummy.data();
            write_len = dummy.size();
        }
        else
        {
            if (
                as_slice().overlaps(Slice { chars, write_len }) &&
                !as_slice().has_subslice(Slice { chars, write_len })
            )
            {
                String temp = Slice { chars, write_len }.into_str();
                return write(temp.data(), temp.size());
            }
        }
    
        ulong new_len = size() + write_len;
        
        if (len & 1)
        {
            if (new_len <= heap.cap)
            {
                helper::memcpy(heap.buffer + size(), chars, write_len);
                heap.buffer[new_len] = '\0';
                len = (new_len << 1) | 1;
            }
            else
            {
                ulong new_cap = helper::goodsize(new_len);
                char* new_buffer = new(std::nothrow) char[new_cap + 1];

                if (new_buffer == nullptr)
                    return { False{}, FormatError::MemoryRequestFailed };

                helper::memcpy(new_buffer, heap.buffer, size());
                helper::memcpy(new_buffer + size(), chars, write_len);
                new_buffer[new_len] = '\0';
                delete[] heap.buffer;
                heap.buffer = new_buffer;
                heap.cap = new_cap;
                len = (new_len << 1) | 1;  
            }
        }
        else
        {
            if (new_len <= STACK_CAPACITY)
            {
                helper::memcpy(stack.buffer + size(), chars, write_len);
                stack.buffer[new_len] = '\0';
                len = new_len << 1;
            }
            else
            {
                ulong new_cap = helper::goodsize(new_len);
                char* new_buffer = new(std::nothrow) char[new_cap + 1];

                if (new_buffer == nullptr)
                    return { False{}, FormatError::MemoryRequestFailed };
                
                helper::memcpy(new_buffer, stack.buffer, size());
                helper::memcpy(new_buffer + size(), chars, write_len);
                new_buffer[new_len] = '\0';
                heap.buffer = new_buffer;
                heap.cap = new_cap;
                len = (new_len << 1) | 1;
            }
        }

        return { True{} };
    }

    constexpr Result<void, FormatError> write(CharArr chars)
    {
        return write(chars, chars.size());
    }

    constexpr Result<void, FormatError> write(char ch, ulong count)
    {
        if (count == 0)
            return { True{} };

        ulong new_len = size() + count;

        if (len & 1)
        {
            if (new_len <= heap.cap)
            {
                helper::memset(heap.buffer + size(), ch, count);
                heap.buffer[new_len] = '\0';
                len = (new_len << 1) | 1;
            }
            else
            {
                ulong new_cap = helper::goodsize(new_len);
                char* new_buffer = new(std::nothrow) char[new_cap + 1];

                if (new_buffer == nullptr)
                    return { False{}, FormatError::MemoryRequestFailed };

                helper::memcpy(new_buffer, heap.buffer, size());
                helper::memset(new_buffer + size(), ch, count);
                new_buffer[new_len] = '\0';
                delete[] heap.buffer;
                heap.buffer = new_buffer;
                heap.cap = new_cap;
                len = (new_len << 1) | 1;  
            }
        }
        else
        {
            if (new_len <= STACK_CAPACITY)
            {
                helper::memset(stack.buffer + size(), ch, count);
                stack.buffer[new_len] = '\0';
                len = new_len << 1;
            }
            else
            {
                ulong new_cap = helper::goodsize(new_len);
                char* new_buffer = new(std::nothrow) char[new_cap + 1];

                if (new_buffer == nullptr)
                    return { False{}, FormatError::MemoryRequestFailed };
                
                helper::memcpy(new_buffer, stack.buffer, size());
                helper::memset(new_buffer + size(), ch, count);
                new_buffer[new_len] = '\0';
                heap.buffer = new_buffer;
                heap.cap = new_cap;
                len = (new_len << 1) | 1;
            }
        }

        return { True{} };
    }

private:

    template <typename Fn>
    constexpr Result<void, FormatError> write_helper(Fn&& spec_write, ulong size, FormatIR IR)
    {
        #define GUARD(result) do { auto res = result; if (res == False{}) return res; } while (false)

        struct helper
        {
            static void format_into(char* buffer, byte value)
            {
                buffer[2] = '0' + (value % 10);
                buffer[1] = '0' + (value / 10) % 10;
                buffer[0] = '0' + (value / 100) % 10;
            }
        };

        bool styled = false;
        if (IR.text_color == True{})
        {
            Color8bit color_vec = IR.text_color.unwrap();
            char buffer[] = "\e[38;2;000;000;000m";
            helper::format_into(buffer + 7, color_vec.red);
            helper::format_into(buffer + 11, color_vec.green);
            helper::format_into(buffer + 15, color_vec.blue);
            GUARD(write(buffer));
            styled = true;
        }

        if (IR.background_color == True{})
        {
            Color8bit color_vec = IR.background_color.unwrap();
            char buffer[] = "\e[48;2;000;000;000m";
            helper::format_into(buffer + 7, color_vec.red);
            helper::format_into(buffer + 11, color_vec.green);
            helper::format_into(buffer + 15, color_vec.blue);
            GUARD(write(buffer));
            styled = true;
        }

        if (IR.style != Style::Regular)
        {
            uint flags = +IR.style;
            char buffer[] = "\e[_;_;_;_;_m";
            int i = 2;
            if (flags & +Style::Bold)      { buffer[i] = '1'; i += 2; }
            if (flags & +Style::Dim)       { buffer[i] = '2'; i += 2; }
            if (flags & +Style::Italic)    { buffer[i] = '3'; i += 2; }
            if (flags & +Style::Underline) { buffer[i] = '4'; i += 2; }
            if (flags & +Style::Strike)    { buffer[i] = '9'; i += 2; }
            buffer[i - 1] = 'm';
            GUARD(write(buffer, i));
            styled = true;
        }

        if (IR.align.width > size)
        {
            ulong fill_len = IR.align.width - size;
            switch (IR.align.mode)
            {
                case Align::Mode::Left:
                {
                    GUARD(spec_write());
                    GUARD(write(IR.align.fill, fill_len));
                    break;
                }
                case Align::Mode::Right:
                {
                    GUARD(write(IR.align.fill, fill_len));
                    GUARD(spec_write());
                    break;
                }
                case Align::Mode::Center:
                {
                    GUARD(write(IR.align.fill, fill_len / 2));
                    GUARD(spec_write());
                    GUARD(write(IR.align.fill, fill_len - fill_len / 2));
                    break;
                }
            }
        }
        else
        {
            GUARD(spec_write());
        }

        if (styled)
            return write("\e[0m");
        else
            return { True{} };

        #undef GUARD
    }

public:

    constexpr Result<void, FormatError> write(char const* chars, ulong write_len, FormatIR IR)
    {
        return write_helper
        (
            [&]() { return write(chars, write_len); },
            write_len, IR
        );
    }

    constexpr Result<void, FormatError> write(CharArr chars, FormatIR IR)
    {
        return write_helper
        (
            [&]() { return write(chars, chars.size()); },
            chars.size(), IR
        );
    }

    constexpr Result<void, FormatError> write(char ch, ulong count, FormatIR IR)
    {
        return write_helper
        (
            [&]() { return write(ch, count); },
            count, IR
        );
    }

};

_GLOBAL constexpr mnc::String operator""_str(char const* _, mnc::ulong __) 
{
    return mnc::Slice { _, __ }.into_str();
} 
_MNC_BEGIN

template <>
class Comparator<String, Ordering::Order> {
public:

    static constexpr Order compare(String const& left, String const& right)
    {
        return Slice::compare(
            left.as_slice(), 
            right.as_slice()
        );
    }
};

template <>
class Formatter<String> {
private:

    Formatter<char const*> base;

public:

    consteval Formatter(): base() {}
    consteval Formatter(char const* params, ulong len): base(params, len) {}
    template <ulong N>
    consteval Formatter(char const (&params)[N]): base(params, N - 1) {}

    template <typename Buffer>
    requires (has_buffer_trait<Buffer>)
    constexpr Result<void, FormatError> format(Buffer& buffer, String const& value, FormatIR IR = FormatIR{}) const
    {
        return base.format(buffer, value.data(), value.size(), IR);
    }
};

template <typename... Ts>
requires ((has_formatter<Ts> || has_default_formatter<Ts>) && ...)
constexpr String format(FormatString<Ts...> Format, Ts const&... values)
{
    String buffer;
    Format.format(buffer, values...).unwrap();
    return buffer;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

constexpr String Slice::into_str() const
{
    return String(*this);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

_MNC_END

#undef _MNC_BEGIN
#undef _MNC_END
#undef _GLOBAL