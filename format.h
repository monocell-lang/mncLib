#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "type_traits.h"
#include "optional.h"
#include "styling.h"





// ================================================================================================
//               MONOCELL TEXT FORMATTER SPECIFICATION (C++20 or latter)
// ================================================================================================
//
// 1. PLACEHOLDERS
// ----------------------------------------------------------------------------
// Every format placeholder follows a single, unified dual-section structure
//     
//     { [Text Format Portion] ; [Type Format Portion] }
//
//   a) Text Format Portion:
//      - Extracted from right after '{' up to the first ';' or closing '}'.
//      - Passed verbatim to "consteval FormatIR::generate(char const*, ulong)".
//      - Computes Alignment, Style, Text Color, and Background Color.
//      - Applicable to ALL types (Generic Format IR). Cannot be customized per type.
//      - If Omitted or left blank, defaults to FormatIR::generate("", 0).
//
//   b) Type Format Portion:
//      - Extracted after the first ';' up to the closing '}'.
//      - Syntax is free-form per type, BUT must NOT contain '{' or '}'.
//      - May contain inner semicolons ';'.
//
//   c) Arity & Argument Matching Rules:
//      - The number of argument placeholders '{}' MUST EXACTLY MATCH the number 
//        of types provided in the variadic parameter pack (`Ts...`).
//      - '{self: ...}' is a Global Theme Assignment and is EXCLUDED from argument counting.
//      - Mismatched placeholder count triggers a Compile-time Error via `__throw`.
//
//      Println examples;
//        println("{self: $red} {} and {}", 10, 20);  // OK: 2 placeholders, 2 args
//        println("{self: $red} {} and {}", 10);      // ERROR: Argument count mismatch!
//
//      TextFormatter (FormatString) examples:
//        TextFormatter<int, int>("{self: $red} {} and {}")  // OK: 2 placeholders, 2 args
//        TextFormatter<int, int>("{self: $red} {} ")        // ERROR: Argument count mismatch!
//
//   d) Escaping Rules: 
//      - Curly braces '{' and '}' are STRICTLY reserved for format argument placeholders.
//      - There is NO escape syntax for braces (e.g., '{{' or '}}' are NOT supported).
//
// 2. THE 'self' GLOBAL THEME ASSIGNMENT
// ----------------------------------------------------------------------------
// 'self' represents the formatting style applied to the entire format string 
// ITSELF (excluding the contents of argument placeholders '{}')
//
//     { self : [Text Format Portion] }
//
//   a) Placement Constraints:
//      - Must be placed EXACTLY at the beginning of the format string (Index 0).
//      - NO leading spaces or characters are allowed before '{self: ...}'.
//
//   b) Default Fallback:
//      - If '{self: ...}' ommited or left blank, it defaults to FormatIR::generate("", 0).
//
//   - Example:
//        "{self: >15. $gray}  hello world, peeps!"
//        => right-aligned + width 15 + '.' padding + gray background
//
// 3. 'self' STYLE INHERITANCE
// ----------------------------------------------------------------------------
// Any argument placeholder '{}' can inherit the exact Text Format IR of 'self' 
// (whether 'self' was explicitly defined or defaulted)
//
//     { self ; [Type Format Portion] }
//
//   a) Placement Constraints:
//      - Spaces around 'self' are ignored, but the word 'self' itself must be contiguous.
//      - NO additional text formatting parameters can be merged with 'self'.
//
//      Valid Examples:
//        "{self: $orange}[Monocell Format Lib]\n used in {self} projects!"
//        "{self; .hex}"   =>  Inherits 'self' text format + uses '.hex' type format
//
//      Invalid Examples:
//        "{self @Red}"    // ERROR: Merging other rules with 'self' is forbidden.
//
// 4. TEXT FORMAT SYNTAX SPECIFICATION
// ----------------------------------------------------------------------------
// Composed of up to 4 orthogonal format specifiers in ANY order, separated 
// by optional whitespace
//
//   [1] ALIGNMENT & PADDING:
//       Syntax: [< | > | _][width][optional: fill_char]
//       - '<' => Left alignment
//       - '>' => Right alignment
//       - '_' => Center alignment
//       - width: Positive integer specifying minimum field width
//       - fill_char: Single character used for padding (defaults to space ' ')
//
//       Examples: 
//         <30   => Left   + width 30 + no fill
//         _20*  => Center + width 20 + '*' padding
//         >15a  => Right  + width 15 + 'a' padding
//
//   [2] TEXT STYLES (STACKABLE):
//       Syntax: ![r|b|i|u|s|d]...
//       - '!' prefix followed by stackable modifier flags:
//           - 'r' => Regular
//           - 'b' => Bold
//           - 'i' => Italic
//           - 'u' => Underline
//           - 's' => Strikethrough
//           - 'd' => Dim
//
//       Examples: 
//         !bi   => Bold + Italic
//         !isu  => Italic + Strikethrough + Underline
//
//   [3] TEXT COLOR (FOREGROUND):
//       Syntax: @[color_spec]
//       - Color Name  : @Name / @name         
//       - RGB Tuple   : @(red, green, blue) (values: 0-255)
//       - 6-Digit Hex : @#RRGGBB / @#rrggbb
//
//       Examples: 
//         Color Names: @Carnelian, @Green, @gray
//         RGB Tuples: @(220, 20, 60) (Crimson), @(220, 203, 163) (Pearl), @(255, 105, 180) (Pink)
//         6-Digit Hex: @#7D5048, @#ff7f34
//
//   [4] BACKGROUND COLOR:
//       Syntax: $[color_spec]
//       - Color Name  : $Name / $name
//       - RGB Tuple   : $(red, green, blue) (values: 0-255)
//       - 6-Digit Hex : $#RRGGBB / $#rrggbb
//
//       Examples: nah, see TEXT COLOR!
//
// 5. TYPE FORMATTERS (EXACT TYPE MATCHING, NO CV-REF)
// ----------------------------------------------------------------------------
//   - Strings (char const*, char[N], char const[N]):
//     Syntax: (empty) | .normal | .debug
//
//   - Integers (short, int, long, long long - signed/unsigned):
//     Syntax: (empty) | .dec | .+dec | .bin | .hex | .Hex | .oct
//
//   - Floating Point (float, double, long double):
//     Syntax: .(empty | precision)(empty | g | f | e)
//
//     Examples:
//       .g    =>   precision 2 (default) + general float
//       .06   =>   precision 6 + general float (default)
//       .10e  =>   precision 15 + scientific float
//
//   - Optional / Result (Opt<T, E>):
//     Syntax: .[(Format For T) ; (Format For E)] (extracts format options recursively)
//
// ================================================================================================
// EXAMPLES & SYNTAX CHEAT SHEET
// ================================================================================================
//
//   {}                                   =>  Default text format, default type format
//   {<30}                                =>  Left align width 30
//   {_30_}                               =>  Center align width 30, fill with '_'
//   {<30_ @Carnelian $Green}             =>  Left align, fill '_', Text color, BG color
//   {_20* @(179, 27, 27) $(0, 255, 0)}   =>  Center align, fill '*', RGB colors
//   {>35. !ui @#FFCA03 $#002B56}         =>  Right align, Underline+Italic, Hex colors
//
//   {self: <30_ @Carnelian $Green}       =>  Assign global format to outer string
//   {self}                               =>  Inherit global 'self' text format
//   {self; .dec}                         =>  Inherit 'self' text format + '.dec' type format
//   {; .hex}                             =>  Default text format + '.hex' type format
//
// ================================================================================================



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
    requires (is_trivially_copy_able<Formatter<T>>);
    requires (is_trivially_move_able<Formatter<T>>);
    requires (is_trivially_destructible<Formatter<T>>);
};

template <typename T>
static constexpr bool has_default_formatter = requires (FormatBuffer& buffer, T const& value, FormatIR IR) {
    requires (same_type<decltype(Formatter<T>().format(buffer, value, IR)), Result<void, FormatError>>);
    requires (is_trivially_copy_able<Formatter<T>>);
    requires (is_trivially_move_able<Formatter<T>>);
    requires (is_trivially_destructible<Formatter<T>>);
};

// Syntax: (empty) | .normal | .debug
template <typename T>
requires (
    is_pointer<T> && same_type<remove_const<remove_pointer<T>>, char>          ||
    is_pointer<T> && same_type<remove_const<remove_pointer<T>>, signed char>   ||
    is_pointer<T> && same_type<remove_const<remove_pointer<T>>, unsigned char> ||
    is_array<T> && same_type<remove_const<remove_array<T>>, char>              ||
    is_array<T> && same_type<remove_const<remove_array<T>>, signed char>       ||
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

// Syntax: (empty) | .dec | .bin | .hex | .Hex | .oct
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

    Result<void, FormatError> format(FormatBuffer& buffer, T value, FormatIR IR = FormatIR {}) const
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
        return buffer.write(value, 1, IR);
    }
};

// Syntax: .[(Format For T) ; (Format For E)]
template <typename T, typename E>
requires (
    (has_formatter<T> || has_default_formatter<T> || same_type<T, void>) &&
    (has_formatter<E> || has_default_formatter<E> || same_type<E, void>)
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


            if constexpr (!same_type<T, void> && has_formatter<T>)
            {
                data.fmt_val = Formatter<T>(tstart, tlen);
            }
            else if constexpr (!same_type<T, void> && has_default_formatter<T>)
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


            if constexpr (!same_type<E, void> && has_formatter<E>)
            {
                data.fmt_err = Formatter<E>(fstart, flen);
            }
            else if constexpr (!same_type<E, void> && has_default_formatter<E>)
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

#pragma endregion
//////////////////////////////////////////////////////////////////////
////////////////////// class: FormatString ///////////////////////////
//////////////////////////////////////////////////////////////////////
#pragma region FormatString<Ts...>

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
    // {self: <30_ @Carnelian $Green}
    // => apply this formating style to the output string itself

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


template <typename... Ts>
requires ((has_formatter<Ts> || has_default_formatter<Ts>) && ...)
class TextFormatter {
private:

    static constexpr bool SORT_ALIGNMENT = false;

    struct Array {
    public:

        int data[sizeof...(Ts)];

    public:

        constexpr int& operator[](int index) { return data[index]; }
        constexpr int const& operator[](int index) const { return data[index]; }
    };

    static constexpr Array compute()
    {
        struct Pair { 
            int index; 
            int align; 
        };

        int index = 0;
        Pair arr[sizeof...(Ts)] = { Pair { index++, alignof(Formatter<Ts>) } ... };

        if constexpr (SORT_ALIGNMENT)
        {
            while (true)
            {
                bool swapped = false;
                
                for (int i = 0; i + 1 < sizeof...(Ts); i++)
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
        }

        Array result = {};
        for (int i = 0; i < sizeof...(Ts); i++)
        {
            result[i] = arr[i].index;
        }

        return result;
    }

    static constexpr Array map = compute();

    template <int _Index, typename T>
    struct TupleCell { T data; };

    template <typename index_sequence>
    struct Tuple;

    template <int... _Index>
    struct Tuple<index_sequence<_Index...>>: public TupleCell<map[_Index], Formatter<Ts>>... {};

private:

    struct __throw
    {
        static void TEXTFORMATTER_Cannot_Assign_Self_Format_More_Than_Once() {}
        static void TEXTFORMATTER_Self_Format_May_Only_Be_Assigned_At_The_Start_Of_Format_String() {}
        static void TEXTFORMATTER_Type_Format_Arguments_Are_Not_Allowed_In_Self_Format_Assignment() {}
        static void TEXTFORMATTER_Only_Self_Is_Allowed_When_Inheriting_Text_Format() {}
        static void TEXTFORMATTER_Expecting_Format_Arguments_Or_Closing_Brace() {}
        static void TEXTFORMATTER_Too_Many_Format_Arguments() {}
        static void TEXTFORMATTER_Too_Few_Format_Arguments() {}
        static void TEXTFORMATTER_Open_Braces_And_Closing_Braces_Are_Reserved_For_Placeholders_Only() {}
        static void TEXTFORMATTER_Some_Types_Expect_No_Format_Options() {}
    };

    struct helper
    {
        template <typename T>
        static consteval Formatter<T> make_formatter(char const* params, ulong len)
        {
            if constexpr (has_formatter<T>)
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
    FormatIR self_text_fmt;
    FormatIR text_fmt[sizeof...(Ts)];
    Tuple<index_sequence_init<sizeof...(Ts)>> type_fmt;
    
private:
    
    template <int... _Index>
    consteval TextFormatter(char const* Format, ulong len, index_sequence<_Index...>)
    : Format(Format), textchunk{}
    {
        #define once while (false)
        #define Finite_State_Machine_Start while (true)

        struct FormatterArg {
            char const* start;
            ulong len;
        };

        char const* p = Format;
        char const* const end = Format + len;

        //TextChunk textchunk[sizeof...(Ts) + 1];
        FormatterArg self_text_fmt_arg = { "", 0 }; 
        FormatterArg text_fmt_arg[sizeof...(Ts)] = {};
        FormatterArg type_fmt_arg[sizeof...(Ts)] = {};
        bool has_self_assignment = false;
        int arg_idx = 0;

        enum class State {
            CheckSelfAssignment,
            CollectChunk,
            CollectTextFmt,
            CollectTypeFmt
        } state = State::CheckSelfAssignment;

        auto skip_spaces = [&]() {
            while (p < end && *p == ' ') { p++; }
        };

        Finite_State_Machine_Start
        {
            if (state == State::CheckSelfAssignment)
            {
                // default next state if nothing happens
                state = State::CollectChunk;

                do
                {
                    char const* const checkpoint1 = p;
                    while (p < end && *p != '{') { p++; }
                    
                    if (!(p < end && *p == '{')) {
                        p = checkpoint1;
                        break;
                    }

                    char const* const checkpoint2 = p;
                    p++;
                    skip_spaces();

                    if (!(end - p >= 4 && p[0] == 's' && p[1] == 'e' && p[2] == 'l' && p[3] == 'f')) {
                        p = checkpoint1;
                        break;
                    }

                    p += 4;
                    skip_spaces();

                    if (!(p < end && *p == ':')) {
                        p = checkpoint1;
                        break;
                    }

                    p++;
                    char const* const checkpoint3 = p;
                    while (p < end && *p != ';' && *p != '}') { p++; }

                    if (*p == ';')
                        __throw::TEXTFORMATTER_Type_Format_Arguments_Are_Not_Allowed_In_Self_Format_Assignment();

                    if (*p != '}')
                        __throw::TEXTFORMATTER_Expecting_Format_Arguments_Or_Closing_Brace();

                    if (has_self_assignment)
                        __throw::TEXTFORMATTER_Cannot_Assign_Self_Format_More_Than_Once();

                    if (checkpoint2 != Format)
                        __throw::TEXTFORMATTER_Self_Format_May_Only_Be_Assigned_At_The_Start_Of_Format_String();
                    
                    self_text_fmt_arg = FormatterArg {
                        .start = checkpoint3,
                        .len = static_cast<ulong>(p - checkpoint3)
                    };
                    
                    p++; // consume '}'
                    has_self_assignment = true;
                    state = State::CheckSelfAssignment;
                } once;
            }
            else if (state == State::CollectChunk)
            {
                char const* const checkpoint = p;
                while (p < end && *p != '{' && *p != '}') { p++; }

                if (p < end && *p == '{')
                {
                    int arg_count = arg_idx;
                    if (arg_count >= sizeof...(Ts))  // enough no more
                        __throw::TEXTFORMATTER_Too_Many_Format_Arguments();

                    textchunk[arg_idx] = TextChunk {
                        .start = static_cast<ushort>(checkpoint - Format),
                        .len = static_cast<ushort>(p - checkpoint)
                    };

                    p++;
                    state = State::CollectTextFmt;
                }
                else if (p < end && *p == '}')
                {
                    __throw::TEXTFORMATTER_Open_Braces_And_Closing_Braces_Are_Reserved_For_Placeholders_Only();
                }
                else // end of format string
                {
                    int arg_count = arg_idx;
                    if (arg_count < sizeof...(Ts))  // still lacking
                        __throw::TEXTFORMATTER_Too_Few_Format_Arguments();

                    textchunk[arg_idx] = TextChunk {
                        .start = static_cast<ushort>(checkpoint - Format),
                        .len = static_cast<ushort>(p - checkpoint)
                    };
                    break;
                }
            }
            else if (state == State::CollectTextFmt)
            {
                enum class MiniState {
                    CheckSelf,
                    CheckAfterSelf,
                    CollectArg
                } ministate = MiniState::CheckSelf;

                Finite_State_Machine_Start
                {
                    if (ministate == MiniState::CheckSelf)
                    {
                        char const* const checkpoint = p;
                        skip_spaces();

                        if (end - p >= 4 && p[0] == 's' && p[1] == 'e' && p[2] == 'l' && p[3] == 'f')
                        {
                            p += 4;
                            ministate = MiniState::CheckAfterSelf;
                        }
                        else
                        {
                            p = checkpoint;
                            ministate = MiniState::CollectArg;
                        }
                    }
                    else if (ministate == MiniState::CheckAfterSelf)
                    {
                        skip_spaces();

                        if (p < end && *p == ';')
                        {
                            text_fmt_arg[arg_idx] = self_text_fmt_arg;
                            p++;  // consume ';'
                            state = State::CollectTypeFmt;
                        }
                        else if (p < end && *p == '}')
                        {
                            text_fmt_arg[arg_idx] = self_text_fmt_arg;
                            p++;  // consume '}'
                            arg_idx++;
                            state = State::CheckSelfAssignment;
                        }
                        else  // other chars or end of format string
                        {
                            __throw::TEXTFORMATTER_Only_Self_Is_Allowed_When_Inheriting_Text_Format();
                        }
                        break;
                    }
                    else if (ministate == MiniState::CollectArg)
                    {
                        char const* const checkpoint = p;
                        
                        while (p < end && *p != ';' && *p != '}') { p++; }

                        if (p < end && *p == ';')
                        {
                            text_fmt_arg[arg_idx] = FormatterArg {
                                .start = checkpoint,
                                .len = static_cast<ulong>(p - checkpoint)
                            };
                            p++;  // consume '}'
                            state = State::CollectTypeFmt;
                        }
                        else if (p < end && *p == '}')
                        {
                            text_fmt_arg[arg_idx] = FormatterArg {
                                .start = checkpoint,
                                .len = static_cast<ulong>(p - checkpoint)
                            };
                            p++;  // consume '}'
                            arg_idx++;
                            state = State::CheckSelfAssignment;
                        }
                        else // end of format string
                        {
                            __throw::TEXTFORMATTER_Expecting_Format_Arguments_Or_Closing_Brace();
                        }
                        break;
                    }
                }
            }
            else if (state == State::CollectTypeFmt)
            {
                char const* checkpoint = p;

                while (p < end && *p != '{' && *p != '}') { p++; }

                if (p < end && *p == '{')
                {
                    __throw::TEXTFORMATTER_Open_Braces_And_Closing_Braces_Are_Reserved_For_Placeholders_Only();
                }
                else if (p < end && *p == '}')
                {
                    type_fmt_arg[arg_idx] = FormatterArg {
                        .start = checkpoint,
                        .len = static_cast<ulong>(p - checkpoint)
                    };
                    p++;  // consume '}'
                    arg_idx++;
                    state = State::CheckSelfAssignment;
                }
                else // end of format string
                {
                    __throw::TEXTFORMATTER_Expecting_Format_Arguments_Or_Closing_Brace();
                }
            }
        }

        // assign values
        {
            self_text_fmt = FormatIR::generate(self_text_fmt_arg.start, self_text_fmt_arg.len);

            for (int i = 0; i < sizeof...(Ts); i++) {
                text_fmt[i] = FormatIR::generate(text_fmt_arg[i].start, text_fmt_arg[i].len);
            }

            type_fmt = { 
                helper::template make_formatter<type_at<_Index, Ts...>>(
                    type_fmt_arg[_Index].start, 
                    type_fmt_arg[_Index].len
                ) ... 
            };
        }

        #undef once
        #undef Finite_State_Machine_Start
    }

private:

    template <int _Index>
    using Cell = TupleCell<map[_Index], type_at<_Index, Formatter<Ts>...>>;

    template <int... _Index>
    Result<void, FormatError> format(index_sequence<_Index...>, FormatBuffer& buffer, Ts const&... values) const
    {
        Result<void, FormatError> res = { True{} };
        (
            (res = res.and_then
            (
                [&]() -> Result<void, FormatError> 
                {
                    auto res = buffer.write(Format + textchunk[_Index].start, textchunk[_Index].len, self_text_fmt);
                    if (res == False{}) return res;
                    return static_cast<Cell<_Index> const&>(type_fmt).data.format(buffer, values, text_fmt[_Index]);
                }
            )), ...
        );

        if (res == False{}) return res;
        return buffer.write(Format + textchunk[sizeof...(_Index)].start, textchunk[sizeof...(_Index)].len, self_text_fmt);
    }

public:

    consteval TextFormatter(char const* Format, ulong len)
    : TextFormatter(Format, len, index_sequence_init<sizeof...(Ts)>{}) {}

    template <ulong N>
    consteval TextFormatter(char const (&Format)[N])
    : TextFormatter(Format, N - 1, index_sequence_init<sizeof...(Ts)>{}) {}
    
    Result<void, FormatError> format(FormatBuffer& buffer, Ts const&... values) const {
        return format(index_sequence_init<sizeof...(Ts)>{}, buffer, values...);
    }

    constexpr char const* data() const {
        return Format;
    }

    constexpr ushort size() const {
        return textchunk[sizeof...(Ts)].len;
    }
};

template <typename... Ts>
using FormatString = TextFormatter<no_type_deduction<Ts>...>;

template <typename... Ts>
requires ((has_formatter<Ts> || has_default_formatter<Ts>) && ...)
Result<void, FormatError> print(FormatString<Ts...> Format, Ts const&... values)
{
    FormatBuffer buffer;
    auto res = Format.format(buffer, values...);
    if (res == False{}) return res;
    std::fwrite(buffer.data(), 1, buffer.size(), stdout);
    return { True{} };
}

template <typename... Ts>
requires ((has_formatter<Ts> || has_default_formatter<Ts>) && ...)
Result<void, FormatError> println(FormatString<Ts...> Format, Ts const&... values) // ME PUSH CHARS FASSSST! 🦍⚡
{
    FormatBuffer buffer;
    auto res = Format.format(buffer, values...);
    if (res == False{}) return res;
    res = buffer.write("\n");
    if (res == False{}) return res;
    std::fwrite(buffer.data(), 1, buffer.size(), stdout);
    return { True{} };
}

#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

_MNC_END

#undef _MNC_BEGIN
#undef _MNC_END