#pragma once
#include <cstdio>
#include <cstdlib>
#include <new>
#include "type_traits.h"


#define _MNC_BEGIN namespace mnc {
#define _MNC_END }

_MNC_BEGIN

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// class: Opt<T, E> //////////////////////////
////////////////////////////////////////////////////////////////////////////
#pragma region Opt<T, E>


template <typename T, typename E>
requires (
    (is_move_constructible<T> /*&& is_noexcept_resource<T>*/ || same_type<T, void>) && 
    (is_move_constructible<E> && is_copy_constructible<E> /*&& is_noexcept_resource<T>*/ || same_type<E, void>) &&
    !is_rvalue_reference<T> &&
    !is_reference<E>
)
class Opt {
private:

    struct __throw 
    {
        static void OPT_unwrap_on_False_state() noexcept
        {
            std::fprintf(stderr, "[mnc::Opt] ;; ABORT WARNING ;; unwrap on False state\n");
            std::fflush(stderr);
            std::abort();
        }

        static void OPT_unwrap_err_on_True_state() noexcept
        {
            std::fprintf(stderr, "[mnc::Opt] ;; ABORT WARNING ;; unwrap_err on True state\n");
            std::fflush(stderr);
            std::abort();
        }
    };

    template <typename Val, typename Err>
    struct Data {
        union {
            Val value;
            Err error;
        };
        bool isOK;
    };

    template <typename Val>
    struct Data<Val, void> {
        union {
            Val value;
        };
        bool isOK;
    };

    template <typename Val, typename Err>
    struct Data<Val&, Err> {
        union {
            Val* ptr;
            Err error;
        };
        bool isOK;
    };

    template <typename Val>
    struct Data<Val&, void> {
        union {
            Val* ptr;
        };
        bool isOK;
    };

    template <typename Err>
    struct Data<void, Err> {
        union {
            Err error;
        };
        bool isOK;
    };

    template <>
    struct Data<void, void> {
        bool isOK;
    };

private:

    Data<T, E> data;

public:

    template <typename Arg>
    constexpr Opt(True, Arg&& val) noexcept
    requires (
        !same_type<T, void> && 
        is_convertible_from<T, Arg&&> &&
        is_trivially_constructible_from<T, Arg&&> && is_trivially_destructible<T> &&
        !is_lvalue_reference<T>
    )
    : data{ .value = val, .isOK = true } {}

    template <typename Arg>
    constexpr Opt(True, Arg&& val) noexcept
    requires (
        !same_type<T, void> && 
        is_convertible_from<T, Arg&&> &&
        is_lvalue_reference<T>
    )
    : data{ .ptr = &val, .isOK = true } {}

    template <typename Arg>
    Opt(True, Arg&& val)
    requires (
        !same_type<T, void> && 
        is_convertible_from<T, Arg&&> &&
        !(is_trivially_constructible_from<T, Arg&&> && is_trivially_destructible<T>) 
        //&& is_noexcept_constructible_from<T, Arg&&>
    )
    : data{ .isOK = true }
    {
        ::new (&data.value) T { mnc::forward<Arg>(val) };
    }

    constexpr Opt(True) noexcept
    requires (same_type<T, void>)
    : data{ .isOK = true } {}

    template <typename Arg>
    constexpr Opt(False, Arg&& err)
    requires (
        !same_type<E, void> && 
        is_convertible_from<E, Arg&&> &&
        is_trivially_constructible_from<E, Arg&&> && is_trivially_destructible<E>
    )
    : data{ .error = err, .isOK = false } {}

    template <typename Arg>
    Opt(False, Arg&& err)
    requires (
        !same_type<E, void> && 
        is_convertible_from<E, Arg&&> &&
        !(is_trivially_constructible_from<E, Arg&&> && is_trivially_destructible<E>)
    )
    : data{ .isOK = false }
    {
        ::new (&data.error) E { mnc::forward<Arg>(err) };
    }

    constexpr Opt(False) noexcept
    requires (same_type<E, void>)
    : data{ .isOK = false } {}

    constexpr Opt(Opt const& other) noexcept
    requires (
        (same_type<T, void> || is_trivially_copy_constructible<T>) &&
        (same_type<E, void> || is_trivially_copy_constructible<E>)
    ) = default;
    //: data{ other.data } {}

    Opt(Opt const& other)
    requires (
        (!same_type<T, void> && !is_trivially_copy_constructible<T> ||
        !same_type<E, void> && !is_trivially_copy_constructible<E>) &&
        is_copy_constructible<T>
    )
    : data{ .isOK = other.data.isOK }
    {
        if constexpr (!same_type<T, void> && !is_trivially_copy_constructible<T>) {
            if (data.isOK) ::new (&data.value) T { other.data.value };
        }
        
        if constexpr (!same_type<E, void> && !is_trivially_copy_constructible<E>) {
            if (!data.isOK) ::new (&data.error) E { other.data.error };
        }
    }

    constexpr Opt(Opt&& other) noexcept
    requires (
        (same_type<T, void> || is_trivially_move_constructible<T>) &&
        (same_type<E, void> || is_trivially_move_constructible<E>)
    ) = default;
    //: data{ mov(other.data) } {}

    Opt(Opt&& other)
    requires (
        (!same_type<T, void> && !is_trivially_move_constructible<T>) ||
        (!same_type<E, void> && !is_trivially_move_constructible<E>)
    )
    : data{ .isOK = other.data.isOK }
    {
        if constexpr (!same_type<T, void> && !is_trivially_move_constructible<T>) {
            if (data.isOK) ::new (&data.value) T { static_cast<T&&>(other.data.value) };
        }

        if constexpr (!same_type<E, void> && !is_trivially_move_constructible<E>) {
            if (!data.isOK) ::new (&data.error) E { static_cast<E&&>(other.data.error) };
        }
    }

    constexpr ~Opt() noexcept
    requires (
        (same_type<T, void> || is_trivially_destructible<T>) &&
        (same_type<E, void> || is_trivially_destructible<E>)
    ) = default;

    ~Opt()
    requires (
        (!same_type<T, void> && !is_trivially_destructible<T>) ||
        (!same_type<E, void> && !is_trivially_destructible<E>)
    )
    {
        if constexpr (!same_type<T, void> && !is_trivially_destructible<T>) {
            if (data.isOK) data.value.~T();
        }

        if constexpr (!same_type<E, void> && !is_trivially_destructible<E>) {
            if (!data.isOK) data.error.~E();
        }
    }

    constexpr Opt& operator=(Opt const& other) noexcept
    requires (
        (is_trivially_copy_constructible<T> || same_type<T, void>) &&
        (is_trivially_copy_constructible<E> || same_type<E, void>)
    ) = default;

    Opt& operator=(Opt const& other)
    requires (
        (!is_trivially_copy_constructible<T> && !same_type<T, void> ||
        !is_trivially_copy_constructible<E> && !same_type<E, void>) &&
        is_copy_constructible<T>
    )
    {
        if (this == &other)
            return *this;

        /// DESTRUCTOR
        {
            if constexpr (!same_type<T, void> && !is_trivially_destructible<T>) {
                if (data.isOK) data.value.~T();
            }

            if constexpr (!same_type<E, void> && !is_trivially_destructible<E>) {
                if (!data.isOK) data.error.~E();
            }
        }

        /// COPY CONSTRUCTOR
        data.isOK = other.data.isOK;
        {
            if constexpr (!same_type<T, void>) {
                if (data.isOK) ::new (&data.value) T { other.data.value };
            }
            
            if constexpr (!same_type<E, void>) {
                if (!data.isOK) ::new (&data.error) E { other.data.error };
            }
        }

        return *this;
    }

    constexpr Opt& operator=(Opt&& other) noexcept
    requires (
        (is_trivially_move_constructible<T> || same_type<T, void>) &&
        (is_trivially_move_constructible<E> || same_type<E, void>)
    ) = default;

    Opt& operator=(Opt&& other)
    requires (
        (!is_trivially_move_constructible<T> && !same_type<T, void>) ||
        (!is_trivially_move_constructible<E> && !same_type<E, void>)
    )
    {
        if (this == &other)
            return *this;

        /// DESTRUCTOR
        {
            if constexpr (!same_type<T, void> && !is_trivially_destructible<T>) {
                if (data.isOK) data.value.~T();
            }

            if constexpr (!same_type<E, void> && !is_trivially_destructible<E>) {
                if (!data.isOK) data.error.~E();
            }
        }
        
        /// MOVE CONSTRUCTOR
        data.isOK = other.data.isOK;
        {
            if constexpr (!same_type<T, void>) {
                if (data.isOK) ::new (&data.value) T { static_cast<T&&>(other.data.value) };
            }

            if constexpr (!same_type<E, void>) {
                if (!data.isOK) ::new (&data.error) E { static_cast<E&&>(other.data.error) };
            }
        }

        return *this;
    }

    constexpr Opt clone() const
    requires (is_copy_constructible<T>)
    {
        return Opt(*this);
    }

    constexpr bool operator==(True) const noexcept { return data.isOK; }
    constexpr bool operator!=(True) const noexcept { return !data.isOK; }
    constexpr bool operator==(False) const noexcept { return !data.isOK; }
    constexpr bool operator!=(False) const noexcept { return data.isOK; }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////// fn: match ///////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

private:

    template <typename Ret, typename... Args>
    struct struct_Sig;

    template <typename Ret, typename Arg1, typename Arg2>
    struct struct_Sig<Ret, Arg1, Arg2> {
        using type = Ret(Arg1, Arg2);
    };

    template <typename Ret, typename Arg1>
    struct struct_Sig<Ret, Arg1, void> {
        using type = Ret(Arg1);
    };

    template <typename Ret, typename Arg2>
    struct struct_Sig<Ret, void, Arg2> {
        using type = Ret(Arg2);
    };

    template <typename Ret>
    struct struct_Sig<Ret, void, void> {
        using type = Ret();
    };

    template <typename Ret, typename... Args>
    using Sig = typename struct_Sig<Ret, Args...>::type;

    template <typename Ty>
    struct struct_Ref {
        using type = Ty&;
    };

    template <>
    struct struct_Ref<void> {
        using type = void;
    };

    template <>
    struct struct_Ref<void const> {
        using type = void;
    };

    template <typename Ty>
    using Ref = typename struct_Ref<Ty>::type;

    template <typename Ty>
    struct Rvalue {
        using type = Ty&&;
    };

    template <>
    struct Rvalue<void> {
        using type = void;
    };

    template <>
    struct Rvalue<void const> {
        using type = void;
    };

public:

    template <
        typename Branch_True, 
        typename Branch_False,
        typename Ret = return_type<Branch_True&&, Sig<_, True, Ref<T>>>
    >
    constexpr Ret match(Branch_True&& fn1, Branch_False&& fn2) 
    requires (
        !same_type<Ret, NAUR> &&
        same_type<
            return_type<Branch_True&&, Sig<_, True, Ref<T>>>,
            return_type<Branch_False&&, Sig<_, False, Ref<E>>>
        >
    )
    {
        if (data.isOK)
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>) 
                return fn1(True{}, data.value);
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return fn1(True{}, *data.ptr);
            else
                return fn1(True{});
        }
        else
        {
            if constexpr (!same_type<E, void>)
                return fn2(False{}, data.error);
            else
                return fn2(False{});
        }
    }

    template <
        typename Branch_True, 
        typename Branch_False,
        typename Ret = return_type<Branch_True&&, Sig<_, True, Ref<T const>>>
    >
    constexpr Ret match(Branch_True&& fn1, Branch_False&& fn2) const 
    requires (
        !same_type<Ret, NAUR> &&
        same_type<
            return_type<Branch_True&&, Sig<_, True, Ref<T const>>>,
            return_type<Branch_False&&, Sig<_, False, Ref<E const>>>
        >
    )
    {
        if (data.isOK)
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>) 
                return fn1(True{}, data.value);
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return fn1(True{}, *data.ptr);
            else
                return fn1(True{});
        }
        else
        {
            if constexpr (!same_type<E, void>)
                return fn2(False{}, data.error);
            else
                return fn2(False{});
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////// fn: unwrap ///////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    //
    constexpr decltype(auto) unwrap() &
    {
        if (!data.isOK)
            __throw::OPT_unwrap_on_False_state();

        if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
            return *&data.value;
        else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
            return *data.ptr;
    }

    constexpr decltype(auto) unwrap() const& 
    {
        if (!data.isOK)
            __throw::OPT_unwrap_on_False_state();

        if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
            return *&data.value;
        else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
            return *data.ptr;
    }

    constexpr decltype(auto) unwrap() && 
    {
        if (!data.isOK)
            __throw::OPT_unwrap_on_False_state();

        if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
            return static_cast<T&&>(data.value);
        else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
            return *data.ptr;
    }

    ////////////////////////////////////////////////////////////////////////////
    /////////////////////////////// fn: unwrap_err /////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    //
    constexpr decltype(auto) unwrap_err() & 
    {
        if (data.isOK)
            __throw::OPT_unwrap_err_on_True_state();

        if constexpr (!same_type<E, void>)
            return *&data.error;
    }

    constexpr decltype(auto) unwrap_err() const& 
    {
        if (data.isOK)
            __throw::OPT_unwrap_err_on_True_state();

        if constexpr (!same_type<E, void>)
            return *&data.error;
    }

    constexpr decltype(auto) unwrap_err() && 
    {
        if (data.isOK)
            __throw::OPT_unwrap_err_on_True_state();
        
        if constexpr (!same_type<E, void>)
            return static_cast<E&&>(data.error);
    }

    ////////////////////////////////////////////////////////////////////////////
    //////////////////////////////// fn: value_or //////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    //
    template <typename Fallback>
    constexpr T value_or(Fallback&& fallback_val) const& 
    requires (!same_type<T, void> && is_copy_constructible<T> && is_convertible_from<T, Fallback&&>)
    {
        if (data.isOK) 
        {
            if constexpr (!is_lvalue_reference<T>)
                return data.value;
            else
                return *data.ptr;
        }
        else
        {
            return forward<Fallback>(fallback_val);
        }
    }

    template <typename Fallback>
    constexpr T value_or(Fallback&& fallback_val) && 
    requires (!same_type<T, void> && is_convertible_from<T, Fallback&&>)
    {
        if (data.isOK)
        {
            if constexpr (!is_lvalue_reference<T>)
                return static_cast<T&&>(data.value);
            else
                return *data.ptr;
        }
        else
        {
            return forward<Fallback>(fallback_val);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: value_or_call ////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    //
    template <typename Fallback>
    constexpr T value_or_call(Fallback&& fallback_fn) const& 
    requires (!same_type<T, void> && is_copy_constructible<T> && is_callable<Fallback&&, T()>)
    {
        if (data.isOK)
        {
            if constexpr (!is_lvalue_reference<T>)
                return data.value;
            else
                return *data.ptr;
        }
        else
        {
            return fallback_fn();
        }
    }

    template <typename Fallback>
    constexpr T value_or_call(Fallback&& fallback_fn) && 
    requires (!same_type<T, void> && is_callable<Fallback&&, T()>)
    {
        if (data.isOK)
        {
            if constexpr (!is_lvalue_reference<T>)
                return static_cast<T&&>(data.value);
            else
                return *data.ptr;
        }
        else
        {
            return fallback_fn();
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////// fn: map /////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

private:

    template <typename Ret, typename Arg>
    struct struct_Sig<Ret, Arg> {
        using type = Ret(Arg);
    };

    template <typename Ret>
    struct struct_Sig<Ret, void> {
        using type = Ret();
    };

public:

    // fn: T -> T'
    template <
        typename Mapper,
        typename Out = return_type<Mapper&&, Sig<_, Ref<T const>>>
    >
    constexpr Opt<Out, E> map(Mapper&& fn) const& 
    requires (!same_type<Out, NAUR> && (is_copy_constructible<T> || same_type<T, void>))
    {
        if (data.isOK)
        {
            if constexpr (!same_type<Out, void> && !same_type<T, void> && !is_lvalue_reference<T>)
                return { True{}, fn(data.value) };
            else if constexpr (!same_type<Out, void> && !same_type<T, void> && is_lvalue_reference<T>)
                return { True{}, fn(*data.ptr) };
            else if constexpr (!same_type<Out, void> && same_type<T, void>)
                return { True{}, fn() };
            else
                return { True{} };
        }
        else
        {
            if constexpr (!same_type<E, void>)
                return { False{}, data.error };
            else
                return { False{} };
        }
    }

    // fn: T -> T'
    template <
        typename Mapper,
        typename Out = return_type<Mapper&&, Sig<_, Rvalue<T>>>
    >
    constexpr Opt<Out, E> map(Mapper&& fn) && 
    requires (!same_type<Out, NAUR>)
    {
        if (data.isOK)
        {
            if constexpr (!same_type<Out, void> && !same_type<T, void> && !is_lvalue_reference<T>)
                return { True{}, fn(static_cast<T&&>(data.value)) };
            else if constexpr (!same_type<Out, void> && !same_type<T, void> && is_lvalue_reference<T>)
                return { True{}, fn(*data.ptr) };
            else if constexpr (!same_type<Out, void> && same_type<T, void>)
                return { True{}, fn() };
            else
                return { True{} };
        }
        else
        {
            if constexpr (!same_type<E, void>)
                return { False{}, static_cast<E&&>(data.error) };
            else
                return { False{} };
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //////////////////////////////// fn: map_err ///////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    // fn: E -> E'
    template <
        typename Mapper,
        typename Out = return_type<Mapper&&, Sig<_, Ref<E const>>>
    >
    constexpr Opt<T, Out> map_err(Mapper&& fn) const& 
    requires (!same_type<Out, NAUR> && is_copy_constructible<T>)
    {
        if (data.isOK)
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
                return { True{}, data.value };
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return { True{}, *data.ptr };
            else
                return { True{} };
        }
        else
        {
            if constexpr (!same_type<Out, void> && !same_type<E, void>)
                return { False{}, fn(data.error) };
            else if constexpr (!same_type<Out, void> && same_type<E, void>)
                return { False{}, fn() };
            else
                return { False{} };
        }
    }

    // fn: E -> E'
    template <
        typename Mapper,
        typename Out = return_type<Mapper&&, Sig<_, Rvalue<E>>>
    >
    constexpr Opt<T, Out> map_err(Mapper&& fn) && 
    requires (!same_type<Out, NAUR>)
    {
        if (data.isOK)
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
                return { True{}, static_cast<T&&>(data.value) };
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return { True{}, *data.ptr };
            else
                return { True{} };
        }
        else
        {
            if constexpr (!same_type<Out, void> && !same_type<E, void>)
                return { False{}, fn(static_cast<E&&>(data.error)) };
            else if constexpr (!same_type<Out, void> && same_type<E, void>)
                return { False{}, fn() };
            else
                return { False{} };
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////// fn: and_then /////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

private:

    template <typename Ty>
    struct AsOpt {
        using val = NAUR;
        using err = NAUR;
    };

    template <typename _T, typename _E>
    struct AsOpt<Opt<_T, _E>> {
        using val = _T;
        using err = _E;
    };

public:

    // fn: T -> Opt<T', E>
    template <
        typename Chain,
        typename Out = return_type<Chain&&, Sig<_, Ref<T const>>>
    >
    constexpr Out and_then(Chain&& fn) const&
    requires (
        !same_type<typename AsOpt<Out>::val, NAUR> && 
        same_type<typename AsOpt<Out>::err, E> &&
        (is_copy_constructible<T> || same_type<T, void>))
    {
        if (data.isOK)
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
                return fn(data.value);
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return fn(*data.ptr);
            else
                return fn();
        }
        else
        {
            if constexpr (!same_type<E, void>)
                return { False{}, data.error };
            else
                return { False{} };
        }
    }

    // fn: T -> Opt<T', E>
    template <
        typename Chain,
        typename Out = return_type<Chain&&, Sig<_, Rvalue<T>>>
    >
    constexpr Out and_then(Chain&& fn) &&
    requires (
        !same_type<typename AsOpt<Out>::val, NAUR> && 
        same_type<typename AsOpt<Out>::err, E>)
    {
        if (data.isOK)
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
                return fn(static_cast<T&&>(data.value));
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return fn(*data.ptr);
            else
                return fn();
        }
        else
        {
            if constexpr (!same_type<E, void>)
                return { False{}, static_cast<E&&>(data.error) };
            else
                return { False{} };
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////// fn: or_else //////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    // fn: E -> Opt<T, E'>
    template <
        typename ErrChain,
        typename Out = return_type<ErrChain&&, Sig<_, Ref<E const>>>
    >
    constexpr Out or_else(ErrChain&& fn) const&
    requires (
        same_type<typename AsOpt<Out>::val, T> &&
        !same_type<typename AsOpt<Out>::err, NAUR> &&
        (is_copy_constructible<T> || same_type<T, void>))
    {
        if (data.isOK)
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
                return { True{}, data.value };
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return { True{}, *data.ptr };
            else
                return { True{} };
        }
        else
        {
            if constexpr (!same_type<E, void>)
                return fn(data.error);
            else
                return fn();
        }
    }

    // fn: E -> Opt<T, E'>
    template <
        typename ErrChain,
        typename Out = return_type<ErrChain&&, Sig<_, Rvalue<E>>>
    >
    constexpr Out or_else(ErrChain&& fn) &&
    requires (
        same_type<typename AsOpt<Out>::val, T> &&
        !same_type<typename AsOpt<Out>::err, NAUR>)
    {
        if (data.isOK)
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
                return { True{}, static_cast<T&&>(data.value) };
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return { True{}, *data.ptr };
            else
                return { True{} };
        }
        else
        {
            if constexpr (!same_type<E, void>)
                return fn(static_cast<E&&>(data.error));
            else
                return fn();
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////// fn: filter ///////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    // fn: T -> bool
    template <typename Predicate>
    constexpr Opt filter(Predicate&& fn) const&
    requires (
        same_type<E, void> && 
        is_callable<Predicate&&, Sig<bool, Ref<T const>>> &&
        is_copy_constructible<T> &&
        !same_type<T, void>)
    {
        if constexpr (!is_lvalue_reference<T>)
        {
            if (data.isOK && fn(data.value)) 
                return { True{}, data.value };
            else
                return { False{} };
        }
        else
        {
            if (data.isOK && fn(*data.ptr)) 
                return { True{}, *data.ptr };
            else
                return { False{} };
        }
    }

    // fn: T -> bool
    template <typename Predicate>
    constexpr Opt filter(Predicate&& fn) &&
    requires (
        same_type<E, void> && 
        is_callable<Predicate&&, Sig<bool, Ref<T const>>> &&
        !same_type<T, void>)
    {
        if constexpr (!is_lvalue_reference<T>)
        {
            if (data.isOK && fn(data.value)) 
                return { True{}, static_cast<T&&>(data.value) };
            else
                return { False{} };
        }
        else
        {
            if (data.isOK && fn(*data.ptr)) 
                return { True{}, *data.ptr };
            else
                return { False{} };
        }
    }
};

template <typename T>
using Option = Opt<T, void>;

template <typename T, typename E>
using Result = Opt<T, E>;

#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

_MNC_END

#undef _MNC_BEGIN
#undef _MNC_END