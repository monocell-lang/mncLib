#pragma once
#include <cstdio>
#include <cstring>
#include "optional.h"
#include "compare.h"
#include "styling.h"
#include "utility.h"


#define _MNC_BEGIN namespace mnc {
#define _MNC_END }
#define _GLOBAL_EXPORT(Item) _MNC_END using mnc::Item; _MNC_BEGIN
#define _MNC ::mnc::
#define _LVALUE (*this).
#define _RVALUE mov(*this).

_MNC_BEGIN

//////////////////////////////////////////////////////////////////////
//////////////////////////// FORWARD DECLARE /////////////////////////
//////////////////////////////////////////////////////////////////////
#pragma region declare

class String;
class Slice;

constexpr Slice operator""_slice(char const*, ulong);
constexpr String operator""_str(char const*, ulong);

_GLOBAL_EXPORT(operator""_str)
_GLOBAL_EXPORT(operator""_slice)

//////////////////////////////////////////////////////////////////////
//////////////////////////// class: Slice ////////////////////////////
//////////////////////////////////////////////////////////////////////
#pragma region class: Slice

class Slice {
private:

    friend class String;
    friend class Comparator<Slice, Ordering::Order>;
    friend class Comparator<String, Ordering::Order>;

    struct __throw 
    {
        static void STRING_SLICE_Acessing_Outside_String()
        {
            std::fprintf(stderr, "[mnc::Slice] ;; ABORT WARNING ;; accessing outside string\n");
            std::fflush(stderr);
            std::abort();
        }
    };

    struct helper
    {
        static constexpr bool is_space(char ch)
        {
            return 
            ch == ' '  || ch == '\t' || ch == '\n' ||
            ch == '\r' || ch == '\v' || ch == '\f';
        }
    };

    //////////////////////////////////////////////////////////////////////
    //////////////////////////////// MAIN ////////////////////////////////
    //////////////////////////////////////////////////////////////////////
    
private:

    char const* _data;
    ulong len;

public:

    template <ulong N>
    constexpr Slice(char const (&data)[N]): _data(data), len(N - 1) {}

    friend constexpr Slice operator""_slice(char const* _, ulong __) 
    {
        return Slice { _, __ };
    } 

    constexpr Slice(): _data(""), len(0) {}
    constexpr Slice(Slice const&) = default;
    constexpr Slice(Slice&&) = default;
    constexpr Slice& operator=(Slice const&) = default;
    constexpr Slice& operator=(Slice&&) = default;
    constexpr ~Slice() = default;

    template <ulong N>
    static constexpr Slice from(char const (&data)[N]) { return Slice { data, N - 1 }; }
    
    static constexpr Slice from_cstr(char const* data, ulong len) { return Slice { data, len }; }

private:

    constexpr explicit Slice(char const* data, ulong len): _data(data), len(len) {}

public:

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////////// BASIC //////////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region basic funcs

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
    #pragma region operator[]

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
    ///////////////////////////// fn: trim ///////////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region trim

    constexpr Slice trim_head() const
    {
        ulong i = 0;
        while (i < len && helper::is_space(_data[i])) { ++i; }
        return Slice { _data + i, len - i };
    }

    constexpr Slice trim_tail() const
    {
        ulong i = len;
        while (i > 0 && helper::is_space(_data[i - 1])) { --i; }
        return Slice { _data, i };
    }

    constexpr Slice trim() const
    {
        return trim_head().trim_tail();
    }

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: compare ////////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region compare

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

    constexpr String to_str() const;

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: find ///////////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region find

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

template <>
class Comparator<Slice, Ordering::Order> {
public:

    static constexpr Order compare(Slice left, Slice right)
    {
        return Slice::compare(left, right);
    }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// class: String ///////////////////////////
//////////////////////////////////////////////////////////////////////
#pragma region class: String

class String {
private:

    friend class Slice;

    struct __throw
    {
        static void STRING_Acessing_Outside_String()
        {
            std::fprintf(stderr, "[mnc::String] ;; ABORT WARNING ;; accessing outside string\n");
            std::fflush(stderr);
            std::abort();
        }

        static void STRING_Memory_Request_Failed()
        {
            std::fprintf(stderr, "[mnc::String] ;; ABORT WARNING ;; memory request failed\n");
            std::fflush(stderr);
            std::abort();
        }

        static void STRING_Writing_From_Null_Character_Stream()
        {
            std::fprintf(stderr, "[mnc::String] ;; ABORT WARNING ;; writing from null character stream\n");
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

        static constexpr void memcpy(char* dest, char const* src, ulong src_size)
        {
            if consteval 
            {
                if (dest == src)
                    return;

                for (ulong i = 0; i < src_size; i++) 
                {
                    dest[i] = src[i];
                }
            }
            else
            {
                std::memcpy(dest, src, src_size);
            }
        }

        static constexpr void memmove(char* dest, char const* src, ulong src_size)
        {
            if consteval
            {
                if (dest == src)
                    return;

                if (dest > src)
                {
                    for (ulong i = 0; i < src_size; i++)
                    {
                        dest[src_size - 1 - i] = src[src_size - 1 - i];
                    }
                }
                else
                {
                    for (ulong i = 0; i < src_size; i++)
                    {
                        dest[i] = src[i];
                    }
                }
            }
            else
            {
                std::memmove(dest, src, src_size);
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
    #pragma region main

private:

    union {
        struct { char buffer[16]; } stack;
        struct { char* buffer; ulong cap; } heap;
    };
    
    ulong len; // bit 0 = HEAP FLAG | bits 1-63 = actual length
    static constexpr ulong STACK_CAPACITY = 15;

public:

    friend constexpr String operator""_str(char const* _, ulong __) 
    {
        return Slice { _, __ }.to_str();
    }

    constexpr String()
    {
        len = 0;
        stack.buffer[0] = '\0';
    }

    constexpr String(String const& other) = delete;

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
        String storage;
        if consteval
        {
            storage = other.to_str();
            other = storage.as_slice();
        }
        else
        {
            if (as_slice().overlaps(other))
            {
                storage = other.to_str();
                other = storage.as_slice();
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

    constexpr String& operator=(String const& other) = delete;

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

    template <ulong N>
    static constexpr String from(char const(&data)[N])
    {
        return Slice { data, N - 1 }.to_str();
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
    #pragma region basic funcs

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
        return as_slice().to_str();
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

    constexpr operator Slice() const
    {
        return as_slice();
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
    ///////////////////////////// fn: operator[] /////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region operator[]

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

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: back ///////////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region back

    constexpr Option<char&> back()
    {
        if (size() == 0)
            return { False{} };

        return { True{}, *(begin() + size() - 1) };
    }

    constexpr Option<char const&> back() const
    {
        if (size() == 0)
            return { False{} };

        return { True{}, *(begin() + size() - 1) };
    }

    constexpr String& push_back(char ch) &
    {
        char arr[] = { ch, '\0' };
        return _LVALUE append(Slice::from(arr));
    }

    constexpr String&& push_back(char ch) &&
    {
        char arr[] = { ch, '\0'  };
        return _RVALUE append(Slice::from(arr));
    }

    constexpr Option<char> pop_back()
    {
        if (size() == 0)
            return { False{} };

        char result = *(data() + size() - 1);
        erase(size() - 1, 1);
        return { True{}, result };
    }

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: front //////////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region front

    constexpr Option<char&> front()
    {
        if (size() == 0)
            return { False{} };

        return { True{}, *begin() };
    }

    constexpr Option<char const&> front() const
    {
        if (size() == 0)
            return { False{} };
        return { True{}, *begin() };
    }

    constexpr String& push_front(char ch) &
    {
        char arr[] = { ch, '\0'  };
        return _LVALUE insert(0, Slice::from(arr));
    }

    constexpr String&& push_front(char ch) &&
    {
        char arr[] = { ch, '\0'  };
        return _RVALUE insert(0, Slice::from(arr));
    }

    constexpr Option<char> pop_front()
    {
        if (size() == 0)
            return { False{} };

        char result = *data();
        erase(0, 1);
        return { True{}, result };
    }

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: append /////////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region append

    constexpr String& append(Slice other) &
    {
        write(
            other.data(), 
            other.size()
        )
        .map_err(
            [](FormatError) {
                __throw::STRING_Memory_Request_Failed();
            }
        )
        .unwrap();

        return *this;
    }

    constexpr String&& append(Slice other) &&
    {
        return static_cast<String&&>(_LVALUE append(other));
    }

    constexpr String& operator+=(Slice other) &
    {
        return _LVALUE append(other);
    }

    constexpr String&& operator+=(Slice other) &&
    {
        return _RVALUE append(other);
    }

    constexpr String operator+(Slice other) &&
    {
        return String(_MNC mov(*this)).append(other);
    }

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: insert /////////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region insert

    constexpr String& insert(ulong at, Slice other) &
    {
        if (other.size() == 0)
            return *this;

        if (at >= size())
            return append(other);

        String storage;
        if consteval
        {
            storage = other.to_str();
            other = storage.as_slice();
        }
        else
        {
            bool cond1 = slice(0, at).overlaps(other);
            bool cond2 = !slice(0, at).has_subslice(other);
            if (cond1 && cond2)
            {
                storage = other.to_str();
                other = storage.as_slice();
            }
        }

        ulong new_len = size() + other.size();

        if (len & 1)
        {
            if (new_len <= heap.cap)
            {
                helper::memmove(heap.buffer + at + other.size(), heap.buffer + at, size() - at);
                helper::memcpy(heap.buffer + at, other.data(), other.size());
                heap.buffer[new_len] = '\0';
                len = (new_len << 1) | 1;
            }
            else
            {
                ulong new_cap = helper::goodsize(new_len);
                char* new_buffer = helper::alloc(new_cap + 1);
                helper::memcpy(new_buffer, heap.buffer, at);
                helper::memcpy(new_buffer + at, other.data(), other.size());
                helper::memcpy(new_buffer + at + other.size(), heap.buffer + at, size() - at);
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
                helper::memmove(stack.buffer + at + other.size(), stack.buffer + at, size() - at);
                helper::memcpy(stack.buffer + at, other.data(), other.size());
                stack.buffer[new_len] = '\0';
                len = (new_len << 1) | 0;
            }
            else
            {
                ulong new_cap = helper::goodsize(new_len);
                char* new_buffer = helper::alloc(new_cap + 1);
                helper::memcpy(new_buffer, stack.buffer, at);
                helper::memcpy(new_buffer + at, other.data(), other.size());
                helper::memcpy(new_buffer + at + other.size(), stack.buffer + at, size() - at);
                new_buffer[new_len] = '\0';

                delete[] heap.buffer;
                heap.buffer = new_buffer;
                heap.cap = new_cap;
                len = (new_len << 1) | 1;
            }
        }

        return *this;
    }

    constexpr String&& insert(ulong at, Slice other) &&
    {
        return static_cast<String&&>(_LVALUE insert(at, other));
    }

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: erase //////////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region erase

    constexpr String& erase(ulong at, ulong max_range) &
    {
        if (at >= size() || max_range == 0)
            return *this;
        
        if (at + max_range >= size())
        {
            if (len & 1)
            {
                heap.buffer[at] = '\0';
                len = (at << 1) | 1;
            }
            else
            {
                stack.buffer[at] = '\0';
                len = (at << 1) | 0;
            }
        }
        else
        {
            ulong end = at + max_range;
            ulong new_len = size() - max_range;

            if (len & 1)
            {
                helper::memmove(heap.buffer + at, heap.buffer + end, size() - end);
                heap.buffer[new_len] = '\0';
                len = (new_len << 1) | 1;
            }
            else
            {
                helper::memmove(stack.buffer + at, stack.buffer + end, size() - end);
                stack.buffer[new_len] = '\0';
                len = (new_len << 1) | 0;
            }
        }

        return *this;
    }

    constexpr String&& erase(ulong at, ulong max_range) &&
    {
        return static_cast<String&&>(_LVALUE erase(at, max_range));
    }

    constexpr String& clear() &
    {
        return _LVALUE erase(0, size());
    }

    constexpr String&& clear() &&
    {
        return _RVALUE erase(0, size());
    }

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: replace /////////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region replace

    constexpr String& replace(ulong at, ulong max_range, Slice other) &
    {
        if (at >= size())
            return append(other);

        String storage;
        if consteval
        {
            storage = other.to_str();
            other = storage.as_slice();
        }
        else
        {
            bool cond1 = slice(0, at).overlaps(other);
            bool cond2 = !slice(0, at).has_subslice(other);
            if (cond1 && cond2)
            {
                storage = other.to_str();
                other = storage.as_slice();
            }
        }

        ulong end = at + max_range;
        ulong new_len = size() + other.size() - max_range;

        if (len & 1)
        {
            if (new_len <= heap.cap)
            {
                helper::memmove(heap.buffer + at + other.size(), heap.buffer + end, size() - end);
                helper::memcpy(heap.buffer + at, other.data(), other.size());
                heap.buffer[new_len] = '\0';
                len = (new_len << 1) | 1;
            }
            else
            {
                ulong new_cap = helper::goodsize(new_len);
                char* new_buffer = helper::alloc(new_cap + 1);
                helper::memcpy(new_buffer, heap.buffer, at);
                helper::memcpy(new_buffer + at, other.data(), other.size());
                helper::memcpy(new_buffer + at + other.size(), heap.buffer + end, size() - end);
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
                helper::memmove(stack.buffer + at + other.size(), stack.buffer + end, size() - end);
                helper::memcpy(stack.buffer + at, other.data(), other.size());
                stack.buffer[new_len] = '\0';
                len = (new_len << 1) | 0;
            }
            else
            {
                ulong new_cap = helper::goodsize(new_len);
                char* new_buffer = helper::alloc(new_cap + 1);
                helper::memcpy(new_buffer, stack.buffer, at);
                helper::memcpy(new_buffer + at, other.data(), other.size());
                helper::memcpy(new_buffer + at + other.size(), stack.buffer + end, size() - end);
                new_buffer[new_len] = '\0';
                
                delete[] heap.buffer;
                heap.buffer = new_buffer;
                heap.cap = new_cap;
                len = (new_len << 1) | 1;
            }
        }
        
        return *this;
    }

    constexpr String&& replace(ulong at, ulong max_range, Slice other) &&
    {
        return static_cast<String&&>(_LVALUE replace(at, max_range, other));
    }

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: trim ///////////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region trim

    constexpr String& trim_head() &
    {
        ulong i = 0;
        ulong size = this->size();
        char const* data = this->data();
        while (i < size && Slice::helper::is_space(data[i])) { ++i; }
        return _LVALUE erase(0, i);
    }

    constexpr String&& trim_head() &&
    {
        return static_cast<String&&>(_LVALUE trim_head());
    }

    constexpr String& trim_tail() &
    {
        ulong i = size();
        ulong size = this->size();
        char const* data = this->data();
        while (i > 0 && Slice::helper::is_space(data[i - 1])) { --i; }
        return _LVALUE erase(i, size - i);
    }

    constexpr String&& trim_tail() &&
    {
        return static_cast<String&&>(_LVALUE trim_tail());
    }

    constexpr String& trim() &
    {
        return _LVALUE trim_head().trim_tail();
    }

    constexpr String&& trim() &&
    {
        return _RVALUE trim_head().trim_tail();
    }

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: write //////////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region write

    constexpr Result<void, FormatError> write(char const* chars, ulong write_len)
    {
        if (write_len == 0)
            return { True{} };
        
        if (chars == nullptr && write_len != 0)
            __throw::STRING_Writing_From_Null_Character_Stream();

        String storage;
        if consteval
        {
            storage = Slice { chars, write_len }.to_str();
            chars = storage.data();
            write_len = storage.size();
        }
        else
        {
            bool cond1 = as_slice().overlaps(Slice { chars, write_len });
            bool cond2 = !as_slice().has_subslice(Slice { chars, write_len });
            if (cond1 && cond2)
            {
                storage = Slice { chars, write_len }.to_str();
                chars = storage.data();
                write_len = storage.size();
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

    constexpr Result<void, FormatError> write(char const* chars, ulong write_len, FormatIR IR)
    {
        return IR.write_into(*this, chars, write_len);
    }

    constexpr Result<void, FormatError> write(char ch, ulong count, FormatIR IR)
    {
        return IR.write_into(*this, ch, count);
    }

    constexpr Result<void, FormatError> write(Slice arr)
    {
        return write(arr.data(), arr.size());
    }

    constexpr Result<void, FormatError> write(Slice arr, FormatIR IR)
    {
        return IR.write_into(*this, arr.data(), arr.size());
    }

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: compare ////////////////////////////
    //////////////////////////////////////////////////////////////////////
    #pragma region compare

    constexpr bool operator==(Slice other) const
    {
        return Slice::compare(as_slice(), other) == Order::Equal;
    }

    constexpr bool operator!=(Slice other) const
    {
        return Slice::compare(as_slice(), other) != Order::Equal;
    }

    constexpr bool operator<(Slice other) const
    {
        return Slice::compare(as_slice(), other) == Order::Less;
    }

    constexpr bool operator>(Slice other) const
    {
        return Slice::compare(as_slice(), other) == Order::Greater;
    }

    constexpr bool operator<=(Slice other) const
    {
        return Slice::compare(as_slice(), other) != Order::Greater;
    }

    constexpr bool operator>=(Slice other) const
    {
        return Slice::compare(as_slice(), other) != Order::Less;
    }
};

template <>
class Comparator<String, Ordering::Order> {
public:

    static constexpr Order compare(String const& left, String const& right)
    {
        return Slice::compare(left.as_slice(), right.as_slice());
    }
};

constexpr String Slice::to_str() const
{
    String result;
    if (size() <= String::STACK_CAPACITY) // STACK
    {
        String::helper::memcpy(result.stack.buffer, data(), size());
        result.stack.buffer[size()] = '\0';
        result.len = (size() << 1) | 0;
    }
    else // HEAP
    {
        result.heap.cap = String::helper::goodsize(size());
        result.heap.buffer = String::helper::alloc(result.heap.cap + 1);
        String::helper::memcpy(result.heap.buffer, data(), size());
        result.heap.buffer[size()] = '\0';
        result.len = (size() << 1) | 1;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

_MNC_END

#undef _MNC_BEGIN
#undef _MNC_END
#undef _GLOBAL_EXPORT
#undef _MNC
#undef _LVALUE
#undef _RVALUE