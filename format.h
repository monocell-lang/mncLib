/*****************************************************************************************#\
##                                                                                        ##
##                ____ _____ _     _     _____ ___  ____  __  __    _  _____              ##
##               / ___| ____| |   | |   |  ___/ _ \|  _ \|  \/  |  / \|_   _|             ##
##              | |   |  _| | |   | |   | |_ | | | | |_) | |\/| | / _ \ | |               ##
##              | |___| |___| |___| |___|  _|| |_| |  _ <| |  | |/ ___ \| |               ##
##               \____|_____|_____|_____|_|   \___/|_| \_\_|  |_/_/   \_\_|               ##
##                                                                                        ##
##               MONOCELL FORMATING LIBRARY v1.0 (C++20 or later)                         ##
##                                                                                        ##
##                                                                                        ##
##                        Minimalist.  Powerful.  Terminal-friendly.                      ##
##                                                                                        ##
##                  => Just format() it. Just align() it. Just color() it.                ##
##                                                                                        ##
\#*****************************************************************************************/




#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "type_traits.h"
#include "optional.h"
#include "styling.h"





// ================================================================================================
//                           HOW TO USE THIS LIB 👉👉                   (cheatsheet below 🤫👇)
// ================================================================================================
//
// 1. PLACEHOLDERS & DUAL-SECTION SYNTAX
// ----------------------------------------------------------------------------
// Every format placeholder inside the format string follows a unified dual-section structure:
//     
//     { [Text Format Portion] ; [Type Format Portion] }
//
//   a) Text Format Portion:
//      - Extracted right after '{' up to the first ';' or closing '}'.
//      - Evaluated at compile-time via "consteval FormatIR::generate(char const*, ulong)".
//      - Computes Alignment/Padding, Styles (Bold, Italic, etc.), Foreground, and Background colors.
//      - Applicable to ALL types.
//      - If omitted or left blank, defaults to FormatIR::generate("", 0).
//
//   b) Type Format Portion:
//      - Extracted after the first ';' up to the closing '}'.
//      - Free-form syntax per type, BUT must NOT contain unescaped '{' or '}'.
//      - May contain inner semicolons ';' (e.g., for recursive type formatting like Opt<T, E>).
//
//   c) Arity & Argument Matching Rules:
//      - The number of placeholders '{}' MUST EXACTLY MATCH the number of arguments provided 
//        in the variadic parameter pack (`Ts...`).
//      - Mismatched argument count triggers a Compile-time Error (via `__throw`).
//
//      Examples:
//        println("x = {} and y = {}", 10, 20);  // OK: 2 placeholders, 2 args
//        println("x = {} and y = {}", 10);      // ERROR: Argument count mismatch!
//
//   d) Escaping Rules:
//      - Escaping curly braces is fully supported: '{{' outputs '{' and '}}' outputs '}'.
//      - Escaping is strictly evaluated AFTER formatting.
//      - Escapes ('{{' or '}}') are FORBIDDEN inside format placeholders!
//
//      Example:
//        println("workspace {{ theme: {}, folder: {} }}", theme, folder);
//        => "workspace { theme: dark, folder: /bin }"
//
// 2. GLOBAL STYLING (GLOBAL THEME / `self`)
// ----------------------------------------------------------------------------
// Global styling is detached from the format string itself and passed as a separate 
// `FormatIR` parameter using the `_fmt` User-Defined Literal.
//
//   a) Passing Global FormatIR:
//      - Pass a `FormatIR` object as the FIRST argument before the format string.
//      - Created via `_fmt` string literal operator.
//
//      Syntax:
//        println(" [Text Format Specifiers] "_fmt, "Format String...", args...);
//
//   b) Scope of Global Theme:
//      - Applies to all literal text segments in the format string.
//      - Applies to placeholders that explicitly request inheritance via `{self}`.
//      - If omitted, default global theme is `FormatIR::generate("", 0)`.
//
//   c) Examples:
//        println(" <30* @green "_fmt, "x = {}", 123);
//        println(" @navy $cream "_fmt, "Name tag: {self}, ID: {self ; .hex}", "Elen", 255);
//
// 3. STYLE INHERITANCE FROM `self`
// ----------------------------------------------------------------------------
// Any argument placeholder can inherit the exact `FormatIR` of the global theme (`self`).
//
//     { self ; [Type Format Portion] }
//
//   a) Rules & Constraints:
//      - Spaces around 'self' are ignored, but the identifier 'self' must be contiguous.
//      - NO additional text format specifiers can be merged with 'self' (DRY principle).
//
//      Valid Examples:
//        println(" $orange !i "_fmt, "Project: {self} | Count: {self ; .dec}", "mncLib", 42);
//        => Both "mncLib" and '42' inherit [italic + orange background].
//
//      Invalid Example:
//        "{self @Red}"  // ERROR: Merging other rules with 'self' is forbidden!
//
// 4. TEXT FORMAT SYNTAX SPECIFICATION
// ----------------------------------------------------------------------------
// Composed of up to 4 orthogonal format specifiers in ANY order, separated by optional whitespace:
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
//         <30   => Left alignment, width 30
//         _20*  => Center alignment, width 20, padded with '*'
//         >15a  => Right alignment, width 15, padded with 'a'
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
//       - RGB Tuple   : @(red, green, blue)  (values: 0-255)
//       - 6-Digit Hex : @#RRGGBB / @#rrggbb
//
//       Examples: 
//         Names     : @Carnelian, @Green, @gray, @Pink
//         RGB Tuples: @(220, 20, 60), @(120, 200, 255)
//         Hex       : @#7D5048, @#ff7f34
//
//   [4] BACKGROUND COLOR:
//       Syntax: $[color_spec]
//       - Color Name  : $Name / $name
//       - RGB Tuple   : $(red, green, blue)  (values: 0-255)
//       - 6-Digit Hex : $#RRGGBB / $#rrggbb
//
//       Examples:
//         Names     : $Navy,$cream
//         RGB Tuples: $(40, 40, 40) //         Hex       :$#2B2D42
//
// 5. TYPE FORMATTERS SPECIFICATION
// ----------------------------------------------------------------------------
//   - Strings (char const*, char[N], std::string_view):
//     Syntax: (empty) | .normal | .debug
//
//   - Integers (short, int, long, long long - signed/unsigned):
//     Syntax: (empty) | .dec | .+dec | .bin | .hex | .Hex | .oct
//
//   - Floating Point (float, double, long double):
//     Syntax: .(precision)(specifier)
//     - Specifiers: 'f' (fixed), 'e' (scientific), 'g' (general - default)
//     Examples:
//       .g     => Default precision (2) + general format
//       .06    => Precision 6 + general format
//       .10e   => Precision 10 + scientific format
//
//   - Optional / Result (Opt<T, E>):
//     Syntax: .[(Format For T) ; (Format For E)]
//     (Recursively parses format options for both Value and Error states)
//
// ================================================================================================
// CHEAT SHEET & QUICK EXAMPLES
// ================================================================================================
//
//   {}                                   =>  Default text format, default type format
//   {<30}                                =>  Left align width 30
//   {_30_}                               =>  Center align width 30, fill with '_'
//   {<30_ @Carnelian $Green}             =>  Left align, fill '_', Text color, BG color
//   {_20* @(179, 27, 27) $(0, 255, 0)}   =>  Center align, fill '*', RGB colors
//   {>35. !ui @#FFCA03 $#002B56}         =>  Right align, Underline+Italic, Hex colors
//
//
//   Example 1:
//   println("{} = {;.hex}", "Value", 255);    => Default styling, 'Value = 0xff'
//
//
//   Example 2:
//   println(
//      "<30 !i @(120, 200, 255) $#2B2D42"_fmt, 
//      "x = {!b @Pink ; .hex}, y = {self; .6f}, s = {; .debug}", 
//      123, 
//      1.666667
//      "hel\t\\lo"
//   );
//
//   => Global theme  : Left align 30, Italic, rgb(120, 200, 255) (ice blue) text, #2B2D42 (dark blue) background
//      123           : bold, pink text, formatted in hex
//      1.666667      : inherited global theme, formatted in fixed 6 decimal places
//      "hel\t\\lo"   : no theme (default theme), printed with \t and \ highlighed for debugging
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

template <typename T>
static constexpr bool has_buffer_trait = requires (T& buffer, char const* chars, ulong len, FormatIR IR) {
    requires (same_type<decltype(buffer.write(chars, len, IR)), Result<void, FormatError>>);
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
*   Example:
*     println("<30* @green"_fmt, "x = {}", 1);
*     println("@navy $cream"_fmt, "name tag: {self}, ID: {self}", "Elen", "IC66985478PDA");
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
*       + Optimized ANSI emission to the least sets and resets
*   - Reduced overhead for { and } escape for long continuous sequences
*
*   Example:
*     println("$green"_fmt, "x = {self}, y = {}", 3, 4);  // prints green "x = 3, y = " and default "4"
*     
*/


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
        static void TEXTFORMATTER_Only_Self_Is_Allowed_When_Inheriting_Text_Format() {}
        static void TEXTFORMATTER_Expecting_Format_Arguments_Or_Closing_Brace() {}
        static void TEXTFORMATTER_Too_Many_Format_Arguments() {}
        static void TEXTFORMATTER_Too_Few_Format_Arguments() {}
        static void TEXTFORMATTER_Some_Types_Expect_No_Format_Options() {}
        static void TEXTFORMATTER_Unmatched_Closing_Brace() {}
        static void TEXTFORMATTER_Open_Braces_Inside_Placeholder_Not_Allowed() {}
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
    Option<FormatIR> text_fmt[sizeof...(Ts)];
    Tuple<index_sequence_init<sizeof...(Ts)>> type_fmt;
    
private:
    
    template <int... _Index>
    consteval TextFormatter(char const* Format, ulong len, index_sequence<_Index...>)
    : Format(Format), textchunk{}, text_fmt{ ((void)_Index, False{}) ... }
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
        //FormatterArg self_text_fmt_arg = { "", 0 }; 
        Option<FormatterArg> text_fmt_arg[sizeof...(Ts)] = { ((void)_Index, False{}) ... };
        FormatterArg type_fmt_arg[sizeof...(Ts)] = {};
        bool has_self_assignment = false;
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
                        __throw::TEXTFORMATTER_Too_Many_Format_Arguments();

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
                        __throw::TEXTFORMATTER_Too_Few_Format_Arguments();

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
    Result<void, FormatError> format(index_sequence<_Index...>, FormatIR self_text_fmt, FormatBuffer& buffer, Ts const&... values) const
    {
        #define GUARD(result) do { auto res = result; if (res == False{}) return res; } while (false)

        struct helper
        {
            static Result<void, FormatError> write_chunk(FormatBuffer& buffer, char const* chars, ulong write_len)
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
        if (self_text_fmt.align.width > 0)
        {
            FormatBuffer temp;
            ulong added_width = 0;
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
                            GUARD(temp.write(prefix, prefixlen));
                            added_prefix = true;
                            added_width += prefixlen;
                        }

                        GUARD(helper::write_chunk(
                            temp, 
                            Format + textchunk[_Index].start, 
                            textchunk[_Index].len
                        ));

                        if (text_fmt[_Index] == True{})
                        {
                            if (styled && added_prefix)
                            {
                                GUARD(temp.write("\e[0m"));
                                added_prefix = false;
                                added_width += 4;
                            }

                            added_width += text_fmt[_Index].unwrap().added_width();
                            return static_cast<Cell<_Index> const&>(type_fmt).data.format(
                                temp, 
                                values,
                                text_fmt[_Index].unwrap()
                            );
                        }
                        else  // => {self}
                        {
                            if (styled && !added_prefix)
                            {
                                GUARD(temp.write(prefix, prefixlen));
                                added_prefix = true;
                                added_width += prefixlen;
                            }

                            return static_cast<Cell<_Index> const&>(type_fmt).data.format(
                                temp, 
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
                GUARD(temp.write(prefix, prefixlen));
                added_prefix = true;
                added_width += prefixlen;
            }

            GUARD(helper::write_chunk(
                temp, 
                Format + textchunk[sizeof...(_Index)].start, 
                textchunk[sizeof...(_Index)].len
            ));

            if (styled && added_prefix)
            {
                GUARD(temp.write("\e[0m"));
                added_prefix = false;
                added_width += 4;
            }

            // add alignment
            ulong visible_length = temp.size() - added_width;
            if (self_text_fmt.align.width > visible_length)
            {
                ulong fill_len = self_text_fmt.align.width - visible_length;
                switch (self_text_fmt.align.mode)
                {
                    case Align::Mode::Left:
                    {
                        GUARD(buffer.write(temp.data(), temp.size()));
                        GUARD(buffer.write(self_text_fmt.align.fill, fill_len));
                        break;
                    }
                    case Align::Mode::Right:
                    {
                        GUARD(buffer.write(self_text_fmt.align.fill, fill_len));
                        GUARD(buffer.write(temp.data(), temp.size()));
                        break;
                    }
                    case Align::Mode::Center:
                    {
                        GUARD(buffer.write(self_text_fmt.align.fill, fill_len / 2));
                        GUARD(buffer.write(temp.data(), temp.size()));
                        GUARD(buffer.write(self_text_fmt.align.fill, fill_len - fill_len / 2));
                        break;
                    }
                }
            }
            else
            {
                GUARD(buffer.write(temp.data(), temp.size()));
            }
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
    
    Result<void, FormatError> format(FormatIR self_text_fmt, FormatBuffer& buffer, Ts const&... values) const {
        return format(index_sequence_init<sizeof...(Ts)>{}, self_text_fmt, buffer, values...);
    }
};

template <typename... Ts>
using FormatString = TextFormatter<no_type_deduction<Ts>...>;

consteval FormatIR operator""_fmt(char const* _, ulong __) {
    return FormatIR::generate(_, __);
}

template <typename... Ts>
requires ((has_formatter<Ts> || has_default_formatter<Ts>) && ...)
Result<void, FormatError> print(FormatString<Ts...> Format, Ts const&... values)
{
    FormatBuffer buffer;
    auto res = Format.format(FormatIR {}, buffer, values...);
    if (res == False{}) return res;
    std::fwrite(buffer.data(), 1, buffer.size(), stdout);
    return { True{} };
}

template <typename... Ts>
requires ((has_formatter<Ts> || has_default_formatter<Ts>) && ...)
Result<void, FormatError> print(FormatIR self, FormatString<Ts...> Format, Ts const&... values)
{
    FormatBuffer buffer;
    auto res = Format.format(self, buffer, values...);
    if (res == False{}) return res;
    std::fwrite(buffer.data(), 1, buffer.size(), stdout);
    return { True{} };
}

template <typename... Ts>
requires ((has_formatter<Ts> || has_default_formatter<Ts>) && ...)
Result<void, FormatError> println(FormatString<Ts...> Format, Ts const&... values)
{
    FormatBuffer buffer;
    auto res = Format.format(FormatIR {}, buffer, values...);
    if (res == False{}) return res;
    res = buffer.write("\n");
    if (res == False{}) return res;
    std::fwrite(buffer.data(), 1, buffer.size(), stdout);
    return { True{} };
}

template <typename... Ts>
requires ((has_formatter<Ts> || has_default_formatter<Ts>) && ...)
Result<void, FormatError> println(FormatIR self, FormatString<Ts...> Format, Ts const&... values)  // ME PUSH CHARS FASSSST! 🦍⚡
{
    FormatBuffer buffer;
    auto res = Format.format(self, buffer, values...);
    if (res == False{}) return res;
    res = buffer.write("\n");
    if (res == False{}) return res;
    std::fwrite(buffer.data(), 1, buffer.size(), stdout);
    return { True{} };
}

template class TextFormatter<int>;

#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

_MNC_END

#undef _MNC_BEGIN
#undef _MNC_END