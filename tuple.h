#pragma once
#include "type_traits.h"
#include "compare.h"


#define _MNC_BEGIN namespace mnc {
#define _MNC_END }

_MNC_BEGIN

/////////////////////////////////////////////////////////////////////////////
//////////////////////////////// class: Tuple ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region tuple

template <typename... Ts>
class Tuple {
private:

    template <typename... Tys>
    friend class Tuple;

    friend class Comparator<Tuple<Ts...>, (orderingof<remove_const_reference<Ts>> + ... + Ordering::Order)>;

    /////////////////////////////////////////////////////////////////////////////
    //////////////////////// stage 1: alignment sorting /////////////////////////
    /////////////////////////////////////////////////////////////////////////////

    struct Array {
        int forward[sizeof...(Ts)];
        int backward[sizeof...(Ts)];
    };

    template <typename T>
    struct Measure { T data; };

    static constexpr Array compute()
    {
        struct Pair { 
            int index; 
            int align; 
        };

        int index = 0;
        Pair arr[sizeof...(Ts)] = { Pair { index++, alignof(Measure<Ts>) } ... };

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

        Array result = {};
        for (int i = 0; i < sizeof...(Ts); i++)
        {
            result.forward[i] = arr[i].index;
            result.backward[arr[i].index] = i;
        }

        return result;
    }

    static constexpr Array map = compute();

    /////////////////////////////////////////////////////////////////////////////
    /////////////////////////////// stage 2: main ///////////////////////////////
    /////////////////////////////////////////////////////////////////////////////

    template <int _Index, typename T>
    struct TupleCell { T data; };

    template <int _Index>  // logical index of types
    using Cell = TupleCell<map.backward[_Index], type_at<_Index, Ts...>>;

    template <typename index_sequence>
    struct _Impl;

    template <int... _Indices>
    struct _Impl<index_sequence<_Indices...>>  // physical index of types
    : public TupleCell<_Indices, type_at<map.forward[_Indices], Ts...>>... {
    public:

        template <typename... Tys>
        constexpr _Impl(Tys&&... elem)  // logical index of types
        requires (sizeof...(Tys) == sizeof...(Ts))
        : TupleCell<map.backward[_Indices], Ts> { static_cast<Tys&&>(elem) } ... {}

        constexpr _Impl(_Impl const& other)
        requires (is_trivially_copy_constructible<Ts> && ...) = default;

        constexpr _Impl(_Impl const& other)
        requires ((is_copy_constructible<Ts> && ...) && (!is_trivially_copy_constructible<Ts> || ...))
        : TupleCell<map.backward[_Indices], Ts> { static_cast<Cell<_Indices> const&>(other).data } ... {}

        constexpr _Impl(_Impl&& other)
        requires (is_trivially_move_constructible<Ts> && ...) = default;
        
        constexpr _Impl(_Impl&& other)
        requires ((is_move_constructible<Ts> && ...) && (!is_trivially_move_constructible<Ts> || ...))
        : TupleCell<map.backward[_Indices], Ts> { static_cast<Cell<_Indices>&&>(other).data } ... {}

        constexpr _Impl& operator=(_Impl const&)
        requires (is_trivially_copy_assignable<Ts> && ...) = default;

        constexpr _Impl& operator=(_Impl const& other)
        requires ((is_copy_assignable<Ts> && ...) && (!is_trivially_copy_assignable<Ts> || ...))
        {
            if (this == &other) return *this;
            ((static_cast<Cell<_Indices>&>(*this).data = static_cast<Cell<_Indices> const&>(other).data), ...);
            return *this;
        }

        constexpr _Impl& operator=(_Impl&&)
        requires (is_trivially_move_assignable<Ts> && ...) = default;

        constexpr _Impl& operator=(_Impl&& other)
        requires ((is_move_assignable<Ts> && ...) && (!is_trivially_move_assignable<Ts> || ...))
        {
            if (this == &other) return *this;
            ((static_cast<Cell<_Indices>&>(*this).data = static_cast<Cell<_Indices>&&>(other).data), ...);
            return *this;
        }

        ~_Impl() = default;
    };

    /////////////////////////////////////////////////////////////////////////////
    /////////////////////////// stage 3: homogeneous tuple //////////////////////
    /////////////////////////////////////////////////////////////////////////////

    template <typename T>
    struct HomoCell { T data; };

    using First = type_at<0, Ts...>;
    static constexpr bool array_condition = (sizeof...(Ts) > 0) && (same_type<Ts...>);

    template <int... _Indices>
    requires (array_condition)
    struct _Impl<index_sequence<_Indices...>> {
    public:

        HomoCell<First> array[sizeof...(Ts)];

    public:

        template <typename... Tys>
        constexpr _Impl(Tys&&... elem)
        requires (sizeof...(Tys) == sizeof...(Ts))
        : array { { static_cast<Tys&&>(elem) } ... } {}

        constexpr _Impl(_Impl const&)
        requires (is_trivially_copy_constructible<First>) = default;

        constexpr _Impl(_Impl const& other)
        requires (is_copy_constructible<First> && !is_trivially_copy_constructible<First>)
        : array { { other.array[_Indices].data } ... } {}

        constexpr _Impl(_Impl&&)
        requires (is_trivially_move_constructible<First>) = default;

        constexpr _Impl(_Impl&& other)
        requires (is_move_constructible<First> && !is_trivially_move_constructible<First>)
        : array { { static_cast<_Impl&&>(other).array[_Indices].data } ... } {}

        constexpr _Impl& operator=(_Impl const&)
        requires (is_trivially_copy_assignable<First>) = default;

        constexpr _Impl& operator=(_Impl const& other)
        requires (is_copy_assignable<First> && !is_trivially_copy_assignable<First>)
        {
            if (this == &other) return *this;
            ((array[_Indices].data = other.array[_Indices].data), ...);
            return *this;
        }

        constexpr _Impl& operator=(_Impl&&)
        requires (is_trivially_move_assignable<First>) = default;

        constexpr _Impl& operator=(_Impl&& other)
        requires (is_move_assignable<First> && !is_trivially_move_assignable<First>)
        {
            if (this == &other) return *this;
            ((array[_Indices].data = static_cast<_Impl&&>(other).array[_Indices].data), ...);
            return *this;
        }

        ~_Impl() = default;
    };

    /////////////////////////////////////////////////////////////////////////////
    //////////////////////////// stage 4: public wraper /////////////////////////
    /////////////////////////////////////////////////////////////////////////////

private:

    _Impl<index_sequence_init<sizeof...(Ts)>> data;

public:

    template <typename... Tys>
    constexpr Tuple(Tys&&... elem)
    requires (sizeof...(Tys) == sizeof...(Ts))
    : data { static_cast<Tys&&>(elem)... } {}

    constexpr Tuple(Tuple const& other)
    requires (is_trivially_copy_constructible<Ts> && ...) = default;

    constexpr Tuple(Tuple const& other)
    requires ((is_copy_constructible<Ts> && ...) && (!is_trivially_copy_constructible<Ts> || ...))
    : data(other.data) {}

    constexpr Tuple(Tuple&& other)
    requires (is_trivially_move_constructible<Ts> && ...) = default;
    
    constexpr Tuple(Tuple&& other)
    requires ((is_move_constructible<Ts> && ...) && (!is_trivially_move_constructible<Ts> || ...))
    : data(static_cast<Tuple&&>(other).data) {}

    constexpr Tuple& operator=(Tuple const&)
    requires (is_trivially_copy_assignable<Ts> && ...) = default;

    constexpr Tuple& operator=(Tuple const& other)
    requires ((is_copy_assignable<Ts> && ...) && (!is_trivially_copy_assignable<Ts> || ...))
    {
        if (this == &other) return *this;
        data = other.data;
        return *this;
    }

    constexpr Tuple& operator=(Tuple&&)
    requires (is_trivially_move_assignable<Ts> && ...) = default;

    constexpr Tuple& operator=(Tuple&& other)
    requires ((is_move_assignable<Ts> && ...) && (!is_trivially_move_assignable<Ts> || ...))
    {
        if (this == &other) return *this;
        data = static_cast<Tuple&&>(other).data;
        return *this;
    }

    ~Tuple() = default;

public:

    template <int _Index>  // logical index of types
    constexpr decltype(auto) get() &
    {
        if constexpr (array_condition)
            return (data.array[_Index].data);
        else
            return (static_cast<Cell<_Index>&>(data).data);
    }

    template <int _Index>
    constexpr decltype(auto) get() const&
    {
        if constexpr (array_condition)
            return (data.array[_Index].data);
        else
            return (static_cast<Cell<_Index> const&>(data).data);
    }

    template <int _Index>
    constexpr decltype(auto) get() &&
    {
        if constexpr (array_condition)
            return (static_cast<HomoCell<First>&&>(data.array[_Index]).data);
        else
            return (static_cast<Cell<_Index>&&>(data).data);
    }

    template <int _Index>
    constexpr decltype(auto) get() const&&
    {
        if constexpr (array_condition)
            return (static_cast<HomoCell<First> const&&>(data.array[_Index]).data);
        else
            return (static_cast<Cell<_Index> const&&>(data).data);
    }

    constexpr decltype(auto) operator[](int _Index) &
    requires (array_condition)
    {
        return (data.array[_Index].data);
    }

    constexpr decltype(auto) operator[](int _Index) const&
    requires (array_condition)
    {
        return (data.array[_Index].data);
    }

    constexpr decltype(auto) operator[](int _Index) &&
    requires (array_condition)
    {
        return (static_cast<HomoCell<First>&&>(data.array[_Index]).data);
    }

    constexpr decltype(auto) operator[](int _Index) const&&
    requires (array_condition)
    {
        return (static_cast<HomoCell<First> const&&>(data.array[_Index]).data);
    }

    /////////////////////////////////////////////////////////////////////////////
    ///////////////////////// stage 5: overload operator= ///////////////////////
    /////////////////////////////////////////////////////////////////////////////

private:

    static constexpr auto this_sequence = index_sequence_init<sizeof...(Ts)>{};

    template <typename Self, typename Other, int... _Index>
    static constexpr Self& assign(Self& self, Other&& other, index_sequence<_Index...>)
    {
        if ((void*)&self == (void*)&other) return self;
        ((self.template get<_Index>() = static_cast<Other&&>(other).template get<_Index>()), ...);
        return self;
    }

public:

    template <typename... Tys>
    requires (
        (sizeof...(Ts) == sizeof...(Tys)) &&
        (!same_type<Ts, Tys> || ...) &&    // disabiguation
        (is_assignable_from<Ts, Tys const&> && ...)
    )
    constexpr Tuple& operator=(Tuple<Tys...> const& other)
    {
        return assign(*this, other, this_sequence);
    }

    template <typename... Tys>
    requires (
        (sizeof...(Ts) == sizeof...(Tys)) &&
        (!same_type<Ts const, Tys> || ...) &&   // disabiguation
        (is_assignable_from<Ts const, Tys const&> && ...)
    )
    constexpr Tuple& operator=(Tuple<Tys...> const& other) const
    {
        return assign(*this, other, this_sequence);
    }

    template <typename... Tys>
    requires (
        (sizeof...(Ts) == sizeof...(Tys)) &&
        (!same_type<Ts, Tys> || ...) &&   // disabiguation
        (is_assignable_from<Ts, Tys&&> && ...)
    )
    constexpr Tuple& operator=(Tuple<Tys...>&& other)
    {
        return assign(*this, static_cast<Tuple<Tys...>&&>(other), this_sequence);
    }

    template <typename... Tys>
    requires (
        (sizeof...(Ts) == sizeof...(Tys)) &&
        (!same_type<Ts const, Tys> || ...) &&   // disabiguation
        (is_assignable_from<Ts const, Tys&&> && ...)
    )
    constexpr Tuple& operator=(Tuple<Tys...>&& other) const
    {
        return assign(*this, static_cast<Tuple<Tys...>&&>(other), this_sequence);
    }

    /////////////////////////////////////////////////////////////////////////////
    //////////////////////// stage 6: overload comparison ///////////////////////
    /////////////////////////////////////////////////////////////////////////////

private:

    static constexpr Ordering this_ordering = (orderingof<remove_const_reference<Ts>> + ... + Ordering::Order);
    using This_Order = OrderType<this_ordering>;

    template <typename Other, int... _Index>
    static constexpr This_Order compare(Tuple const& left, Other const& right, index_sequence<_Index...>)
    {
        /*
        *   Equal, Equal, Less, ...    = Less
        *   Equal, Equal, Greater, ... = Greater
        *   Equal, Equal, UnEqual, ... = UnEqual
        *   Equal, Equal,  ..., Equal  = Equal
        *   Equal, Less, UnEqual, ...  = UnEqual
        */

        This_Order result = This_Order::Equal;

        // unroll
        (
            (result = [](This_Order result, Tuple const& left, Other const& right) 
            {
                if constexpr (this_ordering != Ordering::ParOrder)
                {
                    if (result != This_Order::Equal)
                        return result;

                    return Comparator<remove_const_reference<Ts>, this_ordering>::compare(
                        left.template get<_Index>(),
                        right.template get<_Index>()
                    );
                }
                else
                {
                    switch (result)
                    {
                        case ParOrder::Equal:
                        {
                            return Comparator<remove_const_reference<Ts>, this_ordering>::compare(
                                left.template get<_Index>(),
                                right.template get<_Index>()
                            );
                        }

                        case ParOrder::Greater: 
                        [[fallthrough]];
                        
                        case ParOrder::Less:
                        {
                            ParOrder cmp = Comparator<remove_const_reference<Ts>, this_ordering>::compare(
                                left.template get<_Index>(),
                                right.template get<_Index>()
                            );

                            if (cmp == ParOrder::UnEqual)
                                return cmp;
                            else
                                return result;
                        }

                        default:
                        [[fallthrough]];
                        
                        case ParOrder::UnEqual: 
                        {
                            return result;
                        }
                    }
                }
            }
            (result, left, right)), ...
        );

        return result;
    }

    template <typename T1, typename T2>
    struct struct_equivalent_type {
        static constexpr bool value = same_type<remove_const_reference<T1>, remove_const_reference<T2>>;
    };

    template <typename... Ts1, typename... Ts2>
    struct struct_equivalent_type<Tuple<Ts1...>, Tuple<Ts2...>> {
        static constexpr bool value = (struct_equivalent_type<Ts1, Ts2>::value && ...);
    };

    template <typename T1, typename T2>
    static constexpr bool equivalent_type = struct_equivalent_type<T1, T2>::value;

public:

    template <typename... Tys>
    requires (
        (sizeof...(Ts) == sizeof...(Tys)) &&
        (equivalent_type<remove_const_reference<Ts>, remove_const_reference<Tys>> && ...) &&
        (has_ParEqual<remove_const_reference<Ts>> && ...)
    )
    constexpr bool operator==(Tuple<Tys...> const& other) const
    {
        return compare(*this, other, this_sequence) == This_Order::Equal;
    }

    template <typename... Tys>
    requires (
        (sizeof...(Ts) == sizeof...(Tys)) &&
        (equivalent_type<remove_const_reference<Ts>, remove_const_reference<Tys>> && ...) &&
        (has_ParEqual<remove_const_reference<Ts>> && ...)
    )
    constexpr bool operator!=(Tuple<Tys...> const& other) const
    {
        return compare(*this, other, this_sequence) != This_Order::Equal;
    }

    template <typename... Tys>
    requires (
        (sizeof...(Ts) == sizeof...(Tys)) && 
        (equivalent_type<remove_const_reference<Ts>, remove_const_reference<Tys>> && ...) &&
        (has_ParOrder<remove_const_reference<Ts>> && ...)
    )
    constexpr bool operator<(Tuple<Tys...> const& other) const
    {
        return compare(*this, other, this_sequence) == This_Order::Less;
    }

    template <typename... Tys>
    requires (
        (sizeof...(Ts) == sizeof...(Tys)) && 
        (equivalent_type<remove_const_reference<Ts>, remove_const_reference<Tys>> && ...) &&
        (has_ParOrder<remove_const_reference<Ts>> && ...)
    )
    constexpr bool operator>(Tuple<Tys...> const& other) const
    {
        return compare(*this, other, this_sequence) == This_Order::Greater;
    }

    template <typename... Tys>
    requires (
        (sizeof...(Ts) == sizeof...(Tys)) && 
        (equivalent_type<remove_const_reference<Ts>, remove_const_reference<Tys>> && ...) &&
        (has_ParOrder<remove_const_reference<Ts>> && ...)
    )
    constexpr bool operator<=(Tuple<Tys...> const& other) const
    {
        auto value = compare(*this, other, this_sequence);
        return (value == This_Order::Less) || (value == This_Order::Equal);
    }

    template <typename... Tys>
    requires (
        (sizeof...(Ts) == sizeof...(Tys)) && 
        (equivalent_type<remove_const_reference<Ts>, remove_const_reference<Tys>> && ...) &&
        (has_ParOrder<remove_const_reference<Ts>> && ...)
    )
    constexpr bool operator>=(Tuple<Tys...> const& other) const
    {
        auto value = compare(*this, other, this_sequence);
        return (value == This_Order::Greater) || (value == This_Order::Equal);
    }
};


struct tuple
{
    template <typename... Ts>
    static constexpr auto make(Ts&&... elems) {
        using Tuple = Tuple<remove_const_volatile<remove_reference<Ts>>...>;
        return Tuple { static_cast<Ts&&>(elems)... };
    }

    template <typename... Ts>
    static constexpr Tuple<Ts&...> tie(Ts&... elems) {
        return { elems... };
    }

    template <typename... Ts>
    static constexpr Tuple<Ts&&...> forward(Ts&&... elems) {
        return { static_cast<Ts&&>(elems)... };
    }

public:

    template <int _Index, typename... Ts>
    static constexpr decltype(auto) get(Tuple<Ts...>& self) {
        return self.template get<_Index>();
    }

    template <int _Index, typename... Ts>
    static constexpr decltype(auto) get(Tuple<Ts...> const& self) {
        return self.template get<_Index>();
    }

    template <int _Index, typename... Ts>
    static constexpr decltype(auto) get(Tuple<Ts...>&& self) {
        return static_cast<Tuple<Ts...>&&>(self).template get<_Index>();
    }

    template <int _Index, typename... Ts>
    static constexpr decltype(auto) get(Tuple<Ts...> const&& self) {
        return static_cast<Tuple<Ts...> const&&>(self).template get<_Index>();
    }

public:

    template <typename Fn, typename... Ts>
    requires (is_callable<Fn&&, _(Ts&...)>)
    static constexpr decltype(auto) apply(Fn&& fn, Tuple<Ts...>& self) {
        return apply(fn, self, index_sequence_init<sizeof...(Ts)>{});
    }

    template <typename Fn, typename... Ts>
    requires (is_callable<Fn&&, _(Ts const&...)>)
    static constexpr decltype(auto) apply(Fn&& fn, Tuple<Ts...> const& self) {
        return apply(fn, self, index_sequence_init<sizeof...(Ts)>{});
    }

    template <typename Fn, typename... Ts>
    requires (is_callable<Fn&&, _(Ts&&...)>)
    static constexpr decltype(auto) apply(Fn&& fn, Tuple<Ts...>&& self) {
        return apply(fn, static_cast<Tuple<Ts...>&&>(self), index_sequence_init<sizeof...(Ts)>{});
    }

    template <typename Fn, typename... Ts>
    requires (is_callable<Fn&&, _(Ts const&&...)>)
    static constexpr decltype(auto) apply(Fn&& fn, Tuple<Ts...> const&& self) {
        return apply(fn, static_cast<Tuple<Ts...> const&&>(self), index_sequence_init<sizeof...(Ts)>{});
    }

private:

    template <typename Fn, typename Self, int... _Index>
    static constexpr decltype(auto) apply(Fn&& fn, Self&& self, index_sequence<_Index...>) {
        return fn(static_cast<Self&&>(self).template get<_Index>()...);
    }
};


template <typename... Ts>
requires ((orderingof<remove_const_reference<Ts>> + ... + Ordering::Order) == Ordering::Order)
class Comparator<Tuple<Ts...>, Ordering::Order> {
public:

    static constexpr Order compare(Tuple<Ts...> const& left, Tuple<Ts...> const& right) 
    {
        return Tuple<Ts...>::compare(left, right, index_sequence_init<sizeof...(Ts)>{});
    }
};

template <typename... Ts>
requires ((orderingof<remove_const_reference<Ts>> + ... + Ordering::Order) == Ordering::ParOrder)
class Comparator<Tuple<Ts...>, Ordering::ParOrder> {
public:

    static constexpr ParOrder compare(Tuple<Ts...> const& left, Tuple<Ts...> const& right) 
    {
        return Tuple<Ts...>::compare(left, right, index_sequence_init<sizeof...(Ts)>{});
    }
};

template <typename... Ts>
requires ((orderingof<remove_const_reference<Ts>> + ... + Ordering::Order) == Ordering::Equal)
class Comparator<Tuple<Ts...>, Ordering::Equal> {
public:

    static constexpr Equal compare(Tuple<Ts...> const& left, Tuple<Ts...> const& right) 
    {
        return Tuple<Ts...>::compare(left, right, index_sequence_init<sizeof...(Ts)>{});
    }
};

template <typename... Ts>
requires ((orderingof<remove_const_reference<Ts>> + ... + Ordering::Order) == Ordering::ParEqual)
class Comparator<Tuple<Ts...>, Ordering::ParEqual> {
public:

    static constexpr ParEqual compare(Tuple<Ts...> const& left, Tuple<Ts...> const& right) 
    {
        return Tuple<Ts...>::compare(left, right, index_sequence_init<sizeof...(Ts)>{});
    }
};


_MNC_END namespace std
{
    #if defined (__clang__) && defined(__GNUC__)  // libc++
    inline namespace __1
    {
        template <class>
        struct tuple_size;

        template <mnc::ulong, class>
        struct tuple_element;
    }
    #else  // libstdc++ and msvc
    template <class>
    struct tuple_size;

    template <mnc::ulong, class>
    struct tuple_element;
    #endif

    template <typename... Ts>
    struct tuple_size<mnc::Tuple<Ts...>> {
        static constexpr mnc::ulong value = sizeof...(Ts);
    };

    template <mnc::ulong _Index, typename... Ts>
    struct tuple_element<_Index, mnc::Tuple<Ts...>> {
        using type = mnc::type_at<_Index, Ts...>;
    };
} _MNC_BEGIN

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

_MNC_END

#undef _MNC_BEGIN
#undef _MNC_END
