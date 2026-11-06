#pragma once
#include "type_traits.h"
#include "optional.h"
#include "utility.h"


#define _MNC_BEGIN namespace mnc {
#define _MNC_END }

_MNC_BEGIN


//////////////////////////////////////////////////////////////////////
////////////////////// Formatting Styles /////////////////////////////
//////////////////////////////////////////////////////////////////////
#pragma region styles

enum class Color
{
    // 🎨 Basic
    Red,       //   { 255,   0,   0 }
    Green,     //   {   0, 255,   0 }   //console friendly
    Blue,      //   {   0,   0, 255 }
    Yellow,    //   { 255, 255,   0 }
    Cyan,      //   {   0, 255, 255 }
    Magenta,   //   { 255,   0, 255 }
    Black,     //   {   0,   0,   0 }
    White,     //   { 255, 255, 255 }

    // 🎨 Bonus shades
    Crimson,   //   { 220,  20,  60 }
    Purple,    //   { 128,   0, 128 }
    Pink,      //   { 255, 105, 180 }   (console friendly)
    Brown,     //   { 150,  75,   0 }   (console friendly)
    Orange,    //   { 255, 165,   0 }   (console friendly)
    Amber,     //   { 255, 193,   7 }   (console friendly)
    Chateu,    //   {  78, 186, 101 }
    Cream,     //   { 238, 230, 158 }   (console friendly)
    Sky,       //   { 130, 200, 229 }
    Violet,    //   { 148,   0, 211 }
    Lime,      //   { 191, 255,   0 }
    Teal,      //   {   0, 128, 128 }
    Navy,      //   {   0,   0, 128 }
    Gold,      //   { 255, 215,   0 }
    Silver,    //   { 192, 192, 192 }
    Deepsea,   //   {   0,   0, 200 }
    Gray,      //   { 100, 100, 100 }   (console friendly)
    Claude,    //   { 215, 119,  87 }   (console friendly)
    Carnelian, //   { 179,  27,  27 }   (console friendly)
    Pearl      //   { 220, 203, 163 }
};

struct Color8bit {
public:

    byte red; 
    byte green; 
    byte blue;

public:

    constexpr Color8bit bright(uint percent) const
    {
        if (percent > 100) 
            percent = 100;

        struct helper
        {
            static constexpr byte rounded_mul(byte a, uint percent)
            {
                uint b = static_cast<uint>(a) * percent;
                if (b % 100 >= 50)
                    return b / 100 + 1;
                else
                    return b / 100;
            }
        };

        return Color8bit {
            helper::rounded_mul(red, percent),
            helper::rounded_mul(green, percent),
            helper::rounded_mul(blue, percent)
        };
    }
};

constexpr Color8bit operator+(Color color)
{
    constexpr Color8bit table[] = 
    {
        /*   Red       */  { 255,   0,   0 },
        /*   Green     */  {   0, 255,   0 }, //console friendly
        /*   Blue      */  {   0,   0, 255 },
        /*   Yellow    */  { 255, 255,   0 },
        /*   Cyan      */  {   0, 255, 255 },
        /*   Magenta   */  { 255,   0, 255 },
        /*   Black     */  {   0,   0,   0 },
        /*   White     */  { 255, 255, 255 },

        // 🎨 Bonus shades
        /*   Crimson   */  { 220,  20,  60 },
        /*   Purple    */  { 128,   0, 128 },
        /*   Pink      */ { 255, 105, 180 }, //console friendly
        /*   Brown     */ { 150,  75,   0 }, //console friendly
        /*   Orange    */ { 255, 165,   0 }, //console friendly
        /*   Amber     */ { 255, 193,   7 }, //console friendly
        /*   Chateu    */ {  78, 186, 101 },
        /*   Cream     */ { 238, 230, 158 },
        /*   Sky       */ { 130, 200, 229 },
        /*   Violet    */ { 148,   0, 211 },
        /*   Lime      */ { 191, 255,   0 },
        /*   Teal      */ {   0, 128, 128 },
        /*   Navy      */ {   0,   0, 128 },
        /*   Gold      */ { 255, 215,   0 },
        /*   Silver    */ { 192, 192, 192 },
        /*   Deepsea   */ {   0,   0, 200 },
        /*   Gray      */ { 100, 100, 100 }, //console friendly
        /*   Claude    */ { 215, 119,  87 }, //console friendly
        /*   Carnelian */ { 179,  27,  27 }, //console friendly
        /*   Pearl     */ { 220, 203, 163 }
    };

    return table[static_cast<int>(color)];
}

enum class Style: byte
{
    Regular   = 0,
    Bold      = 1 << 0,
    Dim       = 1 << 1,
    Italic    = 1 << 2,
    Underline = 1 << 3,
    Strike    = 1 << 4
};

constexpr Style operator|(Style A, Style B)
{
    return static_cast<Style>(static_cast<byte>(A) | static_cast<byte>(B));
}

constexpr Style operator&(Style A, Style B)
{
    return static_cast<Style>(static_cast<byte>(A) & static_cast<byte>(B));
}

constexpr uint operator+(Style style)
{
    return static_cast<uint>(style);
}

struct Align {
public:

    enum class Mode: byte
    {
        Left,
        Right, 
        Center 
    };

public:

    ushort width;
    Mode mode;
    char fill;
};

#pragma endregion
//////////////////////////////////////////////////////////////////////
////////////////////// class: FormatIR ///////////////////////////////
//////////////////////////////////////////////////////////////////////
#pragma region FormatIR

struct FormatIR {
public:

    Align align = { 0, Align::Mode::Left, ' ' };
    Style style = Style::Regular;
    Option<Color8bit> text_color = { False{} };
    Option<Color8bit> background_color = { False{} };

private:

    struct __throw 
    {
        static void FormatIR_Unknown_Format_Option() {}
        static void FormatIR_Expecting_Alignment_Width() {}
        static void FormatIR_Alignment_Width_Too_Large() {}
        static void FormatIR_Text_Alignment_Was_Specified_More_Than_Once() {}
        static void FormatIR_Expecting_Style_Flags() {}
        static void FormatIR_Invalid_Style_Flags() {}
        static void FormatIR_Duplicate_Style_Flags() {}
        static void FormatIR_Style_Flags_Was_Specified_More_Than_Once() {}
        static void FormatIR_Expecting_Color_Name_Or_RGB_Code() {}
        static void FormatIR_Expected_3_Arguments_In_RGB() {}
        static void FormatIR_Expecting_RGB_Value() {}
        static void FormatIR_RGB_Value_Exceeding_255() {}
        static void FormatIR_Expected_Closing_Paren_In_RGB() {}
        static void FormatIR_Expecting_RGB_Hex_Code() {}
        static void FormatIR_Expecting_Uniform_Letter_Case_For_RGB_Hex() {}
        static void FormatIR_Color_Name_Too_Long() {}
        static void FormatIR_Color_Name_Not_Recored() {}
        static void FormatIR_Text_Color_Was_Specified_More_Than_Once() {}
        static void FormatIR_Background_Color_Was_Specified_More_Than_Once() {}
    };

public:

    static consteval FormatIR generate(char const* params, ulong len)
    {
        FormatIR result = {};
        bool has_align = false;
        bool has_style = false;
        // has_text_color = false (in Option<Color8bit>)
        // has_background_color = false (in Option<Color8bit>)

        char const* p = params;
        char const* end = params + len;

        auto skip_spaces = [&]() {
            while (p < end && *p == ' ') { p++; }
        };

        while (p < end)
        {
            skip_spaces();

            if (p == end)
                return result;

            if (p < end && (*p == '<' || *p == '>' || *p == '_'))
            {
                auto decode = [](char ch)
                {
                    switch (ch)
                    {
                        case '<': return Align::Mode::Left;
                        case '>': return Align::Mode::Right;
                        case '_': return Align::Mode::Center;
                    }
                    return Align::Mode::Left;
                };

                Align::Mode mode = decode(*p);
                ushort width = 0; ulong wlen = 0;
                char fill = ' ';
                p++;

                while (p < end && '0' <= *p && *p <= '9') 
                {
                    constexpr ushort USHORT_MAX = 0xffff;
                    ushort digit = (*p - '0');
                    
                    if (width > (USHORT_MAX - digit) / 10)
                        __throw::FormatIR_Alignment_Width_Too_Large();

                    width = width * 10 + digit;
                    p++;
                    wlen++;
                }

                if (wlen == 0)
                    __throw::FormatIR_Expecting_Alignment_Width();

                if (p < end && *p != ' ' && *p != '!' && *p != '@' && *p != '$') 
                {
                    fill = *p;
                    p++;
                }

                if (has_align)
                    __throw::FormatIR_Text_Alignment_Was_Specified_More_Than_Once();

                // TODO: assign align
                result.align = Align { width, mode, fill };
                has_align = true;
            }
            else if (p < end && *p == '!')
            {
                p++;
                Style flags = Style::Regular;
                byte count = 0;

                auto decode = [](char ch) -> Option<Style> 
                {
                    switch (ch)
                    {
                        case 'r': return { True{}, Style::Regular };
                        case 'b': return { True{}, Style::Bold };    
                        case 'i': return { True{}, Style::Italic };
                        case 'u': return { True{}, Style::Underline };
                        case 's': return { True{}, Style::Strike };
                        case 'd': return { True{}, Style::Dim };
                        default:  return { False{} };
                    }
                };

                while (p < end && *p != ' ' && *p != '<' && *p != '>' && *p != '_' && *p != '!' && *p != '@' && *p != '$')
                {
                    auto flag = decode(*p);

                    if (flag == False{})
                        __throw::FormatIR_Invalid_Style_Flags();

                    if (static_cast<byte>(flags & flag.unwrap()))
                        __throw::FormatIR_Duplicate_Style_Flags();

                    flags = flags | flag.unwrap();
                    count++;
                    p++;
                }

                if (count == 0)
                    __throw::FormatIR_Expecting_Style_Flags();

                // TODO: assign style
                if (has_style)
                    __throw::FormatIR_Style_Flags_Was_Specified_More_Than_Once();

                result.style = flags;
                has_style = true;
            }
            else if (p < end && (*p == '@' || *p == '$'))
            {
                bool is_background = (*p == '$');
                p++;

                if (p < end && *p == '(')
                {
                    p++;

                    struct {
                        byte value = 0;
                        ulong len = 0;
                    } channel[3] = {};

                    for (int i = 0; i < 3; i++)
                    {
                        skip_spaces();

                        while (p < end && '0' <= *p && *p <= '9')
                        {
                            byte digit = *p - '0';
                            constexpr byte byte_MAX = 255;

                            if (channel[i].value > (byte_MAX - digit) / 10)
                                __throw::FormatIR_RGB_Value_Exceeding_255();

                            channel[i].value = channel[i].value * 10 + digit;
                            channel[i].len++;
                            p++;
                        }

                        skip_spaces();

                        if (i == 0 || i == 1)
                        {
                            if (p >= end || *p != ',') __throw::FormatIR_Expected_3_Arguments_In_RGB();
                            p++;
                        }
                        else if (i == 2)
                        {
                            if (p >= end || *p != ')') __throw::FormatIR_Expected_Closing_Paren_In_RGB();
                            p++;
                        }

                        // check .len after ,  =>  suppess warning while user is typing
                        if (channel[i].len == 0)
                            __throw::FormatIR_Expecting_RGB_Value();
                    }

                    // assign color
                    if (!is_background)
                    {
                        if (result.text_color == True{})
                            __throw::FormatIR_Text_Color_Was_Specified_More_Than_Once();

                        result.text_color = 
                        {
                            True{}, 
                            Color8bit { 
                                channel[0].value, 
                                channel[1].value, 
                                channel[2].value 
                            }
                        };
                    }
                    else
                    {
                        if (result.background_color == True{})
                            __throw::FormatIR_Background_Color_Was_Specified_More_Than_Once();

                        result.background_color = 
                        {
                            True{}, 
                            Color8bit { 
                                channel[0].value, 
                                channel[1].value, 
                                channel[2].value 
                            } 
                        };
                    }
                }
                else if (end - p >= 7 && *p == '#')
                {
                    p++;

                    char const* checkpoint = p;
                    enum class LetterCase { Unknown, LowerCase, UpperCase } hex_type = LetterCase::Unknown;

                    for (int i = 0; i < 6; i++)
                    {
                        if (!('0' <= *p && *p <= '9' || 'a' <= *p && *p <= 'f' || 'A' <= *p && *p <= 'F'))
                            __throw::FormatIR_Expecting_RGB_Hex_Code();
                        
                        if (hex_type == LetterCase::Unknown)
                        {
                            if ('a' <= *p && *p <= 'f')
                                hex_type = LetterCase::LowerCase;
                            else if ('A' <= *p && *p <= 'F')
                                hex_type = LetterCase::UpperCase;
                        }
                        else if (hex_type == LetterCase::LowerCase)
                        {
                            if ('a' <= *p && *p <= 'f')
                                hex_type = LetterCase::LowerCase;
                            else if ('A' <= *p && *p <= 'F')
                                __throw::FormatIR_Expecting_Uniform_Letter_Case_For_RGB_Hex();
                        }
                        else if (hex_type == LetterCase::UpperCase)
                        {
                            if ('a' <= *p && *p <= 'f')
                                __throw::FormatIR_Expecting_Uniform_Letter_Case_For_RGB_Hex();
                            else if ('A' <= *p && *p <= 'F')
                                hex_type = LetterCase::UpperCase;
                        }
                        p++;
                    }

                    p = checkpoint;

                    byte value[6] = {};
                    for (int i = 0; i < 6; i++)
                    {
                        if ('0' <= *p && *p <= '9')
                            value[i] = *p - '0';
                        else if ('a' <= *p && *p <= 'f')
                            value[i] = 10 + *p - 'a';
                        else if ('A' <= *p && *p <= 'F')
                            value[i] = 10 + *p - 'A';
                        p++;
                    }

                    // assign color
                    if (!is_background)
                    {
                        if (result.text_color == True{})
                            __throw::FormatIR_Text_Color_Was_Specified_More_Than_Once();

                        result.text_color = 
                        {
                            True{}, 
                            Color8bit { 
                                static_cast<byte>((value[0] << 4) | value[1]),
                                static_cast<byte>((value[2] << 4) | value[3]),
                                static_cast<byte>((value[4] << 4) | value[5])
                            }
                        };
                    }
                    else
                    {
                        if (result.background_color == True{})
                            __throw::FormatIR_Background_Color_Was_Specified_More_Than_Once();
                        
                        result.background_color = 
                        {
                            True{}, 
                            Color8bit { 
                                static_cast<byte>((value[0] << 4) | value[1]),
                                static_cast<byte>((value[2] << 4) | value[3]),
                                static_cast<byte>((value[4] << 4) | value[5])
                            }
                        };
                    }
                }
                else if (p < end && ('a' <= *p && *p <= 'z' || 'A' <= *p && *p <= 'Z'))
                {
                    char const* name = p;
                    byte len = 0;

                    while (p < end && ('a' <= *p && *p <= 'z' || 'A' <= *p && *p <= 'Z'))
                    {
                        constexpr byte BYTE_MAX = 0xff;

                        if (len == BYTE_MAX)
                            __throw::FormatIR_Color_Name_Too_Long();

                        len++;
                        p++;
                    }

                    Color8bit color = {};
                    switch (operator""_hash(name, len))
                    {
                        // 🎨 Basic
                        case "Red"_hash:        case "red"_hash:        color = +Color::Red;        break;
                        case "Green"_hash:      case "green"_hash:      color = +Color::Green;      break;
                        case "Blue"_hash:       case "blue"_hash:       color = +Color::Blue;       break;
                        case "Yellow"_hash:     case "yellow"_hash:     color = +Color::Yellow;     break;
                        case "Cyan"_hash:       case "cyan"_hash:       color = +Color::Cyan;       break;
                        case "Magenta"_hash:    case "magenta"_hash:    color = +Color::Magenta;    break;
                        case "Black"_hash:      case "black"_hash:      color = +Color::Black;      break;
                        case "White"_hash:      case "white"_hash:      color = +Color::White;      break;

                        // 🎨 Bonus shades
                        case "Crimson"_hash:    case "crimson"_hash:    color = +Color::Crimson;    break;
                        case "Purple"_hash:     case "purple"_hash:     color = +Color::Purple;     break;
                        case "Pink"_hash:       case "pink"_hash:       color = +Color::Pink;       break;
                        case "Brown"_hash:      case "brown"_hash:      color = +Color::Brown;      break;
                        case "Orange"_hash:     case "orange"_hash:     color = +Color::Orange;     break;
                        case "Amber"_hash:      case "amber"_hash:      color = +Color::Amber;      break;
                        case "Chateu"_hash:     case "chateu"_hash:     color = +Color::Chateu;     break;
                        case "Cream"_hash:      case "cream"_hash:      color = +Color::Cream;      break;
                        case "Sky"_hash:        case "sky"_hash:        color = +Color::Sky;        break;
                        case "Violet"_hash:     case "violet"_hash:     color = +Color::Violet;     break;
                        case "Lime"_hash:       case "lime"_hash:       color = +Color::Lime;       break;
                        case "Teal"_hash:       case "teal"_hash:       color = +Color::Teal;       break;
                        case "Navy"_hash:       case "navy"_hash:       color = +Color::Navy;       break;
                        case "Gold"_hash:       case "gold"_hash:       color = +Color::Gold;       break;
                        case "Silver"_hash:     case "silver"_hash:     color = +Color::Silver;     break;
                        case "Deepsea"_hash:    case "deepsea"_hash:    color = +Color::Deepsea;    break;
                        case "Gray"_hash:       case "gray"_hash:       color = +Color::Gray;       break;
                        case "Claude"_hash:     case "claude"_hash:     color = +Color::Claude;     break;
                        case "Carnelian"_hash:  case "carnelian"_hash:  color = +Color::Carnelian;  break;
                        case "Pearl"_hash:      case "pearl"_hash:      color = +Color::Pearl;      break;

                        default: __throw::FormatIR_Color_Name_Not_Recored();
                    }

                    // assign color
                    if (!is_background)
                    {
                        if (result.text_color == True{})
                            __throw::FormatIR_Text_Color_Was_Specified_More_Than_Once();

                        result.text_color = { True{}, color};
                    }
                    else
                    {
                        if (result.background_color == True{})
                            __throw::FormatIR_Background_Color_Was_Specified_More_Than_Once();
                        
                        result.background_color = { True{}, color};
                    }
                }
                else
                {
                    __throw::FormatIR_Expecting_Color_Name_Or_RGB_Code();
                }
            }
            else
            {
                __throw::FormatIR_Unknown_Format_Option();
            }
        }

        return result;
    }

    template <int N>
    static consteval FormatIR generate(char const(&params)[N])
    {
        return generate(params, N - 1);
    }

private:

    template <typename Buffer>
    static constexpr bool has_raw_write = requires (Buffer& buffer, char const* chars, char ch, ulong size) { 
        { buffer.write(chars, size) } -> same_as<Result<void, FormatError>>; 
        { buffer.write(chars, size) } -> same_as<Result<void, FormatError>>; 
    };

public:

    template <typename Buffer>
    requires (has_raw_write<Buffer>)
    Result<void, FormatError> write_into(Buffer& buffer, char const* chars, ulong write_len)
    {
        #define GUARD(result) do { auto res = result; if (res == False{}) return res; } while (false)

        struct helper
        {
            static void format_into(char* prefix, byte value)
            {
                prefix[2] = '0' + (value % 10);
                prefix[1] = '0' + (value / 10) % 10;
                prefix[0] = '0' + (value / 100) % 10;
            }
        };

        bool styled = false;
        if (text_color == True{})
        {
            Color8bit color_vec = text_color.unwrap();
            char prefix[] = "\e[38;2;000;000;000m";
            helper::format_into(prefix + 7, color_vec.red);
            helper::format_into(prefix + 11, color_vec.green);
            helper::format_into(prefix + 15, color_vec.blue);
            GUARD(buffer.write(prefix, 19));
            styled = true;
        }

        if (background_color == True{})
        {
            Color8bit color_vec = background_color.unwrap();
            char prefix[] = "\e[48;2;000;000;000m";
            helper::format_into(prefix + 7, color_vec.red);
            helper::format_into(prefix + 11, color_vec.green);
            helper::format_into(prefix + 15, color_vec.blue);
            GUARD(buffer.write(prefix, 19));
            styled = true;
        }

        if (style != Style::Regular)
        {
            uint flags = +style;
            char prefix[] = "\e[_;_;_;_;_m";
            int i = 2;
            if (flags & +Style::Bold)      { prefix[i] = '1'; i += 2; }
            if (flags & +Style::Dim)       { prefix[i] = '2'; i += 2; }
            if (flags & +Style::Italic)    { prefix[i] = '3'; i += 2; }
            if (flags & +Style::Underline) { prefix[i] = '4'; i += 2; }
            if (flags & +Style::Strike)    { prefix[i] = '9'; i += 2; }
            prefix[i - 1] = 'm';
            GUARD(buffer.write(prefix, i));
            styled = true;
        }

        if (align.width > write_len)
        {
            ulong fill_len = align.width - write_len;
            switch (align.mode)
            {
                case Align::Mode::Left:
                {
                    GUARD(buffer.write(chars, write_len));
                    GUARD(buffer.write(align.fill, fill_len));
                    break;
                }
                case Align::Mode::Right:
                {
                    GUARD(buffer.write(align.fill, fill_len));
                    GUARD(buffer.write(chars, write_len));
                    break;
                }
                case Align::Mode::Center:
                {
                    GUARD(buffer.write(align.fill, fill_len / 2));
                    GUARD(buffer.write(chars, write_len));
                    GUARD(buffer.write(align.fill, fill_len - fill_len / 2));
                    break;
                }
            }
        }
        else
        {
            GUARD(buffer.write(chars, write_len));
        }

        if (styled)
            return buffer.write("\e[0m", 4);
        else
            return { True{} };

        #undef GUARD
    }

    template <typename Buffer>
    requires (has_raw_write<Buffer>)
    Result<void, FormatError> write_into(Buffer& buffer, char ch, ulong count)
    {
        #define GUARD(result) do { auto res = result; if (res == False{}) return res; } while (false)

        struct helper
        {
            static void format_into(char* prefix, byte value)
            {
                prefix[2] = '0' + (value % 10);
                prefix[1] = '0' + (value / 10) % 10;
                prefix[0] = '0' + (value / 100) % 10;
            }
        };

        bool styled = false;
        if (text_color == True{})
        {
            Color8bit color_vec = text_color.unwrap();
            char prefix[] = "\e[38;2;000;000;000m";
            helper::format_into(prefix + 7, color_vec.red);
            helper::format_into(prefix + 11, color_vec.green);
            helper::format_into(prefix + 15, color_vec.blue);
            GUARD(buffer.write(prefix, 19));
            styled = true;
        }

        if (background_color == True{})
        {
            Color8bit color_vec = background_color.unwrap();
            char prefix[] = "\e[48;2;000;000;000m";
            helper::format_into(prefix + 7, color_vec.red);
            helper::format_into(prefix + 11, color_vec.green);
            helper::format_into(prefix + 15, color_vec.blue);
            GUARD(buffer.write(prefix, 19));
            styled = true;
        }

        if (style != Style::Regular)
        {
            uint flags = +style;
            char prefix[] = "\e[_;_;_;_;_m";
            int i = 2;
            if (flags & +Style::Bold)      { prefix[i] = '1'; i += 2; }
            if (flags & +Style::Dim)       { prefix[i] = '2'; i += 2; }
            if (flags & +Style::Italic)    { prefix[i] = '3'; i += 2; }
            if (flags & +Style::Underline) { prefix[i] = '4'; i += 2; }
            if (flags & +Style::Strike)    { prefix[i] = '9'; i += 2; }
            prefix[i - 1] = 'm';
            GUARD(buffer.write(prefix, i));
            styled = true;
        }

        if (align.width > count)
        {
            ulong fill_len = align.width - count;
            switch (align.mode)
            {
                case Align::Mode::Left:
                {
                    GUARD(buffer.write(ch, count));
                    GUARD(buffer.write(align.fill, fill_len));
                    break;
                }
                case Align::Mode::Right:
                {
                    GUARD(buffer.write(align.fill, fill_len));
                    GUARD(buffer.write(ch, count));
                    break;
                }
                case Align::Mode::Center:
                {
                    GUARD(buffer.write(align.fill, fill_len / 2));
                    GUARD(buffer.write(ch, count));
                    GUARD(buffer.write(align.fill, fill_len - fill_len / 2));
                    break;
                }
            }
        }
        else
        {
            GUARD(buffer.write(ch, count));
        }

        if (styled)
            return buffer.write("\e[0m", 4);
        else
            return { True{} };

        #undef GUARD
    }
};

_MNC_END
consteval mnc::FormatIR operator""_fmt(char const* _, mnc::ulong __) {
    return mnc::FormatIR::generate(_, __);
}
_MNC_BEGIN

#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

_MNC_END

#undef _MNC_BEGIN
#undef _MNC_END