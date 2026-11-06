#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <charconv>
#include "type_traits.h"
#include "optional.h"
#include "styling.h"
#include "string.h"
#include "utility.h"


#define _MNC_BEGIN namespace mnc {
#define _MNC_END }

_MNC_BEGIN

//////////////////////////////////////////////////////////////////////
////////////////////// class: FormatBuffer ///////////////////////////
//////////////////////////////////////////////////////////////////////
#pragma region FormatBuffer

template <typename T>
concept Is_Raw_Buffer = requires (T& buffer, char const* chars, ulong size, char ch, CharArray arr) {
    { buffer.write(chars, size) } -> same_as<Result<void, FormatError>>;
    { buffer.write(ch, size) } -> same_as<Result<void, FormatError>>;
    { buffer.write(arr) } -> same_as<Result<void, FormatError>>;
    { buffer.data() } -> same_as<char const*>;
    { buffer.size() } -> same_as<ulong>;
};

template <typename T>
concept Is_Buffer = requires (T& buffer, char const* chars, ulong size, char ch, CharArray arr, FormatIR IR) {
    { buffer.write(chars, size, IR) } -> same_as<Result<void, FormatError>>;
    { buffer.write(ch, size, IR) } -> same_as<Result<void, FormatError>>;
    { buffer.write(arr, IR) } -> same_as<Result<void, FormatError>>;
    { buffer.data() } -> same_as<char const*>;
    { buffer.size() } -> same_as<ulong>;
};

class FormatBuffer {
private:

    static constexpr ulong STACK_BUFFER_SIZE = 512;
    static constexpr bool ENABLE_LOGGING = false;

private:

    struct { char buffer[STACK_BUFFER_SIZE]; } stack;
    struct { char* buffer; ulong cap; } heap;
    ulong len; // bit 0 = HEAP FLAG | bit 0-63 = byte written

public:

    FormatBuffer() { len = 0; stack.buffer[0] = '\0'; }
    FormatBuffer(FormatBuffer const& other) = delete;
    FormatBuffer(FormatBuffer&& other): len(other.len)
    {
        if (other.len & 1)
        {
            heap = other.heap;
        }
        else
        {
            std::memcpy(stack.buffer, other.stack.buffer, other.len >> 1);
            stack.buffer[other.len >> 1] = '\0';
        }

        other.stack.buffer[0] = '\0';
        other.len = 0;
    }

    FormatBuffer& operator=(FormatBuffer const& other) = delete;
    FormatBuffer& operator=(FormatBuffer&& other)
    {
        if (this == &other)
            return *this;
        
        /// DESTRUCTOR
        {
            if (len & 1)
            {
                std::free(heap.buffer);
                //std::printf("[mnc::FormatBuffer] Log: destruction. Heap buffer freed\n");
            }
            else
            {
                //std::printf("[mnc::FormatBuffer] Log: destruction. No prior heap allocation\n");
            }
        }

        /// MOVE CONSTRUCTOR
        len = other.len;
        {
            if (other.len & 1)
            {
                heap = other.heap;
            }
            else
            {
                std::memcpy(stack.buffer, other.stack.buffer, other.len >> 1);
                stack.buffer[other.len >> 1] = '\0';
            }

            other.stack.buffer[0] = '\0';
            other.len = 0;
        }

        return *this;
    }

    ~FormatBuffer()
    {
        if (len & 1)
        {
            std::free(heap.buffer);
            if constexpr (ENABLE_LOGGING) std::printf("[mnc::FormatBuffer] Log: destruction. Heap buffer freed\n");
        }
        else
        {
            if constexpr (ENABLE_LOGGING) std::printf("[mnc::FormatBuffer] Log: destruction. No prior heap allocation\n");
        }
    }

    Result<void, FormatError> write(char const* chars, ulong write_len)
    {
        if (chars == nullptr)
        {
            std::fprintf(stderr, "[mnc::FormatBuffer] ;; ABORT WARNING ;; character stream is null\n");
            std::fflush(stderr);
            std::abort();
        }

        if (write_len == 0)
            return { True{} };

        const ulong is_heap = len & 1;
        const ulong current_len = len >> 1;
        const ulong current_cap = (is_heap? heap.cap: STACK_BUFFER_SIZE - 1);
        const ulong new_len = current_len + write_len;

        struct range {
        public:

            char const* begin;
            char const* end;

        public:

            static bool overlap(range r1, range r2)
            {
                if (r1.begin == r1.end || r2.begin == r2.end)
                    return false;
                
                return r1.begin < r2.end && r2.begin < r1.end;
            }
        };

        if (!is_heap)
        {
            if (range::overlap({ heap.buffer, heap.buffer + size() }, { chars, chars + write_len }))
            {
                std::fprintf(stderr, "[mnc::FormatBuffer] ;; ABORT WARNING ;; reading and writing buffer overlap\n");
                std::fflush(stderr);
                std::abort();
            }

            if (new_len <= current_cap)
            {
                std::memcpy(stack.buffer + current_len, chars, write_len);
                stack.buffer[new_len] = '\0';
                len = new_len << 1;
            }
            else
            {
                if constexpr (ENABLE_LOGGING) std::printf("[mnc::FormatBuffer] Log: stack - heap transfer\n");
                
                ulong new_cap = 3 * new_len / 2 + 1;
                char* new_buffer = (char*)std::malloc(new_cap + 1);

                if (new_buffer == nullptr)
                    return { False{}, FormatError::MemoryRequestFailed };
                
                std::memcpy(new_buffer, stack.buffer, current_len);
                std::memcpy(new_buffer + current_len, chars, write_len);
                new_buffer[new_len] = '\0';
                heap.buffer = new_buffer;
                heap.cap = new_cap;
                len = (new_len << 1) | 1;
            }
        }
        else
        {
            if (range::overlap({ stack.buffer, stack.buffer + size() }, { chars, chars + write_len }))
            {
                std::fprintf(stderr, "[mnc::FormatBuffer] ;; ABORT WARNING ;; reading and writing buffer overlap\n");
                std::fflush(stderr);
                std::abort();
            }

            if (new_len <= current_cap)
            {
                std::memcpy(heap.buffer + current_len, chars, write_len);
                heap.buffer[new_len] = '\0';
                len = (new_len << 1) | 1;
            }
            else
            {
                if constexpr (ENABLE_LOGGING) std::printf("[mnc::FormatBuffer] Log: heap - heap transfer\n");

                ulong new_cap = 3 * new_len / 2 + 1;
                char* new_buffer = (char*)std::malloc(new_cap + 1);

                if (new_buffer == nullptr)
                    return { False{}, FormatError::MemoryRequestFailed };

                std::memcpy(new_buffer, heap.buffer, current_len);
                std::memcpy(new_buffer + current_len, chars, write_len);
                new_buffer[new_len] = '\0';
                std::free(heap.buffer);
                heap.buffer = new_buffer;
                heap.cap = new_cap;
                len = (new_len << 1) | 1;  
            }
        }

        if constexpr (ENABLE_LOGGING) std::printf("[mnc::FormatBuffer] Log: write successfully. Buffer length = %lld\n", new_len);
        
        return { True{} };
    }

    Result<void, FormatError> write(char ch, ulong count)
    {
        if (count == 0)
            return { True{} };

        const ulong is_heap = len & 1;
        const ulong current_len = len >> 1;
        const ulong current_cap = (is_heap? heap.cap: STACK_BUFFER_SIZE - 1);
        const ulong new_len = current_len + count;

        if (!is_heap)
        {
            if (new_len <= current_cap)
            {
                //std::memcpy(stack.buffer + current_len, chars, write_len);
                std::memset(stack.buffer + current_len, ch, count);
                stack.buffer[new_len] = '\0';
                len = new_len << 1;
            }
            else
            {
                if constexpr (ENABLE_LOGGING) std::printf("[mnc::FormatBuffer] Log: stack - heap transfer\n");
                
                ulong new_cap = 3 * new_len / 2 + 1;
                char* new_buffer = (char*)std::malloc(new_cap + 1);

                if (new_buffer == nullptr)
                    return { False{}, FormatError::MemoryRequestFailed };
                
                std::memcpy(new_buffer, stack.buffer, current_len);
                //std::memcpy(new_buffer + current_len, chars, write_len);
                std::memset(new_buffer + current_len, ch, count);
                new_buffer[new_len] = '\0';
                heap.buffer = new_buffer;
                heap.cap = new_cap;
                len = (new_len << 1) | 1;
            }
        }
        else
        {
            if (new_len <= current_cap)
            {
                //std::memcpy(heap.buffer + current_len, chars, write_len);
                std::memset(heap.buffer + current_len, ch, count);
                heap.buffer[new_len] = '\0';
                len = (new_len << 1) | 1;
            }
            else
            {
                if constexpr (ENABLE_LOGGING) std::printf("[mnc::FormatBuffer] Log: heap - heap transfer\n");

                ulong new_cap = 3 * new_len / 2 + 1;
                char* new_buffer = (char*)std::malloc(new_cap + 1);

                if (new_buffer == nullptr)
                    return { False{}, FormatError::MemoryRequestFailed };

                std::memcpy(new_buffer, heap.buffer, current_len);
                //std::memcpy(new_buffer + current_len, chars, write_len);
                std::memset(new_buffer + current_len, ch, count);
                new_buffer[new_len] = '\0';
                std::free(heap.buffer);
                heap.buffer = new_buffer;
                heap.cap = new_cap;
                len = (new_len << 1) | 1;  
            }
        }

        if constexpr (ENABLE_LOGGING) std::printf("[mnc::FormatBuffer] Log: write successfully. Buffer length = %lld\n", new_len);
        
        return { True{} };
    }

    Result<void, FormatError> write(char const* chars, ulong write_len, FormatIR IR)
    {
        return IR.write_into(*this, chars, write_len);
    }

    Result<void, FormatError> write(char ch, ulong count, FormatIR IR)
    {
        return IR.write_into(*this, ch, count);
    }

    Result<void, FormatError> write(CharArray arr)
    {
        return write(arr.data(), arr.size());
    }

    Result<void, FormatError> write(CharArray arr, FormatIR IR)
    {
        return IR.write_into(*this, arr.data(), arr.size());
    }

    ulong size() const
    {
        return len >> 1;
    }

    bool is_empty() const
    {
        return (len >> 1) == 0;
    }

    char const* data() const
    {
        const ulong is_heap = len & 1;

        if (!is_heap)
            return stack.buffer;
        else
            return heap.buffer;
    }
};

#pragma endregion
//////////////////////////////////////////////////////////////////////
////////////////////// class: Formatter<T> ///////////////////////////
//////////////////////////////////////////////////////////////////////
#pragma region Formatter<T>

template <typename T>
class Formatter;

template <typename T>
concept Has_Formatter = requires (const Formatter<T> formatter, char const* params, ulong len, FormatBuffer& buffer, T const& value, FormatIR IR) {
    Formatter<T>(params, len);
    { formatter.format(buffer, value, IR) } -> same_as<Result<void, FormatError>>;
    requires (is_trivially_copy_able<Formatter<T>>);
    requires (is_trivially_move_able<Formatter<T>>);
    requires (is_trivially_destructible<Formatter<T>>);
};

template <typename T>
concept Has_Default_Formatter = requires (const Formatter<T> formatter, FormatBuffer& buffer, T const& value, FormatIR IR) {
    Formatter<T>();
    { formatter.format(buffer, value, IR) } -> same_as<Result<void, FormatError>>;
    requires (is_trivially_copy_able<Formatter<T>>);
    requires (is_trivially_move_able<Formatter<T>>);
    requires (is_trivially_destructible<Formatter<T>>);
};


// Syntax: (empty) | .normal | .debug
template <typename T>
requires (
    (!is_const<T> && !is_volatile<T> && 
    is_pointer<T> && same_type<remove_const<remove_pointer<T>>, char>) ||
    (is_array<T> && same_type<remove_const<remove_array<T>>, char>) ||
    same_type<T, CharArray>
)
class Formatter<T> {
private:

    struct helper
    {
        template <ulong N>
        static constexpr bool same_str(char const* s, ulong len, char const(&other)[N])
        {
            if (len != N - 1)
                return false;

            for (ulong i = 0; i < len; i++)
            {
                if (s[i] != other[i])
                    return false;
            }

            return true;
        }

        static constexpr bool is_empty_str(char const* s, ulong len)
        {
            for (ulong i = 0; i < len; i++)
            {
                if (s[i] != ' ')
                    return false;
            }
            return true;
        }
    };


private:

    struct __throw
    {
        static void FORMATTER_CSTR_Invalid_Format_Parameter_Syntax() {}
    };

    enum class Mode: byte {
        Normal,
        Debug
    };

private:

    Mode mode;

public:

    consteval Formatter(): mode(Mode::Normal) {}

    consteval Formatter(char const* params, ulong len): Formatter()
    {
        // VALID PARAMETERS:
        // .normal .debug

        if (helper::is_empty_str(params, len))
            return;

        char const* start = params;

        // find (.)
        {
            while (start < params + len)
            {
                if (*start == ' ')
                {
                    start++;
                    continue;
                }
                    
                if (*start == '.')
                {
                    start++;
                    break;
                }

                __throw::FORMATTER_CSTR_Invalid_Format_Parameter_Syntax();
            }
        }

        // parse params
        {
            char const* sstart = start;
            ulong slen = 0;
            
            while (start < params + len)
            {
                if (*start == ' ')
                    break;
                else
                    slen++;
                start++;
            }

            if (slen == 0) // just .
                __throw::FORMATTER_CSTR_Invalid_Format_Parameter_Syntax();

            while (start < params + len)
            {
                if (*start != ' ')  // has trailling
                    __throw::FORMATTER_CSTR_Invalid_Format_Parameter_Syntax();
                start++;
            };

            if (helper::same_str(sstart, slen, "normal"))     mode = Mode::Normal;
            else if (helper::same_str(sstart, slen, "debug")) mode = Mode::Debug;
            else __throw::FORMATTER_CSTR_Invalid_Format_Parameter_Syntax();
        }
    }

    template <ulong N>
    consteval Formatter(char const (&params)[N]): Formatter(params, N - 1) {}

    // debug ignores IR
    template <typename Buffer>
    requires (Is_Buffer<Buffer>)
    constexpr Result<void, FormatError> format(Buffer& buffer, char const* value, ulong len, FormatIR IR = FormatIR {}) const
    {
        #define GUARD(result) do { auto res = result; if (res == False{}) return res; } while (false)

        switch (mode)
        {
            case Mode::Normal:
            {
                return buffer.write(value, len, IR);
            }

            case Mode::Debug:
            {                
                GUARD(buffer.write("\""));
                for (ulong i = 0; i < len;)
                {
                    FormatIR profile = { .text_color = { True{}, +Color::Cream } };

                    // 1. special chars
                    switch (value[i])
                    {
                        case '\n': GUARD(buffer.write("\\n", profile)); i++; continue;
                        case '\r': GUARD(buffer.write("\\r", profile)); i++; continue;
                        case '\t': GUARD(buffer.write("\\t", profile)); i++; continue;
                        case '\b': GUARD(buffer.write("\\b", profile)); i++; continue;
                        case '\a': GUARD(buffer.write("\\a", profile)); i++; continue;
                        case '\f': GUARD(buffer.write("\\f", profile)); i++; continue;
                        case '\v': GUARD(buffer.write("\\v", profile)); i++; continue;
                        case '\0': GUARD(buffer.write("\\0", profile)); i++; continue;
                        case '\e': GUARD(buffer.write("\\e", profile)); i++; continue;
                        case '\'': GUARD(buffer.write("\\\'", profile)); i++; continue;
                        case '\"': GUARD(buffer.write("\\\"", profile)); i++; continue;
                        case '\\': GUARD(buffer.write("\\\\", profile)); i++; continue;
                    }

                    // 2. printable chars
                    if ((char)32 <= value[i] && value[i] <= (char)126)
                    {
                        char const* start = &value[i];
                        ulong slen = 0;

                        while (start + slen < value + len)
                        {
                            char ch = start[slen];
                            if ((ch < 32 || 126 < ch) || ch == '\'' || ch == '\"' || ch == '\\')
                                break;
                            else
                                slen++;
                        }

                        GUARD(buffer.write(start, slen));
                        i += slen;
                        continue;
                    }
                    
                    // 3. non-printable chars
                    if (value[i] < 32 || 126 < value[i])
                    {
                        byte val = value[i];
                        char num[30];
                        int j = 30;

                        num[--j] = ')';
                        while (val > 0)
                        {
                            byte old = val;
                            val /= 10;
                            num[--j] = '0' + (old - val * 10);
                        }
                        num[--j] = '(';
                        num[--j] = '\\';

                        GUARD(buffer.write(num + j, 30 - j, profile));
                        i++;
                        continue;
                    }
                }
                return buffer.write("\"");
            }
        }

        #undef GUARD
    }

    // debug ignores IR
    template <typename Buffer>
    requires (Is_Buffer<Buffer>)
    constexpr Result<void, FormatError> format(Buffer& buffer, char const* value, FormatIR IR = FormatIR {}) const
    {
        return format(buffer, value, std::strlen(value), IR);
    }

    // debug ignores IR
    template <typename Buffer>
    requires (Is_Buffer<Buffer>)
    constexpr Result<void, FormatError> format(Buffer& buffer, CharArray value, FormatIR IR = FormatIR {}) const
    {
        return format(buffer, value, value.size(), IR);
    }
};

template <typename T>
requires (
    same_type<T, String> || 
    same_type<T, Slice>
)
class Formatter<T> {
private:

    Formatter<char const*> base;

public:

    consteval Formatter() {}
    consteval Formatter(char const* params, ulong len): base(params, len) {}
    template <ulong N>
    consteval Formatter(char const (&params)[N]): base(params, N - 1) {}

    template <typename Buffer>
    requires (Is_Buffer<Buffer>)
    constexpr Result<void, FormatError> format(Buffer& buffer, T const& value, FormatIR IR = FormatIR{}) const
    {
        return base.format(buffer, value.data(), value.size(), IR);
    }
};

// Syntax: (empty) | .dec | .bin | .hex | .Hex | .oct
template <typename T>
requires (
    same_type<T, signed char>        ||
    same_type<T, signed short>       ||
    same_type<T, signed int>         ||
    same_type<T, signed long>        ||
    same_type<T, signed long long>   ||
    same_type<T, unsigned char>      ||
    same_type<T, unsigned short>     ||
    same_type<T, unsigned int>       ||
    same_type<T, unsigned long>      ||
    same_type<T, unsigned long long>
)
class Formatter<T> {
private:

    template <bool is_signed>
    struct struct_Int {
        using type = signed long long;
    };

    template <>
    struct struct_Int<false> {
        using type = unsigned long long;
    };

    using Int = typename struct_Int<is_signed_int<T>>::type;

    template <typename Ty>
    struct make_unsigned {
        using type = Ty;
    };

    template <typename Ty>
    requires (same_type<Ty, signed short>)
    struct make_unsigned<Ty> {
        using type = unsigned short;
    };

    template <typename Ty>
    requires (same_type<Ty, signed int>)
    struct make_unsigned<Ty> {
        using type = unsigned int;
    };

    template <typename Ty>
    requires (same_type<Ty, signed long>)
    struct make_unsigned<Ty> {
        using type = unsigned long;
    };

    template <typename Ty>
    requires (same_type<Ty, signed long long>)
    struct make_unsigned<Ty> {
        using type = unsigned long long;
    };

    using Unsigned = typename make_unsigned<T>::type;

private:

    struct __throw
    {
        static void FORMATTER_INT_Invalid_Format_Parameter_Syntax() {}
        //static void FORMATTER_INT_Unknown_Integer_Format() {}
    };

    struct helper
    {
        template <ulong N>
        static constexpr bool same_str(char const* s, ulong len, char const(&other)[N])
        {
            if (len != N - 1)
                return false;

            for (ulong i = 0; i < len; i++)
            {
                if (s[i] != other[i])
                    return false;
            }

            return true;
        }

        static constexpr bool is_empty_str(char const* s, ulong len)
        {
            for (ulong i = 0; i < len; i++)
            {
                if (s[i] != ' ')
                    return false;
            }
            return true;
        }
    };

    enum class Mode: byte { 
        Decimal,
        SignedDecimal,
        Binary, 
        Hexadecimal, 
        CapHexadecimal, 
        Octal 
    };

private:

    Mode mode;

public:

    consteval Formatter(): mode(Mode::Decimal) {}

    consteval Formatter(char const* params, ulong len): Formatter()
    {
        // VALID PARAMETERS:
        // .dec .bin .hex .Hex .oct

        if (helper::is_empty_str(params, len))
            return;

        char const* start = params;

        // find (.)
        {
            while (start < params + len)
            {
                if (*start == ' ')
                {
                    start++;
                    continue;
                }
                    
                if (*start == '.')
                {
                    start++;
                    break;
                }

                __throw::FORMATTER_INT_Invalid_Format_Parameter_Syntax();
            }
        }

        // parse params
        {
            char const* sstart = start;
            ulong slen = 0;
            
            while (start < params + len)
            {
                if (*start == ' ')
                    break;
                else
                    slen++;
                start++;
            }

            if (slen == 0) // just .
                __throw::FORMATTER_INT_Invalid_Format_Parameter_Syntax();

            while (start < params + len)
            {
                if (*start != ' ')  // has trailling
                    __throw::FORMATTER_INT_Invalid_Format_Parameter_Syntax();
                start++;
            };

            if (helper::same_str(sstart, slen, "dec"))       mode = Mode::Decimal;
            else if (helper::same_str(sstart, slen, "+dec")) mode = Mode::SignedDecimal;
            else if (helper::same_str(sstart, slen, "bin"))  mode = Mode::Binary;
            else if (helper::same_str(sstart, slen, "hex"))  mode = Mode::Hexadecimal;
            else if (helper::same_str(sstart, slen, "Hex"))  mode = Mode::CapHexadecimal;
            else if (helper::same_str(sstart, slen, "oct"))  mode = Mode::Octal;
            else __throw::FORMATTER_INT_Invalid_Format_Parameter_Syntax();
        }
    }

    template <ulong N>
    consteval Formatter(char const (&params)[N]): Formatter(params, N - 1) {}

    template <typename Buffer>
    requires (Is_Buffer<Buffer>)
    constexpr Result<void, FormatError> format(Buffer& buffer, T value, FormatIR IR = FormatIR {}) const
    {
        switch (mode)
        {
            case Mode::Decimal:
            case Mode::SignedDecimal:
            {
                if (value == 0)
                    return buffer.write("0", IR);

                if constexpr (is_signed_int<T>)
                {
                    if (value == -9223372036854775807LL - 1LL) 
                        return buffer.write("-9223372036854775808", IR);
                }

                char buffer_temp[30];
                int i = 30;

                Int val = value;
                bool is_negative = false;
                if constexpr (is_signed_int<T>) if (val < 0) 
                {
                    is_negative = true;
                    val = -val;
                }
                while (val > 0)
                {
                    Int old = val;
                    val /= 10;
                    buffer_temp[--i] = '0' + (old - val * 10);
                }
                if constexpr (is_signed_int<T>) 
                {
                    if (is_negative)
                    {
                        buffer_temp[--i] = '-';
                    }
                    else if (mode == Mode::SignedDecimal)
                    {
                        buffer_temp[--i] = '+';
                    }
                }
                else
                {
                    if (mode == Mode::SignedDecimal)
                    {
                        buffer_temp[--i] = '+';
                    }
                }

                return buffer.write(buffer_temp + i, 30 - i, IR);
            }
            case Mode::Binary:
            {
                if (value == 0)
                    return buffer.write("0b0", IR);

                char buffer_temp[80];
                int i = 80;

                Unsigned val = value;
                while (val > 0)
                {
                    buffer_temp[--i] = '0' + (val & 0b1);
                    val >>= 1;
                }
                buffer_temp[--i] = 'b';
                buffer_temp[--i] = '0';

                return buffer.write(buffer_temp + i, 80 - i, IR);
            }
            case Mode::Hexadecimal:
            case Mode::CapHexadecimal:
            {
                if (value == 0)
                    return buffer.write("0x0", IR);

                static constexpr char hex[] = "0123456789abcdef";
                static constexpr char Hex[] = "0123456789ABCDEF";
                char const* const conv = (mode == Mode::Hexadecimal? hex: Hex);

                char buffer_temp[30];
                int i = 30;

                Unsigned val = value;
                while (val > 0)
                {
                    buffer_temp[--i] = conv[val & 0b1111];
                    val >>= 4;
                }
                buffer_temp[--i] = 'x';
                buffer_temp[--i] = '0';

                return buffer.write(buffer_temp + i, 30 - i, IR);
            }
            case Mode::Octal:
            {
                if (value == 0)
                    return buffer.write("0o0", IR);

                char buffer_temp[30];
                int i = 30;

                Unsigned val = value;
                while (val > 0)
                {
                    buffer_temp[--i] = '0' + (val & 0b111);
                    val >>= 3;
                }
                buffer_temp[--i] = 'o';
                buffer_temp[--i] = '0';

                return buffer.write(buffer_temp + i, 30 - i, IR);
            }
        }
    }
};


// syntax: .(empty | precision)(empty | g | f | e)
template <typename T>
requires (
    same_type<T, float>       ||
    same_type<T, double>      ||
    same_type<T, long double>
)
class Formatter<T> {
private:

    struct __throw
    {
        static void FORMATTER_FLOAT_Invalid_Format_Syntax() {}
        static void FORMATTER_FLOAT_Floating_Point_Precision_Too_Large() {}
        //static void FORMATTER_FLOAT_Floating_Point_Precision_Cannot_Be_Zero() {}
    };

    struct helper
    {
        template <ulong N>
        static constexpr bool same_str(char const* s, ulong len, char const(&other)[N])
        {
            if (len != N - 1)
                return false;

            for (ulong i = 0; i < len; i++)
            {
                if (s[i] != other[i])
                    return false;
            }

            return true;
        }

        static constexpr Option<byte> parse(char const* s, ulong len)
        {
            if (len == 0)
                return { False{} };

            byte value = 0;
            for (ulong i = 0; i < len; i++)
            {
                if (s[i] < '0' || '9' < s[i]) // non-digit chars
                    return { False{} };

                constexpr byte BYTE_MAX = 0xff;
                byte digit = s[i] - '0';

                // overflow check
                if (value > (BYTE_MAX - digit) / 10u)
                    return { False{} };

                value = value * 10 + digit;
            }
            
            return { True{}, value };
        }

        static constexpr bool is_empty_str(char const* s, ulong len)
        {
            for (ulong i = 0; i < len; i++)
            {
                if (s[i] != ' ')
                    return false;
            }
            return true;
        }
    };

    enum class Mode: byte {
        General,
        Fixed,
        Scientific
    };

    static constexpr byte PRECISION_UPPER_LIMIT = 50;

private:

    byte precision;
    Mode mode;

public:

    consteval Formatter(): precision(2), mode(Mode::General) {}

    consteval Formatter(char const* params, ulong len): Formatter()
    {
        // VALID PARAMETERS:
        // .g .f .e
        // .012
        // .012g .012f .012e

        if (helper::is_empty_str(params, len))
            return;

        char const* start = params;

        // find (.)
        {
            while (start < params + len)
            {
                if (*start == '.')
                {
                    start++;
                    break;
                }

                if (*start == ' ')
                {
                    start++;
                    continue;
                }
                    
                __throw::FORMATTER_FLOAT_Invalid_Format_Syntax();
            }
        }

        // parse
        {
            // let checkpoint = start

            // for numbers:
            char const* nstart = start;  // checkpoint
            ulong nlen = 0;

            // for chars:
            char const* cstart = start;  // checkpoint
            ulong clen = 0;

            enum class State { None, GetN, GetC } state = State::None;

            // only 3 cases:
            // None -> GetN -> GetC -> end  // nstart = checkpoint ; cstart = nstart + nlen
            // None -> GetN -> end          // nstart = checkpoint
            // None -> GetC -> end          // cstart = checkpoint

            while (start < params + len)
            {
                if (state == State::None)
                {
                    if ('0' <= *start && *start <= '9')
                    {
                        nlen++;
                        start++;
                        state = State::GetN;
                    }
                    else if (('a' <= *start && *start <= 'z') || ('A' <= *start && *start <= 'Z'))
                    {
                        clen++;
                        start++;
                        state = State::GetC;
                    }
                    else if (*start == ' ')
                    {
                        break;
                    }
                    else
                    {
                        __throw::FORMATTER_FLOAT_Invalid_Format_Syntax();
                    }
                }
                else if (state == State::GetN)
                {
                    if ('0' <= *start && *start <= '9')
                    {
                        nlen++;
                        start++;
                    }
                    else if (('a' <= *start && *start <= 'z') || ('A' <= *start && *start <= 'Z'))
                    {
                        cstart = start;
                        clen++;
                        start++;
                        state = State::GetC;
                    }
                    else if (*start == ' ')  // or end of string
                    {
                        break;
                    }
                    else
                    {
                        __throw::FORMATTER_FLOAT_Invalid_Format_Syntax();
                    }
                }
                else if (state == State::GetC)
                {
                    if (('a' <= *start && *start <= 'z') || ('A' <= *start && *start <= 'Z'))
                    {
                        clen++;
                        start++;
                    }
                    else if (*start == ' ')   // or end of string
                    {
                        break;
                    }
                    else
                    {
                        __throw::FORMATTER_FLOAT_Invalid_Format_Syntax();
                    }
                }
            }

            if (nlen == 0 && clen == 0) // just .
                __throw::FORMATTER_FLOAT_Invalid_Format_Syntax();

            while (start < params + len)
            {
                if (*start != ' ') // has trailling
                    __throw::FORMATTER_FLOAT_Invalid_Format_Syntax();
                start++;
            };

            if (nlen > 0)
            {
                precision = helper::parse(nstart, nlen).value_or(255);
                // False{} means overflow
            }

            if (clen > 0)
            {
                if (helper::same_str(cstart, clen, "g")) mode = Mode::General;
                else if (helper::same_str(cstart, clen, "f")) mode = Mode::Fixed;
                else if (helper::same_str(cstart, clen, "e")) mode = Mode::Scientific;
                else __throw::FORMATTER_FLOAT_Invalid_Format_Syntax();
            }
        }

        // check boundaries
        {
            if (precision > PRECISION_UPPER_LIMIT)
                __throw::FORMATTER_FLOAT_Floating_Point_Precision_Too_Large();
            //if (precision == 0)
                //__throw::FORMATTER_FLOAT_Floating_Point_Precision_Cannot_Be_Zero();
        }
    }

    template <ulong N>
    consteval Formatter(char const (&params)[N]): Formatter(params, N - 1) {}

    template <typename Buffer>
    requires (Is_Buffer<Buffer>)
    constexpr Result<void, FormatError> format(Buffer& buffer, T value, FormatIR IR = FormatIR {}) const
    {
        char buffer_temp[PRECISION_UPPER_LIMIT + 30];

        std::chars_format std_mode;
        switch (mode)
        {
            case Mode::General:     std_mode = std::chars_format::general;     break;
            case Mode::Fixed:       std_mode = std::chars_format::fixed;       break;
            case Mode::Scientific:  std_mode = std::chars_format::scientific;  break;
        }

        auto res = std::to_chars(
            buffer_temp, 
            buffer_temp + sizeof(buffer_temp), 
            value, 
            std_mode, 
            precision
        );

        if (res.ec == std::errc::value_too_large)  // no other errors
        {
            res = std::to_chars(
                buffer_temp, 
                buffer_temp + sizeof(buffer_temp), 
                value, 
                std::chars_format::scientific, 
                precision
            );
        }

        return buffer.write(buffer_temp, res.ptr - buffer_temp, IR);
    }
};

template <typename T>
requires (
    !is_const<T> && 
    !is_volatile<T> && 
    !same_type<remove_const<remove_pointer<T>>, char>
)
class Formatter<T*> {
public:

    template <typename Buffer>
    requires (Is_Buffer<Buffer>)
    constexpr Result<void, FormatError> format(Buffer& buffer, T const* value, FormatIR IR = FormatIR {}) const
    {
        if (value == nullptr)
            return buffer.write("0000000000000000", IR);

        static constexpr char hex[16] = 
        {
            '0','1','2','3',
            '4','5','6','7',
            '8','9','A','B',
            'C','D','E','F'
        };

        char buffer_temp[16];
        int i = 15;

        ulong val = (ulong)value;
        while (val)
        {
            buffer_temp[i--] = hex[val & 0xf];
            val >>= 4;
        }
        while (i >= 0)
        {
            buffer_temp[i--] = '0';
        }

        return buffer.write(buffer_temp, 16, IR);
    }
};

template <>
class Formatter<bool> {
public:

    template <typename Buffer>
    requires (Is_Buffer<Buffer>)
    constexpr Result<void, FormatError> format(Buffer& buffer, bool value, FormatIR IR = FormatIR {}) const
    {
        if (value)
            return buffer.write("true", IR);
        else
            return buffer.write("false", IR);
    }
};

template <>
class Formatter<char> {
public:

    template <typename Buffer>
    requires (Is_Buffer<Buffer>)
    constexpr Result<void, FormatError> format(Buffer& buffer, char value, FormatIR IR = FormatIR {}) const
    {
        return buffer.write(value, 1, IR);
    }
};

template <>
class Formatter<decltype(nullptr)> {
public:

    template <typename Buffer>
    requires (Is_Buffer<Buffer>)
    constexpr Result<void, FormatError> format(Buffer& buffer, decltype(nullptr), FormatIR IR = FormatIR {}) const
    {
        return buffer.write("0000000000000000", IR);
    }
};


template <typename T, ulong N>
requires (
    Has_Default_Formatter<remove_const<T>> &&
    !same_type<remove_const<T>, char>
)
class Formatter<T[N]> {
private:

    Formatter<T> fmt_elem;

public:

    consteval Formatter() {}

    template <typename Buffer>
    requires (Is_Buffer<Buffer>)
    constexpr Result<void, FormatError> format(Buffer& buffer, T const(&value)[N], FormatIR IR = FormatIR {}) const
    {
        #define GUARD(result) do { auto res = result; if (res == False{}) return res; } while (false)
        bool first = true;

        FormatBuffer temp;
        GUARD(temp.write("["));
        for (T const& elem: value)
        {
            if (!first) GUARD(temp.write(", "));
            GUARD(fmt_elem.format(temp, elem));
            first = false;
        }
        GUARD(temp.write("]"));

        return buffer.write(temp.data(), temp.size(), IR);
        #undef GUARD
    }
};

// Syntax: .[(Format For T) ; (Format For E)]
template <typename T, typename E>
requires (
    !is_const<T> && !is_volatile<T> && 
    !is_const<E> && !is_volatile<E> && 
    (Has_Formatter<T> || Has_Default_Formatter<T> || same_type<T, void>) &&
    (Has_Formatter<E> || Has_Default_Formatter<E> || same_type<E, void>)
)
class Formatter<Opt<T, E>> {
private:

    template <typename Val, typename Err>
    struct Data {
        Formatter<Val> fmt_val;
        Formatter<Err> fmt_err;
    };

    template <typename Val>
    struct Data<Val, void> {
        Formatter<Val> fmt_val;
    };

    template <typename Err>
    struct Data<void, Err> {
        Formatter<Err> fmt_err;
    };

private:

    struct __throw
    {
        static void FORMATTER_OPT_Invalid_Format_Parameter_Syntax() {}
        static void FORMATTER_OPT_Unbalanced_Square_Brackets() {}   
        static void FORMATTER_OPT_Expected_Semicolon_Between_Format_Arguments() {}  // no semicolon
        static void FORMATTER_OPT_Provided_More_Than_Two_Format_Arguments() {}  // too many semincolons
        static void FORMATTER_OPT_No_Format_Option_Available_For_True() {}
        static void FORMATTER_OPT_Argument_For_True_Is_Void() {}
        static void FORMATTER_OPT_No_Format_Option_Available_For_False() {}
        static void FORMATTER_OPT_Argument_For_False_Is_Void() {}
    };

    struct helper
    {
        static constexpr bool is_empty_str(char const* s, ulong len)
        {
            for (ulong i = 0; i < len; i++)
            {
                if (s[i] != ' ')
                    return false;
            }
            return true;
        }
    };

private:

    Data<T, E> data;

public:

    consteval Formatter(char const* params, ulong len)
    {
        // PARAMETERS:
        // .[ for True ; for False ]

        if (helper::is_empty_str(params, len))
            return;

        char const* start = params;
        

        // find (.)
        {
            bool dot = false;

            while (start < params + len)
            {
                if (*start == ' ')
                {
                    start++;
                    continue;
                }

                if (*start == '.')
                {
                    start++;
                    dot = true;
                    break;
                }

                __throw::FORMATTER_OPT_Invalid_Format_Parameter_Syntax();
            }

            if (dot && (start == params + len || *start == ' '))  // just . alone of end of string
                __throw::FORMATTER_OPT_Invalid_Format_Parameter_Syntax();

            if (dot && helper::is_empty_str(start, params + len - start))  // just .
                __throw::FORMATTER_OPT_Invalid_Format_Parameter_Syntax();
        }

        // check bracket ballance
        {
            char const* checkpoint = start;
            int open = 0;
            int close = 0;

            while (start < params + len)
            {
                if (*start == '[')
                    open++;
                else if (*start == ']')
                    close++;
                start++;
            }

            if (open != close)
                __throw::FORMATTER_OPT_Unbalanced_Square_Brackets();

            if (open == 0) // no []
                __throw::FORMATTER_OPT_Invalid_Format_Parameter_Syntax();

            start = checkpoint;
        }

        // check trailing after ]
        {
            char const* p = params + len - 1;
            while (p >= params)
            {
                if (*p == ' ')
                {
                    if (p == params)
                        break;
                    else
                        p--;
                }
                else if (*p == ']')
                    break;
                else
                    __throw::FORMATTER_OPT_Invalid_Format_Parameter_Syntax();
            }
        }

        // parse params
        {
            // for True
            char const* tstart = start;
            ulong tlen = 0;

            // for False
            char const* fstart = start;
            ulong flen = 0;
            
            enum class State { Start, ArgTrue, ArgFalse, Finish } state = State::Start;
            int open = 0;
            int close = 0;

            while (start < params + len)
            {
                if (state == State::Start)
                {
                    if (*start == '[')
                    {
                        open++;
                        state = State::ArgTrue;
                        tstart = start + 1;
                        start++;
                    }
                    else
                    {
                        __throw::FORMATTER_OPT_Invalid_Format_Parameter_Syntax();
                    }
                }
                else if (state == State::ArgTrue)
                {
                    if (*start == '[')
                    {
                        open++;
                        tlen++;
                        start++;
                    }
                    else if (*start == ']')
                    {
                        close++;
                        tlen++;
                        start++;
                    }
                    else if (*start == ';')
                    {
                        if (open == close + 1)
                        {
                            state = State::ArgFalse;
                            fstart = start + 1;
                            start++;
                        }
                        else
                        {
                            tlen++;
                            start++;
                        }
                    }
                    else
                    {
                        tlen++;
                        start++;
                    }
                }
                else if (state == State::ArgFalse)
                {
                    if (*start == '[')
                    {
                        open++;
                        flen++;
                        start++;
                    }
                    else if (*start == ']')
                    {
                        close++;
                        if (open == close)
                        {
                            start++;
                            state = State::Finish;
                            break;
                        }
                        else
                        {
                            flen++;
                            start++;
                        }
                    }
                    else if (*start == ';')
                    {
                        if (open == close + 1)
                        {
                            __throw::FORMATTER_OPT_Provided_More_Than_Two_Format_Arguments();
                        }
                        else
                        {
                            flen++;
                            start++;
                        }
                    }
                    else
                    {
                        flen++;
                        start++;
                    }
                }
            }

            if (state != State::Finish)
                __throw::FORMATTER_OPT_Expected_Semicolon_Between_Format_Arguments();


            if constexpr (!same_type<T, void> && Has_Formatter<T>)
            {
                data.fmt_val = Formatter<T>(tstart, tlen);
            }
            else if constexpr (!same_type<T, void> && Has_Default_Formatter<T>)
            {
                if (!helper::is_empty_str(fstart, flen))
                    __throw::FORMATTER_OPT_No_Format_Option_Available_For_True();
                data.fmt_val = Formatter<T>();
            }
            else if constexpr (same_type<T, void>)
            {
                if (!helper::is_empty_str(fstart, flen))
                    __throw::FORMATTER_OPT_Argument_For_True_Is_Void();
            }


            if constexpr (!same_type<E, void> && Has_Formatter<E>)
            {
                data.fmt_err = Formatter<E>(fstart, flen);
            }
            else if constexpr (!same_type<E, void> && Has_Default_Formatter<E>)
            {
                if (!helper::is_empty_str(fstart, flen))
                    __throw::FORMATTER_OPT_No_Format_Option_Available_For_False();
                data.fmt_err = Formatter<E>();
            }
            else if constexpr (same_type<E, void>)
            {
                if (!helper::is_empty_str(fstart, flen))
                    __throw::FORMATTER_OPT_Argument_For_False_Is_Void();
            }
        }
    }

    template <ulong N>
    consteval Formatter(char const(&params)[N]): Formatter(params, N - 1) {}

    template <typename Buffer>
    requires (Is_Buffer<Buffer>)
    constexpr Result<void, FormatError> format(Buffer& buffer, Opt<T, E> const& value, FormatIR IR = FormatIR {}) const
    {
        #define GUARD(result) do { auto res = result; if (res == False{}) return res; } while (false)

        if (value == True{})
        {
            FormatBuffer temp;
            GUARD(temp.write("True(value: "));
            if constexpr (!same_type<T, void>) {
                GUARD(Formatter<T>().format(temp, value.unwrap()));
            }
            GUARD(temp.write(")"));
            return buffer.write(temp.data(), temp.size(), IR);
        }
        else
        {
            FormatBuffer temp;
            GUARD(temp.write("False(error: "));
            if constexpr (!same_type<E, void>) {
                GUARD(Formatter<E>().format(temp, value.unwrap_err()));
            }
            GUARD(temp.write(")"));
            return buffer.write(temp.data(), temp.size(), IR);
        }

        #undef GUARD
    }
};

#pragma endregion
//////////////////////////////////////////////////////////////////////
////////////////////// class: TextFormatter //////////////////////////
//////////////////////////////////////////////////////////////////////
#pragma region TextFormatter<Ts...>

    // {}
    // {<30}
    // {_30_}
    // {<30_ @Carnelian $Green}
    // {_20* @(179, 27, 27) $(0, 255, 0)}
    // {>35. !ui @#FFCA03 $#002B56}

    // WHERE:
    // < > _ for align (left, right, center). Width and fill come after align (fill can be skipped)
    // ! for style
    // @ for text color
    // $ for background color
    // spaces dont matter

    // SPECIAL:
    // pass this to the first parameter of println: "<30_ @Carnelian $Green"_fmt
    // => applies this formating style to everything except format args

    // {@(179, 27, 27); .2}  => FormatIR::generate("@(179, 27, 27)"). Fomatter<float>(" .2")
    // {self; .dec}          => use the same formatting style as the output string. Formatter<int>(" .dec")
    // {self;   }            => use the same formatting style as the output string. Formatter<int>("   ")
    // {self}                => use the same formatting style as the output string. Formatter<int>("")
    // {; .dec}              => FormatIR::generate(""). Formatter<int>(" .dec")
    // {;}                   => FormatIR::generate(""). Formatter<int>("")

    // type formatters use the second param after ;
    // {... ; .hex}
    // {... ; .bin}
    // {... ; .debug}
    // {; .[.dec;]}  leaving first param empty is ok


/*
*   Update v2:
*   - Detatched self from string
*   - self is now passed as a seperate argument
*   - Format inheritance from self stays the same as v1
*
*   Example 1:
*     old: println("{self: <30* @green}x = {}", 1);
*     new: println("<30* @green"_fmt, "x = {}", 1);
*
*   Example 2:
*     old: println("{self: @navy $cream}name tag: {self}, ID: {self}", "Elen", "IC66985478PDA");
*     new: println("@navy $cream"_fmt, "name tag: {self}, ID: {self}", "Elen", "IC66985478PDA");
*
*   Update v3:
*   - Supports escape syntax for { and }
*   - After formatted, {{ becomes { and }} becomes }
*   - Escape is not allowed in format arguments
*   
*   Example:
*     println("workspace {{ theme: {}, folder: {} }}", theme, folder);
*
*   Update v4:
*   - self now make the argument a part of the format string
*       + Arguments specifying {self} do not inherit thr IR blindly but smartly blend into the format string
*       + The whole output text correctly aligns based on its visual width, treating {self} arguments as native parts
*       + Optimized ANSI emission to actually make {self} args blend into the string while keeping formatting fast
*   - Reduced overhead for { and } escape for long continuous sequences
*
*   Example 1:
*     println("@green"_fmt, "x = {self}, y = {}", 3, 4);  
*     // prints green "x = 3, y = " and default "4"
*
*   Example 2:
*     println("_30*@green"_fmt, "x = {self}, y = {<5-}", 3, 4);  
*     // prints green "*******x = 3, y = ", default "4----" and green "*******""
*
*/

template <typename... Ts>
requires ((Has_Formatter<Ts> || Has_Default_Formatter<Ts>) && ...)
class TextFormatter {
private:

    struct Array {
    public:

        int data[sizeof...(Ts)];

    public:

        constexpr int& operator[](int index) { return data[index]; }
        constexpr int const& operator[](int index) const { return data[index]; }
    };

    template <int _Index, typename T>
    struct TupleCell { T data; };

    template <typename index_sequence>
    struct Tuple;

    template <int... _Index>
    struct Tuple<index_sequence<_Index...>>: public TupleCell<_Index, Formatter<Ts>>... {};

    template <int _Index>
    using Cell = TupleCell<_Index, type_at<_Index, Formatter<Ts>...>>;

private:

    struct __throw
    {
        static void TEXTFORMATTER_Only_Self_Is_Allowed_When_Inheriting_Text_Format() {}
        static void TEXTFORMATTER_Expecting_Format_Arguments_Or_Closing_Brace() {}
        static void TEXTFORMATTER_Too_Many_Format_Placeholders() {}
        static void TEXTFORMATTER_Too_Few_Format_Placeholders() {}
        static void TEXTFORMATTER_Some_Types_Expect_No_Format_Options() {}
        static void TEXTFORMATTER_Unmatched_Closing_Brace() {}
        static void TEXTFORMATTER_Open_Braces_Inside_Placeholder_Not_Allowed() {}
    };

    struct helper
    {
        template <typename T>
        static consteval Formatter<T> make_formatter(char const* params, ulong len)
        {
            if constexpr (Has_Formatter<T>)
            {
                return Formatter<T>(params, len);
            }
            else
            {
                char const* p = params;
                /// CHECK EMPTY
                {
                    while (p < params + len) 
                    {
                        if (*p != ' ')
                            __throw::TEXTFORMATTER_Some_Types_Expect_No_Format_Options();
                        p++;
                    }
                }
                return Formatter<T>();
            }
        }
    };

    struct TextChunk {
        ushort start;
        ushort len;
    };

private:

    char const* Format;
    TextChunk textchunk[sizeof...(Ts) + 1];
    Option<FormatIR> text_fmt[sizeof...(Ts)];
    Tuple<index_sequence_init<sizeof...(Ts)>> type_fmt;
    
private:
    
    template <int... _Index>
    consteval TextFormatter(char const* Format, ulong len, index_sequence<_Index...>)
    : Format(Format), textchunk{}, text_fmt{ ((void)_Index, False{}) ... }
    {
        #define Finite_State_Machine_Start while (true)

        struct FormatterArg {
            char const* start;
            ulong len;
        };

        char const* p = Format;
        char const* const end = Format + len;

        //TextChunk textchunk[sizeof...(Ts) + 1];
        //FormatterArg self_text_fmt_arg = { "", 0 }; 
        Option<FormatterArg> text_fmt_arg[sizeof...(Ts)] = { ((void)_Index, False{}) ... };
        FormatterArg type_fmt_arg[sizeof...(Ts)] = {};
        int arg_idx = 0;

        enum class State {
            CollectChunk,
            CollectTextFmt,
            CollectTypeFmt
        } state = State::CollectChunk;

        auto skip_spaces = [&]() {
            while (p < end && *p == ' ') { p++; }
        };


        /*  [IDEA]
        *
        *   CollectChunk:
        *     see '{'  =>  count {{...  =>  | even {{...  =>  skip
        *                                   | odd {{...   =>  last one is placeholder  =>  save chunk, goto: CollectTextFmt
        *
        *     see '}'  =>  count }}...  =>  | even }}..   =>  skip
        *                                   | odd }}...   =>  throw: Unmatched_Closing_Brace
        *
        *     see  _   =>  skip
        *
        *     EOF      =>  end FSM
        *
        *
        *   CollectTextFmt:  (no escape allowed)
        *     see '{'  =>  throw: Open_Braces_Inside_Placeholder_Not_Allowed
        *
        *     see ';'  =>  goto: CollectTypeFmt
        *
        *     see '}'  =>  goto: CollectChunk
        *
        *     see  _   =>  skip
        *
        *     EOF      =>  throw: Expecting_Format_Arguments_Or_Closing_Brace
        *
        *
        *   CollectTypeFmt:  (no escape allowed)
        *     see '{'  =>  throw: Open_Braces_Inside_Placeholder_Not_Allowed
        *
        *     see '}'  =>  goto: CollectChunk
        *
        *     see  _   =>  skip
        *
        *     EOF      =>  throw: Expecting_Format_Arguments_Or_Closing_Brace
        *
        */

        /*  [SIMPLIFY]
        *
        *   CollectChunk:
        *     see '{{'  =>  skip
        *
        *     see '{'   =>  save chunk, goto: CollectTextFmt
        *
        *     see '}}'  =>  skip
        *
        *     see '}'   =>  throw: Unmatched_Closing_Brace
        *
        *     see  _    =>  skip
        *
        *     EOF       =>  end FSM
        *
        */

        Finite_State_Machine_Start
        {
            if (state == State::CollectChunk)
            {
                char const* const checkpoint = p;
                //while (p < end && *p != '{' && *p != '}') { p++; }

                while (p < end)
                {
                    if (end - p > 0 && p[0] == '{')
                    {
                        if (end - p > 1 && p[1] == '{')
                            p += 2;  //  '{{'  =>  skip
                        else
                            break;   //  '{'   =>  goto: CollectTextFmt
                    }
                    else if (end - p > 0 && p[0] == '}')
                    {
                        if (end - p > 1 && p[1] == '}')
                            p += 2;  //  '}}'  =>  skip
                        else
                            __throw::TEXTFORMATTER_Unmatched_Closing_Brace();  
                    }
                    else
                    {
                        p++;
                    }
                }

                if (p < end && *p == '{')
                {
                    int arg_count = arg_idx;
                    if (arg_count >= sizeof...(Ts))  // enough no more
                        __throw::TEXTFORMATTER_Too_Many_Format_Placeholders();

                    textchunk[arg_idx] = TextChunk {
                        .start = static_cast<ushort>(checkpoint - Format),
                        .len = static_cast<ushort>(p - checkpoint)
                    };

                    p++;
                    state = State::CollectTextFmt;
                }
                else  // EOF
                {
                    int arg_count = arg_idx;
                    if (arg_count < sizeof...(Ts))  // still lacking
                        __throw::TEXTFORMATTER_Too_Few_Format_Placeholders();

                    textchunk[arg_idx] = TextChunk {
                        .start = static_cast<ushort>(checkpoint - Format),
                        .len = static_cast<ushort>(p - checkpoint)
                    };
                    break;  // end FSM
                }
            }
            else if (state == State::CollectTextFmt)
            {
                bool has_self = false;

                // check for self
                {
                    char const* const checkpoint = p;
                    skip_spaces();

                    if (end - p >= 4 && p[0] == 's' && p[1] == 'e' && p[2] == 'l' && p[3] == 'f')
                    {
                        p += 4;  // consume 'self'
                        has_self = true;
                    }
                    else
                    {
                        p = checkpoint;
                    }
                }

                if (has_self)
                {
                    skip_spaces();

                    if (p < end && *p == '{')
                    {
                        __throw::TEXTFORMATTER_Open_Braces_Inside_Placeholder_Not_Allowed();
                    }
                    if (p < end && *p == ';')
                    {
                        text_fmt_arg[arg_idx] = { False{} };
                        p++;  // consume ';'
                        state = State::CollectTypeFmt;
                    }
                    else if (p < end && *p == '}')
                    {
                        text_fmt_arg[arg_idx] = { False{} };
                        p++;  // consume '}'
                        arg_idx++;
                        state = State::CollectChunk;
                    }
                    else if (p < end)  // other chars
                    {
                        __throw::TEXTFORMATTER_Only_Self_Is_Allowed_When_Inheriting_Text_Format();
                    }
                    else  // EOF
                    {
                        __throw::TEXTFORMATTER_Expecting_Format_Arguments_Or_Closing_Brace();
                    }
                }
                else
                {
                    char const* const checkpoint = p;
                    while (p < end && *p != '{' && *p != ';' && *p != '}') { p++; }

                    if (p < end && *p == '{')
                    {
                        __throw::TEXTFORMATTER_Open_Braces_Inside_Placeholder_Not_Allowed();
                    }
                    if (p < end && *p == ';')
                    {
                        text_fmt_arg[arg_idx] =
                        {
                            True{}, 
                            FormatterArg {
                                .start = checkpoint,
                                .len = static_cast<ulong>(p - checkpoint)
                            }
                        };
                        p++;  // consume ';'
                        state = State::CollectTypeFmt;
                    }
                    else if (p < end && *p == '}')
                    {
                        text_fmt_arg[arg_idx] =
                        {
                            True{},
                                FormatterArg {
                                .start = checkpoint,
                                .len = static_cast<ulong>(p - checkpoint)
                            }
                        };
                        p++;  // consume '}'
                        arg_idx++;
                        state = State::CollectChunk;
                    }
                    else  // EOF
                    {
                        __throw::TEXTFORMATTER_Expecting_Format_Arguments_Or_Closing_Brace();
                    }
                }
            }
            else if (state == State::CollectTypeFmt)
            {
                char const* checkpoint = p;
                while (p < end && *p != '{' && *p != '}') { p++; }

                if (p < end && *p == '{')
                {
                    __throw::TEXTFORMATTER_Open_Braces_Inside_Placeholder_Not_Allowed();
                }
                if (p < end && *p == '}')
                {
                    type_fmt_arg[arg_idx] = FormatterArg {
                        .start = checkpoint,
                        .len = static_cast<ulong>(p - checkpoint)
                    };
                    p++;  // consume '}'
                    arg_idx++;
                    state = State::CollectChunk;
                }
                else // EOF
                {
                    __throw::TEXTFORMATTER_Expecting_Format_Arguments_Or_Closing_Brace();
                }
            }
        }

        // assign values
        {
            for (int i = 0; i < sizeof...(Ts); i++)
            {
                text_fmt[i] = text_fmt_arg[i].map(
                    [&](FormatterArg fmt_arg) -> FormatIR {
                        return FormatIR::generate(fmt_arg.start, fmt_arg.len);
                    }
                );
            }

            type_fmt = 
            { 
                { 
                    helper::template make_formatter<type_at<_Index, Ts...>>(
                        type_fmt_arg[_Index].start, 
                        type_fmt_arg[_Index].len
                    ) 
                } ... 
            };
        }

        #undef Finite_State_Machine_Start
    }

private:

    template <typename Buffer, int... _Index>
    requires (Is_Buffer<Buffer>)
    constexpr Result<void, FormatError> format(index_sequence<_Index...>, Buffer& buffer, Ts const&... values) const
    {
        #define GUARD(result) do { auto res = result; if (res == False{}) return res; } while (false)

        struct helper
        {
            static Result<void, FormatError> write_chunk(Buffer& buffer, char const* chars, ulong write_len)
            {
                char const* const end = chars + write_len;
                char const* p = chars;

                while (p < end)
                {
                    if (*p == '{')      // => must be even {{..
                    {
                        char const* checkpoint = p;
                        while (p < end && *p == '{') { p++; }
                        GUARD(buffer.write('{', (p - checkpoint) / 2));  // write duplicate chars
                    }
                    else if (*p == '}') // => must be even }}..
                    {
                        char const* checkpoint = p;
                        while (p < end && *p == '}') { p++; }
                        GUARD(buffer.write('}', (p - checkpoint) / 2));  // write duplicate chars
                    }
                    else
                    {
                        char const* checkpoint = p;
                        while (p < end && *p != '{' && *p != '}') { p++; }
                        GUARD(buffer.write(checkpoint, p - checkpoint));  // write chars
                    }
                }

                return { True{} };
            }
        };

        Result<void, FormatError> res = { True{} };

        // unroll...
        (
            (res = res.and_then
            (
                [&]() -> Result<void, FormatError> 
                {
                    GUARD(helper::write_chunk(
                        buffer, 
                        Format + textchunk[_Index].start, 
                        textchunk[_Index].len
                    ));

                    return static_cast<Cell<_Index> const&>(type_fmt).data.format(
                        buffer, 
                        values,
                        text_fmt[_Index].value_or(FormatIR {})
                    );
                }
            )), ...
        );

        if (res == False{}) 
            return res;

        GUARD(helper::write_chunk(
            buffer, 
            Format + textchunk[sizeof...(_Index)].start, 
            textchunk[sizeof...(_Index)].len
        ));
        
        return { True{} };
        #undef GUARD
    }

    template <typename Buffer, int... _Index>
    requires (Is_Buffer<Buffer>)
    constexpr Result<void, FormatError> format(index_sequence<_Index...>, FormatIR self_text_fmt, Buffer& buffer, Ts const&... values) const
    {
        #define GUARD(result) do { auto res = result; if (res == False{}) return res; } while (false)

        struct helper
        {
            static Result<void, FormatError> write_chunk(Buffer& buffer, char const* chars, ulong write_len)  
            {
                char const* const end = chars + write_len;
                char const* p = chars;

                while (p < end)
                {
                    if (*p == '{')      // => must be even {{..
                    {
                        char const* checkpoint = p;
                        while (p < end && *p == '{') { p++; }
                        GUARD(buffer.write('{', (p - checkpoint) / 2));  // write duplicate chars
                    }
                    else if (*p == '}') // => must be even }}..
                    {
                        char const* checkpoint = p;
                        while (p < end && *p == '}') { p++; }
                        GUARD(buffer.write('}', (p - checkpoint) / 2));  // write duplicate chars
                    }
                    else
                    {
                        char const* checkpoint = p;
                        while (p < end && *p != '{' && *p != '}') { p++; }
                        GUARD(buffer.write(checkpoint, p - checkpoint));  // write chars
                    }

                    /*else if (*p == '\n')
                    {
                        char const* checkpoint = p;
                        while (p < end && *p == '\n') { p++; }

                        if (styled && added_prefix)
                        {
                            GUARD(temp.write("\e[0m"));
                            added_width += 4;
                            added_prefix = false;
                        }

                        GUARD(buffer.write(prefix, prefixlen));
                        GUARD(buffer.write('\n', p - checkpoint));  // write duplicate chars
                        GUARD(buffer.write("\e[0m"));
                        added_width.map(
                            [&](ulong& added_width) { added_width += prefixlen + 4; }
                        );

                    }*/
                }

                return { True{} };
            }

            static void format_into(char* buffer, byte value)
            {
                buffer[2] = '0' + (value % 10);
                buffer[1] = '0' + (value / 10) % 10;
                buffer[0] = '0' + (value / 100) % 10;
            }
        };

        char prefix[64];
        byte prefixlen = 0;
        bool styled = false;

        // precompute text_fmt prefix
        {
            if (self_text_fmt.text_color == True{})
            {
                Color8bit color_vec = self_text_fmt.text_color.unwrap();
                std::memcpy(prefix + prefixlen, "\e[38;2;000;000;000m", 19);
                helper::format_into(prefix + prefixlen + 7, color_vec.red);
                helper::format_into(prefix + prefixlen + 11, color_vec.green);
                helper::format_into(prefix + prefixlen + 15, color_vec.blue);
                prefixlen += 19;
                styled = true;
            }

            if (self_text_fmt.background_color == True{})
            {
                Color8bit color_vec = self_text_fmt.background_color.unwrap();
                std::memcpy(prefix + prefixlen, "\e[48;2;000;000;000m", 19);
                helper::format_into(prefix + prefixlen + 7, color_vec.red);
                helper::format_into(prefix + prefixlen + 11, color_vec.green);
                helper::format_into(prefix + prefixlen + 15, color_vec.blue);
                prefixlen += 19;
                styled = true;
            }

            if (self_text_fmt.style != Style::Regular)
            {
                uint flags = +self_text_fmt.style;
                std::memcpy(prefix + prefixlen, "\e[_;_;_;_;_m", 12);
                prefixlen += 2;
                if (flags & +Style::Bold)      { prefix[prefixlen] = '1'; prefixlen += 2; }
                if (flags & +Style::Dim)       { prefix[prefixlen] = '2'; prefixlen += 2; }
                if (flags & +Style::Italic)    { prefix[prefixlen] = '3'; prefixlen += 2; }
                if (flags & +Style::Underline) { prefix[prefixlen] = '4'; prefixlen += 2; }
                if (flags & +Style::Strike)    { prefix[prefixlen] = '9'; prefixlen += 2; }
                prefix[prefixlen - 1] = 'm';
                styled = true;
            }
        }

        // write to buffer
        /*if (self_text_fmt.align.width > 0)
        {
            FormatBuffer temp;
            ulong added_width = 0;
            Result<void, FormatError> res = { True{} };
            bool added_prefix = false;

            bool add_head_prefix = false;
            bool first_chunk = true;

            // unroll...
            (
                (res = res.and_then
                (
                    [&]() -> Result<void, FormatError> 
                    {
                        if (styled && !added_prefix && textchunk[_Index].len > 0)
                        {
                            if (first_chunk)
                            {
                                add_head_prefix = true;
                                added_prefix = true;
                            }
                            else
                            {
                                GUARD(temp.write(prefix, prefixlen));
                                added_width += prefixlen;
                                added_prefix = true;
                            }
                        }

                        if (textchunk[_Index].len > 0)
                        {
                            GUARD(helper::write_chunk(
                                temp, 
                                Format + textchunk[_Index].start, 
                                textchunk[_Index].len
                            ));
                            first_chunk = false;
                        }

                        if (text_fmt[_Index] == True{})
                        {
                            if (styled && added_prefix)
                            {
                                GUARD(temp.write("\e[0m"));
                                added_width += 4;
                                added_prefix = false;
                            }

                            added_width += text_fmt[_Index].unwrap().added_width();
                            return static_cast<Cell<_Index> const&>(type_fmt).data.format(
                                temp, 
                                values,
                                text_fmt[_Index].unwrap()
                            );
                            
                            first_chunk = false;
                        }
                        else  // => {self}
                        {
                            if (styled && !added_prefix)
                            {
                                if (first_chunk)
                                {
                                    add_head_prefix = true;
                                    added_prefix = true;
                                }
                                else
                                {
                                    GUARD(temp.write(prefix, prefixlen));
                                    added_width += prefixlen;
                                    added_prefix = true;
                                }
                            }

                            char const* checkpoint = temp.data();
                            GUARD(static_cast<Cell<_Index> const&>(type_fmt).data.format(
                                temp, 
                                values,
                                FormatIR {}
                            ));
                            ulong write_len = temp.data() - checkpoint;

                            if (write_len > 0)
                            {
                                first_chunk = false;
                            }
                            
                            return { True{} };
                        }
                    }
                )), ...
            );

            if (res == False{}) 
                return res;

            // add alignment
            ulong visible_length = temp.size() - added_width;
            if (self_text_fmt.align.width > visible_length)
            {
                ulong fill_len = self_text_fmt.align.width - visible_length;
                ulong tail_padding_len = 0;
                ulong head_padding_len = 0;

                switch (self_text_fmt.align.mode)
                {
                    case Align::Mode::Left:  // head-prefix -> temp -> tail-prefix -> tail -> tail-padding -> tail-suffix
                    {
                        head_padding_len = 0;
                        tail_padding_len = fill_len;
                        break;
                    }
                    case Align::Mode::Right:  // head-prefix -> head-padding -> temp -> tail-prefix -> tail -> tail-suffix
                    {
                        head_padding_len = fill_len;
                        tail_padding_len = 0;
                        break;
                    }
                    case Align::Mode::Center: // head-prefix -> head-padding -> temp -> tail-prefix -> tail -> tail-pading -> tail-suffix
                    {
                        head_padding_len = fill_len / 2;
                        tail_padding_len = fill_len - fill_len / 2;
                        break;
                    }
                }

                // head-prefix
                if (add_head_prefix)
                {
                    GUARD(buffer.write(prefix, prefixlen));
                }

                // head-padding
                GUARD(buffer.write(self_text_fmt.align.fill, head_padding_len));

                // temp
                GUARD(buffer.write(temp.data(), temp.size()));

                // tail-prefix
                if (styled && !added_prefix && (textchunk[sizeof...(_Index)].len + tail_padding_len) > 0)
                {
                    GUARD(buffer.write(prefix, prefixlen));
                    added_width += prefixlen;
                    added_prefix = true;
                }

                // tail
                GUARD(helper::write_chunk(
                    buffer, 
                    Format + textchunk[sizeof...(_Index)].start, 
                    textchunk[sizeof...(_Index)].len
                ));

                // tail-padding
                GUARD(buffer.write(self_text_fmt.align.fill, tail_padding_len));

                // tail-suffix
                if (styled && added_prefix)
                {
                    GUARD(buffer.write("\e[0m"));
                    added_width += 4;
                    added_prefix = false;
                }
            }
            else
            {
                GUARD(buffer.write(temp.data(), temp.size()));
            }
        }*/
        if (self_text_fmt.align.width > 0)
        {
            FormatBuffer temp;
            struct slice { char const* start; ulong len; };

            /*  [basic idea]
            *
            *   (sizeof...(Ts) + 1) text chunks
            *   sizeof...(Ts) args
            *
            *   {self} args merge with text chunk => combine into merged_text
            *
            *   example: 
            *   arg[0] is {self}, arg[1] is {self}, arg[5] is {self}, arg[7] is empty
            *   text_chunk[0] -> arg[0] ->
            *   text_chunk[1] -> arg[1] ->
            *   text_chunk[2] -> arg[2] ->
            *   text_chunk[3] -> arg[3] ->
            *   text_chunk[4] -> arg[4] ->
            *   text_chunk[5] -> arg[5] ->
            *   text_chunk[6] -> arg[6] ->
            *   text_chunk[7] -> arg[7] ->
            *   text_chunk[8] -> arg[8] ->
            *   text_chunk[9]
            *   
            *   => merged_chunk[0] = text_chunk[0] + arg[0] + text_chunk[1] + arg[1] + text_chunk[2]           ;   added_width += prefixlen + suffixlen
            *   => formatted_arg[0] = arg[2]
            *   => merged_chunk[1] = text_chunk[3]                                                             ;   added_width += prefixlen + suffixlen
            *   => formatted_arg[1] = arg[3]
            *   => merged_chunk[2] = text_chunk[4]                                                             ;   added_width += prefixlen + suffixlen
            *   => formatted_arg[2] = arg[4]
            *   => merged_chunk[3] = text_chunk[5] + arg[5] + text_chunk[6] + arg[7] (empty) + text_chunk[8]   ;   added_width += prefixlen + suffixlen
            *   => formatted_arg[3] = arg[8]
            *   => merged_chunk[4] = text_chunk[9]                                                             ;   added_width += prefixlen + suffixlen
            *   
            *   has_left_padding  && (merged_chunk[0].len > 0)  =>  added_width -= prefixlen
            *   has_right_padding && (merged_chunk[4].len > 0)  =>  added_width -= suffixlen
            *
            *   visible_length = Sum i { merged_chunk[i].len } + Sum j { formatted_arg[j].len } - added_width;
            */

            /*  [problems]
            *
            *   \n \f \v cause ansi leak
            *   \r is weird but can be ignored
            *
            *   
            */
            
        }
        else
        {
            Result<void, FormatError> res = { True{} };
            bool added_prefix = false;

            // unroll...
            (
                (res = res.and_then
                (
                    [&]() -> Result<void, FormatError> 
                    {
                        if (styled && !added_prefix && textchunk[_Index].len > 0)
                        {
                            GUARD(buffer.write(prefix, prefixlen));
                            added_prefix = true;
                        }

                        GUARD(helper::write_chunk(
                            buffer, 
                            Format + textchunk[_Index].start, 
                            textchunk[_Index].len
                        ));

                        if (text_fmt[_Index] == True{})
                        {
                            if (styled && added_prefix)
                            {
                                GUARD(buffer.write("\e[0m"));
                                added_prefix = false;
                            }

                            return static_cast<Cell<_Index> const&>(type_fmt).data.format(
                                buffer, 
                                values,
                                text_fmt[_Index].unwrap()
                            );
                        }
                        else  // => {self}
                        {
                            if (styled && !added_prefix)
                            {
                                GUARD(buffer.write(prefix, prefixlen));
                                added_prefix = true;
                            }

                            return static_cast<Cell<_Index> const&>(type_fmt).data.format(
                                buffer, 
                                values,
                                FormatIR {}
                            );
                        }
                    }
                )), ...
            );

            if (res == False{}) 
                return res;

            if (styled && !added_prefix && textchunk[sizeof...(_Index)].len > 0)
            {
                GUARD(buffer.write(prefix, prefixlen));
                added_prefix = true;
            }

            GUARD(helper::write_chunk(
                buffer, 
                Format + textchunk[sizeof...(_Index)].start, 
                textchunk[sizeof...(_Index)].len
            ));

            if (styled && added_prefix)
            {
                GUARD(buffer.write("\e[0m"));
                added_prefix = false;
            }
        }

        return { True{} };
        #undef GUARD
    }

public:

    consteval TextFormatter(char const* Format, ulong len)
    : TextFormatter(Format, len, index_sequence_init<sizeof...(Ts)>{}) {}

    template <ulong N>
    consteval TextFormatter(char const (&Format)[N])
    : TextFormatter(Format, N - 1, index_sequence_init<sizeof...(Ts)>{}) {}

    template <typename Buffer>
    requires (Is_Buffer<Buffer>)
    constexpr Result<void, FormatError> format(Buffer& buffer, Ts const&... values) const {
        return format(index_sequence_init<sizeof...(Ts)>{}, buffer, values...);
    }
    
    template <typename Buffer>
    requires (Is_Buffer<Buffer>)
    constexpr Result<void, FormatError> format(FormatIR self_text_fmt, Buffer& buffer, Ts const&... values) const {
        return format(index_sequence_init<sizeof...(Ts)>{}, self_text_fmt, buffer, values...);
    }
};

template <typename... Ts>
using FormatString = TextFormatter<no_type_deduction<Ts>...>;

template <typename... Ts>
requires ((Has_Formatter<Ts> || Has_Default_Formatter<Ts>) && ...)
void print(FormatString<Ts...> Format, Ts const&... values)
{
    FormatBuffer buffer;
    Format.format(buffer, values...).unwrap();
    std::fwrite(buffer.data(), 1, buffer.size(), stdout);
}

template <typename... Ts>
requires ((Has_Formatter<Ts> || Has_Default_Formatter<Ts>) && ...)
void println(FormatString<Ts...> Format, Ts const&... values)          // ME PUSH CHARS FASSSST! 🦍⚡
{
    FormatBuffer buffer;
    Format.format(buffer, values...).unwrap();
    buffer.write("\n").unwrap();
    std::fwrite(buffer.data(), 1, buffer.size(), stdout);
}

template <typename... Ts>
requires ((Has_Formatter<Ts> || Has_Default_Formatter<Ts>) && ...)
constexpr String format(FormatString<Ts...> Format, Ts const&... values)
{
    String buffer;
    Format.format(buffer, values...).unwrap();
    return buffer;
}




/*

    template <typename... Ts>
    requires ((Has_Formatter<Ts> || Has_Default_Formatter<Ts>) && ...)
    Result<void, FormatError> print(FormatIR self, FormatString<Ts...> Format, Ts const&... values)
    {
        FormatBuffer buffer;
        auto res = Format.format(self, buffer, values...);
        if (res == False{}) return res;
        std::fwrite(buffer.data(), 1, buffer.size(), stdout);
        return { True{} };
    }

    template <typename... Ts>
    requires ((Has_Formatter<Ts> || Has_Default_Formatter<Ts>) && ...)
    Result<void, FormatError> println(FormatIR self, FormatString<Ts...> Format, Ts const&... values)
    {
        FormatBuffer buffer;
        auto res = Format.format(self, buffer, values...);
        if (res == False{}) return res;
        res = buffer.write("\n");
        if (res == False{}) return res;
        std::fwrite(buffer.data(), 1, buffer.size(), stdout);
        return { True{} };
    }

*/

#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

_MNC_END

#undef _MNC_BEGIN
#undef _MNC_END