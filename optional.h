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
class Opt;

template <typename T, typename E>
requires (
    (is_move_constructible<T> || same_type<T, void>) && 
    (is_move_constructible<E> && is_copy_constructible<E> || same_type<E, void>) &&
    !is_rvalue_reference<T> &&
    !is_reference<E>
)
class Opt<T, E> {
private:

    struct __throw 
    {
        static void OPT_unwrap_on_False_state() 
        {
            std::fprintf(stderr, "[mnc::Opt] ;; ABORT WARNING ;; unwrap on False state\n");
            std::fflush(stderr);
            std::abort();
        }

        static void OPT_unwrap_err_on_True_state() 
        {
            std::fprintf(stderr, "[mnc::Opt] ;; ABORT WARNING ;; unwrap_err on True state\n");
            std::fflush(stderr);
            std::abort();
        }
    };

    #if defined(__clang__) || defined(__GNUC__)   // optimization for clang and gcc fanboys🤠
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wpedantic"
        using Unit = bool[0];
        #pragma GCC diagnostic pop
    #else
        using Unit = bool;
    #endif

    template <typename Ty>
    struct Enable_If_Value {
        using type = Ty;
    };

    template <typename Ty>
    struct Enable_If_Value<Ty&> {
        using type = Unit;
    };

    template <>
    struct Enable_If_Value<void> {
        using type = Unit;
    };

    using Val = typename Enable_If_Value<T>::type;
    using Err = typename Enable_If_Value<E>::type;

    template <typename Ty>
    struct Enable_If_Ref {
        using type = Unit;
    };

    template <typename Ty>
    struct Enable_If_Ref<Ty&> {
        using type = Ty*;
    };

    using Ptr = typename Enable_If_Ref<T>::type;

    template <bool condition>
    struct Disable_Bool {
        using type = bool;
    };

    template <>
    struct Disable_Bool<true> {
        using type = Unit;
    };

    using Bool = typename Disable_Bool<
        is_lvalue_reference<T> && 
        same_type<E, void>
    >::type;

private:

    union {
        Val value;
        Ptr ptr;
        Err error;
    };

    Bool _isOK;

public:

    template <typename Arg>
    constexpr Opt(True, Arg&& val) 
    requires (
        !same_type<T, void> && 
        is_convertible_from<T, Arg&&> &&
        is_trivially_constructible_from<T, Arg&&> && is_trivially_destructible<T> &&
        !is_lvalue_reference<T>
    )
    : value(val), _isOK(true) {}

    template <typename Arg>
    constexpr Opt(True, Arg&& val) 
    requires (
        !same_type<T, void> && 
        is_convertible_from<T, Arg&&> &&
        is_lvalue_reference<T> && !same_type<E, void>
    )
    : ptr(&val), _isOK(true) {}

    template <typename Arg>
    constexpr Opt(True, Arg&& val) 
    requires (
        !same_type<T, void> && 
        is_convertible_from<T, Arg&&> &&
        is_lvalue_reference<T> && same_type<E, void>
    )
    : ptr(&val) {}

    template <typename Arg>
    Opt(True, Arg&& val)
    requires (
        !same_type<T, void> && 
        is_convertible_from<T, Arg&&> &&
        !(is_trivially_constructible_from<T, Arg&&> && is_trivially_destructible<T>)
    )
    : _isOK(true)
    {
        ::new (&value) T { mnc::forward<Arg>(val) };
    }

    constexpr Opt(True) 
    requires (same_type<T, void>)
    : _isOK(true) {}

    template <typename Arg>
    constexpr Opt(False, Arg&& err)
    requires (
        !same_type<E, void> && 
        is_convertible_from<E, Arg&&> &&
        is_trivially_constructible_from<E, Arg&&> && is_trivially_destructible<E>
    )
    : error(err), _isOK(false) {}

    template <typename Arg>
    Opt(False, Arg&& err)
    requires (
        !same_type<E, void> && 
        is_convertible_from<E, Arg&&> &&
        !(is_trivially_constructible_from<E, Arg&&> && is_trivially_destructible<E>)
    )
    : _isOK(false)
    {
        ::new (&error) E { mnc::forward<Arg>(err) };
    }

    constexpr Opt(False) 
    requires (!is_lvalue_reference<T> && same_type<E, void>)
    : _isOK(false) {}

    constexpr Opt(False) 
    requires (is_lvalue_reference<T> && same_type<E, void>)
    : ptr(nullptr) {}

    constexpr Opt(Opt const& other) 
    requires (
        (same_type<T, void> || is_trivially_copy_constructible<T>) &&
        (same_type<E, void> || is_trivially_copy_constructible<E>)
    ) = default;

    Opt(Opt const& other)
    requires (
        (!same_type<T, void> && !is_trivially_copy_constructible<T> ||
        !same_type<E, void> && !is_trivially_copy_constructible<E>) &&
        is_copy_constructible<T>
    )
    : _isOK(other._isOK)
    {
        if constexpr (!same_type<T, void> && !is_trivially_copy_constructible<T>) {
            if (_isOK) ::new (&value) T { other.value };
        }
        
        if constexpr (!same_type<E, void> && !is_trivially_copy_constructible<E>) {
            if (!_isOK) ::new (&error) E { other.error };
        }
    }

    constexpr Opt(Opt&& other) 
    requires (
        (same_type<T, void> || is_trivially_move_constructible<T>) &&
        (same_type<E, void> || is_trivially_move_constructible<E>)
    ) = default;

    Opt(Opt&& other)
    requires (
        (!same_type<T, void> && !is_trivially_move_constructible<T>) ||
        (!same_type<E, void> && !is_trivially_move_constructible<E>)
    )
    : _isOK(other._isOK)
    {
        if constexpr (!same_type<T, void> && !is_trivially_move_constructible<T>) {
            if (_isOK) ::new (&value) T { static_cast<T&&>(other.value) };
        }

        if constexpr (!same_type<E, void> && !is_trivially_move_constructible<E>) {
            if (!_isOK) ::new (&error) E { static_cast<E&&>(other.error) };
        }
    }

    constexpr ~Opt() 
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
            if (_isOK) value.~T();
        }

        if constexpr (!same_type<E, void> && !is_trivially_destructible<E>) {
            if (!_isOK) error.~E();
        }
    }

    constexpr Opt& operator=(Opt const& other) 
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
                if (_isOK) value.~T();
            }

            if constexpr (!same_type<E, void> && !is_trivially_destructible<E>) {
                if (!_isOK) error.~E();
            }
        }

        /// COPY CONSTRUCTOR
        _isOK = other._isOK;
        {
            if constexpr (!same_type<T, void>) {
                if (_isOK) ::new (&value) T { other.value };
            }
            
            if constexpr (!same_type<E, void>) {
                if (!_isOK) ::new (&error) E { other.error };
            }
        }

        return *this;
    }

    constexpr Opt& operator=(Opt&& other) 
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
                if (_isOK) value.~T();
            }

            if constexpr (!same_type<E, void> && !is_trivially_destructible<E>) {
                if (!_isOK) error.~E();
            }
        }
        
        /// MOVE CONSTRUCTOR
        _isOK = other._isOK;
        {
            if constexpr (!same_type<T, void>) {
                if (_isOK) ::new (&value) T { static_cast<T&&>(other.value) };
            }

            if constexpr (!same_type<E, void>) {
                if (!_isOK) ::new (&error) E { static_cast<E&&>(other.error) };
            }
        }

        return *this;
    }

private:

    constexpr bool isOK() const
    {
        if constexpr (is_lvalue_reference<T> && same_type<E, void>)
            return ptr != nullptr;
        else
            return _isOK;
    }

public:

    constexpr Opt clone() const
    requires (is_copy_constructible<T>)
    {
        return Opt(*this);
    }

    constexpr bool operator==(True) const { return isOK(); }
    constexpr bool operator!=(True) const { return !isOK(); }
    constexpr bool operator==(False) const { return !isOK(); }
    constexpr bool operator!=(False) const { return isOK(); }

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
        if (isOK())
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>) 
                return fn1(True{}, value);
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return fn1(True{}, *ptr);
            else
                return fn1(True{});
        }
        else
        {
            if constexpr (!same_type<E, void>)
                return fn2(False{}, error);
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
        if (isOK())
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>) 
                return fn1(True{}, value);
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return fn1(True{}, *ptr);
            else
                return fn1(True{});
        }
        else
        {
            if constexpr (!same_type<E, void>)
                return fn2(False{}, error);
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
        if (!isOK())
            __throw::OPT_unwrap_on_False_state();

        if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
            return *&value;
        else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
            return *ptr;
    }

    constexpr decltype(auto) unwrap() const& 
    {
        if (!isOK())
            __throw::OPT_unwrap_on_False_state();

        if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
            return *&value;
        else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
            return *ptr;
    }

    constexpr decltype(auto) unwrap() && 
    {
        if (!isOK())
            __throw::OPT_unwrap_on_False_state();

        if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
            return static_cast<T&&>(value);
        else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
            return *ptr;
    }

    ////////////////////////////////////////////////////////////////////////////
    /////////////////////////////// fn: unwrap_err /////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    //
    constexpr decltype(auto) unwrap_err() & 
    {
        if (isOK())
            __throw::OPT_unwrap_err_on_True_state();

        if constexpr (!same_type<E, void>)
            return *&error;
    }

    constexpr decltype(auto) unwrap_err() const& 
    {
        if (isOK())
            __throw::OPT_unwrap_err_on_True_state();

        if constexpr (!same_type<E, void>)
            return *&error;
    }

    constexpr decltype(auto) unwrap_err() && 
    {
        if (isOK())
            __throw::OPT_unwrap_err_on_True_state();
        
        if constexpr (!same_type<E, void>)
            return static_cast<E&&>(error);
    }

    ////////////////////////////////////////////////////////////////////////////
    //////////////////////////////// fn: value_or //////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    //
    template <typename Fallback>
    constexpr T value_or(Fallback&& fallback_val) const& 
    requires (!same_type<T, void> && is_copy_constructible<T> && is_convertible_from<T, Fallback&&>)
    {
        if (isOK()) 
        {
            if constexpr (!is_lvalue_reference<T>)
                return value;
            else
                return *ptr;
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
        if (isOK())
        {
            if constexpr (!is_lvalue_reference<T>)
                return static_cast<T&&>(value);
            else
                return *ptr;
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
        if (isOK())
        {
            if constexpr (!is_lvalue_reference<T>)
                return value;
            else
                return *ptr;
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
        if (isOK())
        {
            if constexpr (!is_lvalue_reference<T>)
                return static_cast<T&&>(value);
            else
                return *ptr;
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
        if (isOK())
        {
            if constexpr (!same_type<Out, void> && !same_type<T, void> && !is_lvalue_reference<T>)
                return { True{}, fn(value) };
            else if constexpr (!same_type<Out, void> && !same_type<T, void> && is_lvalue_reference<T>)
                return { True{}, fn(*ptr) };
            else if constexpr (!same_type<Out, void> && same_type<T, void>)
                return { True{}, fn() };
            else
                return { True{} };
        }
        else
        {
            if constexpr (!same_type<E, void>)
                return { False{}, error };
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
        if (isOK())
        {
            if constexpr (!same_type<Out, void> && !same_type<T, void> && !is_lvalue_reference<T>)
                return { True{}, fn(static_cast<T&&>(value)) };
            else if constexpr (!same_type<Out, void> && !same_type<T, void> && is_lvalue_reference<T>)
                return { True{}, fn(*ptr) };
            else if constexpr (!same_type<Out, void> && same_type<T, void>)
                return { True{}, fn() };
            else
                return { True{} };
        }
        else
        {
            if constexpr (!same_type<E, void>)
                return { False{}, static_cast<E&&>(error) };
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
        if (isOK())
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
                return { True{}, value };
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return { True{}, *ptr };
            else
                return { True{} };
        }
        else
        {
            if constexpr (!same_type<Out, void> && !same_type<E, void>)
                return { False{}, fn(error) };
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
        if (isOK())
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
                return { True{}, static_cast<T&&>(value) };
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return { True{}, *ptr };
            else
                return { True{} };
        }
        else
        {
            if constexpr (!same_type<Out, void> && !same_type<E, void>)
                return { False{}, fn(static_cast<E&&>(error)) };
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
        if (isOK())
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
                return fn(value);
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return fn(*ptr);
            else
                return fn();
        }
        else
        {
            if constexpr (!same_type<E, void>)
                return { False{}, error };
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
        if (isOK())
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
                return fn(static_cast<T&&>(value));
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return fn(*ptr);
            else
                return fn();
        }
        else
        {
            if constexpr (!same_type<E, void>)
                return { False{}, static_cast<E&&>(error) };
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
        if (isOK())
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
                return { True{}, value };
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return { True{}, *ptr };
            else
                return { True{} };
        }
        else
        {
            if constexpr (!same_type<E, void>)
                return fn(error);
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
        if (isOK())
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
                return { True{}, static_cast<T&&>(value) };
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return { True{}, *ptr };
            else
                return { True{} };
        }
        else
        {
            if constexpr (!same_type<E, void>)
                return fn(static_cast<E&&>(error));
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
            if (isOK() && fn(value)) 
                return { True{}, value };
            else
                return { False{} };
        }
        else
        {
            if (isOK() && fn(*ptr)) 
                return { True{}, *ptr };
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
            if (isOK() && fn(value)) 
                return { True{}, static_cast<T&&>(value) };
            else
                return { False{} };
        }
        else
        {
            if (isOK() && fn(*ptr)) 
                return { True{}, *ptr };
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