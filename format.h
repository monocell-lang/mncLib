#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "type_traits.h"
#include "optional.h"
#include "styling.h"



#define _MNC_BEGIN namespace mnc {
#define _MNC_END }

_MNC_BEGIN

#pragma endregion
//////////////////////////////////////////////////////////////////////
////////////////////// class: FormatBuffer ///////////////////////////
//////////////////////////////////////////////////////////////////////
#pragma region FormatBuffer

enum class FormatError: byte {
    InvalidParameter,
    MemoryRequestFailed
};

class FormatBuffer {
private:

    static constexpr ulong STACK_BUFFER_SIZE = 512;

    union { 
        struct { char buffer[STACK_BUFFER_SIZE]; } stack;
        struct { char* buffer; ulong cap; } heap;
    };
    ulong len; // bit 0 = HEAP FLAG | bit 0-63 = byte written

    static constexpr bool ENABLE_LOGGING = 0;

public:

    FormatBuffer(): len(0) {}
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

        if (
            !is_heap && !((ulong)chars >= (ulong)stack.buffer + STACK_BUFFER_SIZE - 2 || (ulong)chars + write_len <= (ulong)stack.buffer) ||
            is_heap && !((ulong)chars >= (ulong)heap.buffer + heap.cap - 1 || (ulong)chars + write_len <= (ulong)heap.buffer)
        )
        {
            std::fprintf(stderr, "[mnc::FormatBuffer] ;; ABORT WARNING ;; reading and writing buffer overlap\n");
            std::fflush(stderr);
            std::abort();
        }

        if (!is_heap)
        {
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

    Result<void, FormatError> write(char ch, ulong duplicates)
    {
        if (duplicates == 0)
            return { True{} };

        const ulong is_heap = len & 1;
        const ulong current_len = len >> 1;
        const ulong current_cap = (is_heap? heap.cap: STACK_BUFFER_SIZE - 1);
        const ulong new_len = current_len + duplicates;

        if (!is_heap)
        {
            if (new_len <= current_cap)
            {
                //std::memcpy(stack.buffer + current_len, chars, write_len);
                std::memset(stack.buffer + current_len, ch, duplicates);
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
                std::memset(new_buffer + current_len, ch, duplicates);
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
                std::memset(heap.buffer + current_len, ch, duplicates);
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
                std::memset(new_buffer + current_len, ch, duplicates);
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

    template <ulong N>
    Result<void, FormatError> write(char const(&chars)[N])
    {
        return write(chars, N - 1);
    }

    Result<void, FormatError> write(char const* chars, ulong write_len, FormatIR IR)
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
        Result<void, FormatError> res = { True{} };

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

        if (IR.align.width > write_len)
        {
            ulong fill_len = IR.align.width - write_len;
            switch (IR.align.mode)
            {
                case Align::Mode::Left:
                {
                    GUARD(write(chars, write_len));
                    GUARD(write(IR.align.fill, fill_len));
                    break;
                }
                case Align::Mode::Right:
                {
                    GUARD(write(IR.align.fill, fill_len));
                    GUARD(write(chars, write_len));
                    break;
                }
                case Align::Mode::Center:
                {
                    GUARD(write(IR.align.fill, fill_len / 2));
                    GUARD(write(chars, write_len));
                    GUARD(write(IR.align.fill, fill_len - fill_len / 2));
                    break;
                }
            }
        }
        else
        {
            GUARD(write(chars, write_len));
        }

        if (styled)
            return write("\e[0m");
        else
            return { True{} };

        #undef GUARD
    }

    Result<void, FormatError> write(char ch, ulong duplicates, FormatIR IR)
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
        Result<void, FormatError> res = { True{} };

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

        if (IR.align.width > duplicates)
        {
            ulong fill_len = IR.align.width - duplicates;
            switch (IR.align.mode)
            {
                case Align::Mode::Left:
                {
                    GUARD(write(ch, duplicates));
                    GUARD(write(IR.align.fill, fill_len));
                    break;
                }
                case Align::Mode::Right:
                {
                    GUARD(write(IR.align.fill, fill_len));
                    GUARD(write(ch, duplicates));
                    break;
                }
                case Align::Mode::Center:
                {
                    GUARD(write(IR.align.fill, fill_len / 2));
                    GUARD(write(ch, duplicates));
                    GUARD(write(IR.align.fill, fill_len - fill_len / 2));
                    break;
                }
            }
        }
        else
        {
            GUARD(write(ch, duplicates));
        }

        if (styled)
            return write("\e[0m");
        else
            return { True{} };

        #undef GUARD
    }

    template <ulong N>
    Result<void, FormatError> write(char const(&chars)[N], FormatIR IR)
    {
        return write(chars, N - 1, IR);
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
static constexpr bool has_formatter = requires (char const* params, ulong len, FormatBuffer& buffer, T const& value, FormatIR IR) {
    requires (same_type<decltype(Formatter<T>(params, len).format(buffer, value, IR)), Result<void, FormatError>>);
};

template <typename T>
static constexpr bool has_default_formatter = requires (FormatBuffer& buffer, T const& value, FormatIR IR) {
    requires (same_type<decltype(Formatter<T>().format(buffer, value, IR)), Result<void, FormatError>>);
};

// Syntax: .(mode: normal | debug | empty)
template <typename T>
requires (
    is_pointer<T> && same_type<remove_const<remove_pointer<T>>, char>           ||
    is_pointer<T> && same_type<remove_const<remove_pointer<T>>, signed char>    ||
    is_pointer<T> && same_type<remove_const<remove_pointer<T>>, unsigned char>  ||
    is_array<T> && same_type<remove_const<remove_array<T>>, char>               ||
    is_array<T> && same_type<remove_const<remove_array<T>>, signed char>        ||
    is_array<T> && same_type<remove_const<remove_array<T>>, unsigned char>
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
        //static void FORMATTER_CSTR_Unknown_String_Format() {}
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
    Result<void, FormatError> format(FormatBuffer& buffer, char const* value, ulong len, FormatIR IR = FormatIR {}) const
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
                        case '\'': GUARD(buffer.write("\'", profile)); i++; continue;
                        case '\"': GUARD(buffer.write("\"", profile)); i++; continue;
                        //case '\\': GUARD(buffer.write("\\", profile)); i++; continue;
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
    Result<void, FormatError> format(FormatBuffer& buffer, char const* value, FormatIR IR = FormatIR {}) const
    {
        return format(buffer, value, std::strlen(value), IR);
    }

    // debug ignores IR
    template <ulong N>
    Result<void, FormatError> format(FormatBuffer& buffer, char const (&value)[N], FormatIR IR = FormatIR {}) const
    {
        return format(buffer, value, N - 1, IR);
    }
};

// Syntax: .(mode: dec | bin | hex | Hex | oc | empty)
template <typename T>
requires (
    same_type<T, signed short>       ||
    same_type<T, signed int>         ||
    same_type<T, signed long>        ||
    same_type<T, signed long long>   ||
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
    requires (same_type<remove_cv<Ty>, signed short>)
    struct make_unsigned<Ty> {
        using type = unsigned short;
    };

    template <typename Ty>
    requires (same_type<remove_cv<Ty>, signed int>)
    struct make_unsigned<Ty> {
        using type = unsigned int;
    };

    template <typename Ty>
    requires (same_type<remove_cv<Ty>, signed long>)
    struct make_unsigned<Ty> {
        using type = unsigned long;
    };

    template <typename Ty>
    requires (same_type<remove_cv<Ty>, signed long long>)
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

            if (helper::same_str(sstart, slen, "dec"))      mode = Mode::Decimal;
            else if (helper::same_str(sstart, slen, "bin")) mode = Mode::Binary;
            else if (helper::same_str(sstart, slen, "hex")) mode = Mode::Hexadecimal;
            else if (helper::same_str(sstart, slen, "Hex")) mode = Mode::CapHexadecimal;
            else if (helper::same_str(sstart, slen, "oct")) mode = Mode::Octal;
            else __throw::FORMATTER_INT_Invalid_Format_Parameter_Syntax();
        }
    }

    template <ulong N>
    consteval Formatter(char const (&params)[N]): Formatter(params, N - 1) {}

    Result<void, FormatError> format(FormatBuffer& buffer, T value, FormatIR IR = FormatIR {}) const
    {
        switch (mode)
        {
            case Mode::Decimal:
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
                if constexpr (is_signed_int<T>) if (is_negative)
                {
                    buffer_temp[--i] = '-';
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


// syntax: .(precision: uint | empty)(mode: g | f | e | empty)
template <typename T>
requires (
    same_type<T, float>     ||
    same_type<T, double>    ||
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
                if (s[i] < '0' && '9' < s[i]) // non-digit chars
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

    Result<void, FormatError> format(FormatBuffer& buffer, T value, FormatIR IR = FormatIR {}) const
    {
        char buffer_temp[64];
        char fmt_s[8] = "%.02"; // base

        // two-digit precision (e.g. "06")
        fmt_s[2] = '0' + (precision / 10);
        fmt_s[3] = '0' + (precision % 10);

        if constexpr (!same_type<T, long double>)
        {
            switch (mode)
            {
                case Mode::General:
                fmt_s[4] = 'g';
                break;

                case Mode::Fixed:
                fmt_s[4] = 'f';
                break;

                case Mode::Scientific:
                fmt_s[4] = 'e';
                break;
            }
            fmt_s[5] = '\0';
        }
        else
        {
            fmt_s[4] = 'L';
            switch (mode)
            {
                case Mode::General:
                fmt_s[5] = 'g';
                break;

                case Mode::Fixed:
                fmt_s[5] = 'f';
                break;

                case Mode::Scientific:
                fmt_s[5] = 'e';
                break;
            }
            fmt_s[6] = '\0';
        }
        int write_len = sprintf(buffer_temp, fmt_s, value);

        return buffer.write(buffer_temp, write_len, IR);
    }
};

template <typename T>
requires (
    !same_type<remove_const<remove_pointer<T>>, char>           &&
    !same_type<remove_const<remove_pointer<T>>, signed char>    &&
    !same_type<remove_const<remove_pointer<T>>, unsigned char>
)
class Formatter<T*> {
public:

    Result<void, FormatError> format(FormatBuffer& buffer, T const* value, FormatIR IR = FormatIR {}) const
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
class Formatter<decltype(nullptr)> {
public:

    Result<void, FormatError> format(FormatBuffer& buffer, decltype(nullptr) value, FormatIR IR = FormatIR {}) const
    {
        return buffer.write("0000000000000000", IR);
    }
};

template <>
class Formatter<bool> {
public:

    Result<void, FormatError> format(FormatBuffer& buffer, bool value, FormatIR IR = FormatIR {}) const
    {
        if (value)
            return buffer.write("true", IR);
        else
            return buffer.write("false", IR);
    }
};

// Syntax: .[ Format_Arg_T ; Format_Arg_E ]
template <typename T>
requires (
    same_type<T, char>          ||
    same_type<T, signed char>   ||
    same_type<T, unsigned char>
)
class Formatter<T> {
public:

    Result<void, FormatError> format(FormatBuffer& buffer, T value, FormatIR IR = FormatIR {}) const
    {
        return buffer.write(&value, 1, IR);
    }
};

template <typename T, typename E>
requires (
    (has_formatter<T> || same_type<T, void>) &&
    (has_formatter<E> || same_type<E, void>)
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
        static void FORMATTER_OPT_Argument_For_Void_Must_Be_Empty() {}
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

            if constexpr (!same_type<T, void> && !same_type<E, void>)
            {
                data.fmt_val = Formatter<T>(tstart, tlen);
                data.fmt_err = Formatter<E>(fstart, flen);
            }
            else if constexpr (!same_type<T, void> && same_type<E, void>)
            {
                if (!helper::is_empty_str(fstart, flen))
                    __throw::FORMATTER_OPT_Argument_For_Void_Must_Be_Empty();

                data.fmt_val = Formatter<T>(tstart, tlen);
            }
            else if (same_type<T, void> && !same_type<E, void>)
            {
                if (!helper::is_empty_str(tstart, tlen))
                    __throw::FORMATTER_OPT_Argument_For_Void_Must_Be_Empty();

                data.fmt_err = Formatter<E>(fstart, flen);
            }
        }
    }

    template <ulong N>
    consteval Formatter(char const(&params)[N]): Formatter(params, N - 1) {}

    Result<void, FormatError> format(FormatBuffer& buffer, Opt<T, E> const& value, FormatIR IR = FormatIR {}) const
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

template class Formatter<char const*>;
template class Formatter<int>;
template class Formatter<long long>;
template class Formatter<unsigned int>;
template class Formatter<unsigned long long>;
template class Formatter<double>;
template class Formatter<long double>;
template class Formatter<void*>;
template class Formatter<char>;
template class Formatter<Result<int, char const*>>;
template class Formatter<Result<void, float>>;
template class Formatter<Option<int>>;


#pragma endregion
//////////////////////////////////////////////////////////////////////
////////////////////// class: FormatString ///////////////////////////
//////////////////////////////////////////////////////////////////////
#pragma region FormatString<Ts...>

template <typename... Ts>
class TextFormatter {
private:

    struct TextChunk {
        ushort start;
        ushort len;
    };

    struct Array {
    public:

        int data[sizeof...(Ts) * 3 + 1];

    public:

        constexpr int& operator[](int index) { return data[index]; }
        constexpr int const& operator[](int index) const { return data[index]; }
    };

    template <typename Ty, typename Alt>
    using alter = Alt;

    static constexpr Array compute()
    {
        struct Pair { 
            int index; 
            int align; 
        };

        int index = 0;
        Pair arr[sizeof...(Ts) * 3] = 
        {
            Pair { index++, alignof(alter<Ts, FormatIR>) } ...,
            Pair { index++, alignof(alter<Ts, TextChunk>) } ...,
            Pair { index++, alignof(Formatter<Ts>) } ... 
        };

        while (true)
        {
            bool swapped = false;
            
            for (int i = 0; i + 1 < sizeof...(Ts) * 3; i++)
            {
                if (arr[i].align < arr[i + 1].align)
                {
                    Pair temp = arr[i];
                    arr[i] = arr[i + 1];
                    arr[i + 1] = temp;
                    swapped = true;
                }
            }

            if (!swapped)
                break;
        }

        Array result = {};
        for (int i = 0; i < sizeof...(Ts) * 3; i++)
        {
            result[i] = arr[i].index;
        }

        return result;
    }

    static constexpr Array map = compute();
    
    template <int index, typename T>
    struct DataCell {
        T data;
    };

    template <typename index_sequence>
    struct Data;

    template <int... index>
    struct Data<index_sequence<index...>>
    : public DataCell<map[index], type_at<map[index], alter<Ts, FormatIR>..., alter<Ts, TextChunk>..., Formatter<Ts>...>>... {
    public:

        template <int idx>
        using TextChunk_DataCell = DataCell<map[idx], TextChunk>;

        template <int idx>
        using Formatter_DataCell = DataCell<map[idx], type_at<map[idx], Formatter<Ts>...>>;

        template <int idx>
        using Formatter_at = type_at<map[idx], Formatter<Ts>...>;

    public:

        template <int idx>
        TextChunk& get_chunk() {
            return static_cast<TextChunk_DataCell<idx>&>(*this).data;
        }

        template <int idx>
        TextChunk const& get_chunk() const {
            return static_cast<TextChunk_DataCell<idx> const&>(*this).data;
        }

        template <int idx>
        Formatter_at<idx>& get_formatter() {
            return static_cast<Formatter_DataCell<idx>&>(*this).data;
        }

        template <int idx>
        Formatter_at<idx> const& get_formatter() const {
            return static_cast<Formatter_DataCell<idx> const&>(*this).data;
        }
    };

private:

    char const* text;
    FormatIR IR_self;
    Data<index_sequence_init<sizeof...(Ts) * 3>> data;
    TextChunk tail;

public:
    
    consteval TextFormatter(char const* Format, ulong len)
    {
        ////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        // {}
        // {:<30}
        // {:30_}
        // {:_30_ @Carnelian $Green}
        // {:_30_ @(179, 27, 27) $(0, 255, 0)}
        // {:_30_ !c @#FFCA03 $#002B56}

        // WHERE:
        // first 3: align, with, fill (not mentioned => ' ')
        // ! for style
        // @ for text color
        // $ for background color
        // spaces dont matter

        // {self: _30_ !c @(179, 27, 27) $(0, 255, 0)} 
        // => apply this formating style to the output string itself

        // other Formatters use the second param after ;
        // {... ; .hex}
        // {... ; .bin}
        // {... ; .debug}
        // {; .option1}  leaving first param empty is ok

        ////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        // TODO
    }

    Result<void, FormatError> format(FormatBuffer& buffer, Ts const&... values) const
    {
        // TODO
    }
};

template <typename... Ts>
using FormatString = TextFormatter<no_type_deduction<Ts>...>;


template <typename... Ts>
void println(FormatString<Ts...> Format, Ts const&... args) {
    
}

template class TextFormatter<>::Data<index_sequence_init<0>>;
template class TextFormatter<int, int, double, double, char>;
template class TextFormatter<int, int, double, double, char>::Data<index_sequence_init<15>>;


#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

_MNC_END

#undef _MNC_BEGIN
#undef _MNC_END