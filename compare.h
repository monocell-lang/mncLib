#pragma once
#include "type_traits.h"


#define _MNC_BEGIN namespace mnc {
#define _MNC_END }

_MNC_BEGIN

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region basic traits

template <typename T>
static constexpr bool has_equal_operators = requires (T const& left, T const& right) {
    requires (same_type<decltype(left == right), bool>);
    requires (same_type<decltype(left != right), bool>);
};

template <typename T>
static constexpr bool has_order_operators = requires (T const& left, T const& right) {
    requires (same_type<decltype(left == right), bool>);
    requires (same_type<decltype(left != right), bool>);
    requires (same_type<decltype(left < right), bool>);
    requires (same_type<decltype(left > right), bool>);
    requires (same_type<decltype(left <= right), bool>);
    requires (same_type<decltype(left >= right), bool>);
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region Order

enum class Ordering: byte {
    Order,      // ==, !=, <, >, <=, >=
    ParOrder,   // ==, !=, <, >, <=, >=, sentinel
    Equal,      // ==, !=
    ParEqual,   // ==, !=, sentinel
};

constexpr Ordering operator+(Ordering left, Ordering right)
{
    /*
    *   |================|============|============|===========|============|
    *   | Convertibility |    Order   |  ParOrder  |   Equal   |  ParEqual  |
    *   |================|============|============|===========|============|
    *   |     Order      |     \\     |    yes     |    yes    |    yes     |
    *   |----------------|------------|------------|-----------|------------|
    *   |    ParOrder    |            |     \\     |           |    yes     |
    *   |----------------|------------|------------|-----------|------------|
    *   |     Equal      |            |            |     \\    |    yes     |
    *   |----------------|------------|------------|-----------|------------|
    *   |    ParEqual    |            |            |           |     \\     |
    *   |----------------|------------|------------|-----------|------------|
    */

    /*
    *   |================|============|============|===========|============|
    *   |    Addition    |    Order   |  ParOrder  |   Equal   |  ParEqual  |
    *   |================|============|============|===========|============|
    *   |     Order      |    Order   |  ParOrder  |   Equal   |  ParEqual  |
    *   |----------------|------------|------------|-----------|------------|
    *   |    ParOrder    |  ParOrder  |  ParOrder  |  ParEqual |  ParEqual  |
    *   |----------------|------------|------------|-----------|------------|
    *   |     Equal      |    Equal   |  ParEqual  |   Equal   |  ParEqual  |
    *   |----------------|------------|------------|-----------|------------|
    *   |    ParEqual    |  ParEqual  |  ParEqual  |  ParEqual |  ParEqual  |
    *   |----------------|------------|------------|-----------|------------|
    */

    /*
    *   WeakOrder(a, b)    = Order(norm(a), norm(b))
    *   WeakParOrder(a, b) = ParOrder(norm(a), norm(b))
    *   WeakEqual(a, b)    = Equal(norm(a), norm(b))
    *   WeakParEqual(a, b) = ParEqual(norm(a), norm(b))
    *
    *   norm: T -> subset of T
    *   ((a <= b) -> (norm(a) <= norm(b))) xor ((a <= b) -> (norm(a) >= norm(b))  (-> is implication)
    *   no circuits in graph
    *
    *   Example:
    *   any Ace > any King > any Queen > any Jack > 9 > 8 > ... > 1 > 0
    *   Ace♠ != Ace♥ != Ace♦ != Ace♣
    *
    *   norm(card):
    *   | Ace♠ => Ace♠
    *   | Ace♥ => Ace♠
    *   | Ace♦ => Ace♠
    *   | Ace♣ => Ace♠  ...
    */
    
    auto _left = static_cast<byte>(left);
    auto _right = static_cast<byte>(right);

    if (_left >= 4 || _right >= 4)
        return static_cast<Ordering>(4);
    
    constexpr Ordering map[4][4] = 
    { 
        { Ordering::Order,    Ordering::ParOrder, Ordering::Equal,    Ordering::ParEqual },
        { Ordering::ParOrder, Ordering::ParOrder, Ordering::ParEqual, Ordering::ParEqual },
        { Ordering::Equal,    Ordering::ParEqual, Ordering::Equal,    Ordering::ParEqual },
        { Ordering::ParEqual, Ordering::ParEqual, Ordering::ParEqual, Ordering::ParEqual }
    };

    return map[_left][_right];
}

enum class Order: byte {
    Equal,    //  ==, <=, >=
    Less,     //  <, <=, !=
    Greater   //  >, >=, !=
};

enum class ParOrder: byte {
    Equal,    //  ==, <=, >=
    Less,     //  <, <=, !=
    Greater,  //  >, >=, !=
    UnEqual   //  !=
};

enum class Equal: byte {
    Equal,    //  ==
    UnEqual   //  !=
};

enum class ParEqual: byte {
    Equal,    //  ==
    UnEqual   //  !=
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region Comparator

template <typename T>
struct struct_orderingof;

template <typename T>
static constexpr Ordering orderingof = struct_orderingof<T>::value;

template <typename T, Ordering Trait = orderingof<T>>
class Comparator;

template <typename T>
static constexpr bool has_Order = requires (T const& left, T const& right) {
    requires (same_type<decltype(Comparator<T, Ordering::Order>::compare(left, right)), Order>);
};

template <typename T>
static constexpr bool has_ParOrder = requires (T const& left, T const& right) {
    requires (same_type<decltype(Comparator<T, Ordering::ParOrder>::compare(left, right)), ParOrder>);
};

template <typename T>
static constexpr bool has_Equal = requires (T const& left, T const& right) {
    requires (same_type<decltype(Comparator<T, Ordering::Equal>::compare(left, right)), Equal>);
};

template <typename T>
static constexpr bool has_ParEqual = requires (T const& left, T const& right) {
    requires (same_type<decltype(Comparator<T, Ordering::ParEqual>::compare(left, right)), ParEqual>);
};

template <typename T>
struct struct_orderingof {
private:

    static constexpr Ordering compute()
    {
        if (has_Order<T>)
            return Ordering::Order;
        else if (has_ParOrder<T>)
            return Ordering::ParOrder;
        else if (has_Equal<T>)
            return Ordering::Equal;
        else if (has_ParEqual<T>)
            return Ordering::ParEqual;
        else
            return static_cast<Ordering>(5);
    }

public:

    static constexpr Ordering value = compute();
};

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
    same_type<T, unsigned long long> ||
    same_type<T, bool>               ||
    same_type<T, char>
)
class Comparator<T, Ordering::Order> {
public:

    static constexpr Order compare(T const& left, T const& right)
    {
        if (left < right)
            return Order::Less;
        else if (left > right)
            return Order::Greater;
        else
            return Order::Equal;
    }
};

template <typename T>
requires (
    same_type<T, float>       ||
    same_type<T, double>      ||
    same_type<T, long double>
)
class Comparator<T, Ordering::ParOrder> {
public:

    static constexpr ParOrder compare(T const& left, T const& right)
    {
        if (left < right)
            return ParOrder::Less;
        else if (left > right)
            return ParOrder::Greater;
        else if (left == right)
            return ParOrder::Equal;
        else
            return ParOrder::UnEqual;
    }
};

template <typename T>
class Comparator<T*, Ordering::Order> {
public:

    static constexpr Order compare(T* const& left, T* const& right)
    {
        if (left < right)
            return Order::Less;
        else if (left > right)
            return Order::Greater;
        else
            return Order::Equal;
    }
};

template <typename T>
requires (is_enum<T> && !is_const<T> && !is_volatile<T> && !has_Order<T>)
class Comparator<T, Ordering::Equal> {
public:

    static constexpr Equal compare(T const& left, T const& right)
    {
        if (left == right)
            return Equal::Equal;
        else
            return Equal::UnEqual;
    }
};


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region trait conversion

template <typename T>
requires (has_Order<T>)
class Comparator<T, Ordering::ParOrder> {
public:

    static constexpr ParOrder compare(T const& left, T const& right)
    {
        switch (Comparator<T, Ordering::Order>::compare(left, right))
        {
            case Order::Equal:   return ParOrder::Equal;
            case Order::Less:    return ParOrder::Less;
            case Order::Greater: return ParOrder::Greater;
            default:             return ParOrder::UnEqual;
        }
    }
};

template <typename T>
requires (has_Order<T>)
class Comparator<T, Ordering::Equal> {
public:

    static constexpr Equal compare(T const& left, T const& right)
    {
        switch (Comparator<T, Ordering::Order>::compare(left, right))
        {
            case Order::Equal:   return Equal::Equal;
            case Order::Less:    return Equal::UnEqual;
            case Order::Greater: return Equal::UnEqual;
            default:             return Equal::UnEqual;
        }
    }
};

template <typename T>
requires (has_Order<T>)
class Comparator<T, Ordering::ParEqual> {
public:

    static constexpr ParEqual compare(T const& left, T const& right)
    {
        switch (Comparator<T, Ordering::Order>::compare(left, right))
        {
            case Order::Equal:   return ParEqual::Equal;
            case Order::Less:    return ParEqual::UnEqual;
            case Order::Greater: return ParEqual::UnEqual;
            default:             return ParEqual::UnEqual;
        }
    }
};

template <typename T>
requires (has_ParOrder<T> && !has_Order<T>)
class Comparator<T, Ordering::ParEqual> {
public:

    static constexpr ParEqual compare(T const& left, T const& right)
    {
        switch (Comparator<T, Ordering::ParOrder>::compare(left, right))
        {
            case ParOrder::Equal:   return ParEqual::Equal;
            case ParOrder::Less:    return ParEqual::UnEqual;
            case ParOrder::Greater: return ParEqual::UnEqual;
            case ParOrder::UnEqual: return ParEqual::UnEqual;
            default:                return ParEqual::UnEqual;
        }
    }
};

template <typename T>
requires (has_Equal<T> && !has_ParOrder<T>&& !has_Order<T>)
class Comparator<T, Ordering::ParEqual> {
public:

    static constexpr ParEqual compare(T const& left, T const& right)
    {
        Equal value = Comparator<T, Ordering::Equal>::compare(left, right);
        switch (value)
        {
            case Equal::Equal:   return ParEqual::Equal;
            case Equal::UnEqual: return ParEqual::UnEqual;
            default:             return ParEqual::UnEqual;
        }
    }
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region queries

template <Ordering>
struct struct_order_type {
    using type = NAUR;
};

template <>
struct struct_order_type<Ordering::Order> {
    using type = Order;
};

template <>
struct struct_order_type<Ordering::ParOrder> {
    using type = ParOrder;
};

template <>
struct struct_order_type<Ordering::Equal> {
    using type = Equal;
};

template <>
struct struct_order_type<Ordering::ParEqual> {
    using type = ParEqual;
};

template <Ordering ordering>
using order_type = typename struct_order_type<ordering>::type;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

_MNC_END

#undef _MNC_BEGIN
#undef _MNC_END