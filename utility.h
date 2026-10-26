#pragma once
#include <cstdio>
#include <cstdlib>
#include "type_traits.h"
#include "compare.h"


#define _MNC_BEGIN namespace mnc {
#define _MNC_END }

_MNC_BEGIN

/////////////////////////////////////////////////////////////////////////////
//////////////////////////////// fn: min, max ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region min-max

template <typename T>
requires (has_ParOrder<T>)
constexpr T min(T left, T right)
{
    struct __throw 
    {
        [[noreturn]] static void FN_MIN_Comparison_Didnt_Return_Order()
        {
            std::fprintf(stderr, "[fn mnc::min] ;; ABORT WARNING ;; comparison didnt return order\n");
            std::fflush(stderr);
            std::abort();
        }
    };

    switch (Comparator<T>::compare(left, right))
    {
        case ParOrder::Less:    return left;
        case ParOrder::Equal:
        case ParOrder::Greater: return right;
        case ParOrder::UnEqual: __throw::FN_MIN_Comparison_Didnt_Return_Order();
    }
}

template <typename T>
requires (has_ParOrder<T>)
constexpr T max(T left, T right)
{
    struct __throw 
    {
        [[noreturn]] static void FN_MAX_Comparison_Didnt_Return_Order()
        {
            std::fprintf(stderr, "[fn mnc::max] ;; ABORT WARNING ;; comparison didnt return order\n");
            std::fflush(stderr);
            std::abort();
        }
    };

    switch (Comparator<T>::compare(left, right))
    {
        case ParOrder::Greater: return left;
        case ParOrder::Equal:
        case ParOrder::Less:    return right;
        case ParOrder::UnEqual: __throw::FN_MAX_Comparison_Didnt_Return_Order();
    }
}

/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// fn: swap ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region SWAP

template <typename T>
requires (is_move_able<T> || is_copy_able<T>)
constexpr void swap(T& left, T& right)
{
    if constexpr (is_move_able<T>)
    {
        T temp = mov(left);
        left = mov(right);
        right = mov(temp);
    }
    else if constexpr (is_copy_able<T>)
    {
        T temp = left;
        left = right;
        right = temp;
    }
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////// CONDITIONAL CASTS /////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region CASTS

template <typename T>
constexpr T const& make_const(T& val)
{
    return static_cast<T const&&>(val);
}

template <typename T>
constexpr T const&& make_const(T&& val)
{
    return static_cast<T const&&>(val);
}

template <typename T>
constexpr remove_reference<T>&& mov(T& obj)
{
    return static_cast<remove_reference<T>&&>(obj);
}

template <typename T>
constexpr T&& forward(remove_reference<T>& arg) //always takes lvalue
{
    return static_cast<T&&>(arg);
}

template <typename T>
constexpr T&& forward(remove_reference<T>&& arg) //always takes rvalue
{
    static_assert(!is_lvalue_reference<T>, "trying to move while forwarding");
    return static_cast<T&&>(arg);
}

/////////////////////////////////////////////////////////////////////////////
//////////////////////////// fn: operator""_hash ////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region HASH

constexpr ulong operator""_hash(char const* s, ulong len)
{
    ulong hash = 14695981039346656037ULL;
    for (ulong i = 0; i < len; ++i) 
    {
        char c = s[i];
        //if ('A' <= c && c <= 'Z') c += ('a' - 'A');
        hash ^= static_cast<ulong>(c);
        hash *= 1099511628211ULL;
    }
    return hash;
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////// class: Pair ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////

template <typename A, typename B>
struct Pair {
    A first;
    B second;
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////// fn: unreachable /////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region HASH

[[noreturn]] inline void unreachable(char const* file, int line, char const* msg)
{
    std::fprintf(stderr, "%s:%d: [ABORT WARNING] %s", file, line, msg);
    std::fflush(stderr);

    #if defined(_MSC_VER) && !defined(__clang__)
        __assume(false);
    #elif defined(__clang__) || defined(__GNUC__)
        __builtin_trap();
    #else
        std::abort();
    #endif

    using _0 = decltype(std::abort());
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

_MNC_END

#undef _MNC_BEGIN
#undef _MNC_END