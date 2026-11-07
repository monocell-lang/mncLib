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
    (same_type<T, void> || is_move_constructible<T>) && !is_rvalue_reference<T> && 
    (same_type<E, void> || (is_move_constructible<E> && is_copy_constructible<E>)) && !is_reference<E>
)
class [[nodiscard]] Opt {
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
    : value(static_cast<Arg&&>(val)), _isOK(true) {}

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
    /* constexpr */ Opt(True, Arg&& val)
    requires (
        !same_type<T, void> && 
        is_convertible_from<T, Arg&&> &&
        !(is_trivially_constructible_from<T, Arg&&> && is_trivially_destructible<T>)
    )
    : _isOK(true)
    {
        ::new (&value) T { static_cast<Arg&&>(val) };
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
    : error(static_cast<Arg&&>(err)), _isOK(false) {}

    template <typename Arg>
    /* constexpr */ Opt(False, Arg&& err)
    requires (
        !same_type<E, void> && 
        is_convertible_from<E, Arg&&> &&
        !(is_trivially_constructible_from<E, Arg&&> && is_trivially_destructible<E>)
    )
    : _isOK(false)
    {
        ::new (&error) E { static_cast<Arg&&>(err) };
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

    /* constexpr */ Opt(Opt const& other)
    requires (
        ((!same_type<T, void> && !is_trivially_copy_constructible<T>) ||
        (!same_type<E, void> && !is_trivially_copy_constructible<E>)) &&
        (same_type<T, void> || is_copy_constructible<T>)
    )
    : _isOK(other._isOK)
    {
        if constexpr (!same_type<T, void> && !is_lvalue_reference<T>) {
            if (_isOK) ::new (&value) T { other.value };
        }
        else if constexpr (!same_type<T, void> && is_lvalue_reference<T>) {
            if (_isOK) ptr = other.ptr;
        }

        if constexpr (!same_type<E, void>) {
            if (!_isOK) ::new (&error) E { other.error };
        }
    }

    constexpr Opt(Opt&& other) 
    requires (
        (same_type<T, void> || is_trivially_move_constructible<T>) &&
        (same_type<E, void> || is_trivially_move_constructible<E>)
    ) = default;

    /* constexpr */ Opt(Opt&& other)
    requires (
        (!same_type<T, void> && !is_trivially_move_constructible<T>) ||
        (!same_type<E, void> && !is_trivially_move_constructible<E>)
    )
    : _isOK(other._isOK)
    {
        if constexpr (!same_type<T, void> && !is_lvalue_reference<T>) {
            if (_isOK) ::new (&value) T { static_cast<T&&>(other.value) };
        }
        else if constexpr (!same_type<T, void> && is_lvalue_reference<T>) {
            if (_isOK) ptr = other.ptr;
        }

        if constexpr (!same_type<E, void>) {
            if (!_isOK) ::new (&error) E { static_cast<E&&>(other.error) };
        }
    }

    constexpr ~Opt() 
    requires (
        (same_type<T, void> || is_trivially_destructible<T>) &&
        (same_type<E, void> || is_trivially_destructible<E>)
    ) = default;

    /* constexpr */ ~Opt()
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
        (same_type<T, void> || is_trivially_copy_constructible<T>) &&
        (same_type<E, void> || is_trivially_copy_constructible<E>)
    ) = default;

    /* constexpr */ Opt& operator=(Opt const& other)
    requires (
        ((!same_type<T, void> && !is_trivially_copy_constructible<T>) ||
        (!same_type<E, void> && !is_trivially_copy_constructible<E>)) &&
        (same_type<T, void> || is_copy_constructible<T>)
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
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>) {
                if (_isOK) ::new (&value) T { other.value };
            }
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>) {
                if (_isOK) ptr = other.ptr;
            }

            if constexpr (!same_type<E, void>) {
                if (!_isOK) ::new (&error) E { other.error };
            }
        }

        return *this;
    }

    constexpr Opt& operator=(Opt&& other) 
    requires (
        (same_type<T, void> || is_trivially_move_constructible<T>) &&
        (same_type<E, void> || is_trivially_move_constructible<E>)
    ) = default;

    /* constexpr */ Opt& operator=(Opt&& other)
    requires (
        (!same_type<T, void> && !is_trivially_move_constructible<T>) ||
        (!same_type<E, void> && !is_trivially_move_constructible<E>)
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
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>) {
                if (_isOK) ::new (&value) T { static_cast<T&&>(other.value) };
            }
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>) {
                if (_isOK) ptr = other.ptr;
            }

            if constexpr (!same_type<E, void>) {
                if (!_isOK) ::new (&error) E { static_cast<E&&>(other.error) };
            }
        }

        return *this;
    }

    constexpr Opt clone() const
    requires (is_copy_constructible<T>)
    {
        return Opt(*this);
    }

    constexpr bool operator==(True) const { return isOK(); }
    constexpr bool operator!=(True) const { return !isOK(); }
    constexpr bool operator==(False) const { return !isOK(); }
    constexpr bool operator!=(False) const { return isOK(); }

private:

    constexpr bool isOK() const
    {
        if constexpr (is_lvalue_reference<T> && same_type<E, void>)
            return ptr != nullptr;
        else
            return _isOK;
    }

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
        typename Fn1, 
        typename Fn2,
        typename Ret = return_type<Fn1&&, Sig<_, True, Ref<T>>>
    >
    constexpr Ret match(Fn1&& branch_true, Fn2&& branch_false) 
    requires (
        !same_type<Ret, NAUR> &&
        same_type<
            return_type<Fn1&&, Sig<_, True, Ref<T>>>,
            return_type<Fn2&&, Sig<_, False, Ref<E>>>
        >
    )
    {
        if (isOK())
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>) 
                return branch_true(True{}, value);
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return branch_true(True{}, *ptr);
            else
                return branch_true(True{});
        }
        else
        {
            if constexpr (!same_type<E, void>)
                return branch_false(False{}, error);
            else
                return branch_false(False{});
        }
    }

    template <
        typename Fn1, 
        typename Fn2,
        typename Ret = return_type<Fn1&&, Sig<_, True, Ref<T const>>>
    >
    constexpr Ret match(Fn1&& branch_true, Fn2&& branch_false) const 
    requires (
        !same_type<Ret, NAUR> &&
        same_type<
            return_type<Fn1&&, Sig<_, True, Ref<T const>>>,
            return_type<Fn2&&, Sig<_, False, Ref<E const>>>
        >
    )
    {
        if (isOK())
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>) 
                return branch_true(True{}, value);
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return branch_true(True{}, *ptr);
            else
                return branch_true(True{});
        }
        else
        {
            if constexpr (!same_type<E, void>)
                return branch_false(False{}, error);
            else
                return branch_false(False{});
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
            return (value);  // () preserves reference
        else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
            return *ptr;
    }

    constexpr decltype(auto) unwrap() const& 
    {
        if (!isOK())
            __throw::OPT_unwrap_on_False_state();

        if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
            return (value);  // () preserves reference
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

    constexpr decltype(auto) unwrap() const&& 
    {
        if (!isOK())
            __throw::OPT_unwrap_on_False_state();

        if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
            return static_cast<T const&&>(value);
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
            return (error);  // () preserves reference
    }

    constexpr decltype(auto) unwrap_err() const& 
    {
        if (isOK())
            __throw::OPT_unwrap_err_on_True_state();

        if constexpr (!same_type<E, void>)
            return (error);  // () preserves reference
    }

    constexpr decltype(auto) unwrap_err() && 
    {
        if (isOK())
            __throw::OPT_unwrap_err_on_True_state();
        
        if constexpr (!same_type<E, void>)
            return static_cast<E&&>(error);
    }

    constexpr decltype(auto) unwrap_err() const&& 
    {
        if (isOK())
            __throw::OPT_unwrap_err_on_True_state();
        
        if constexpr (!same_type<E, void>)
            return static_cast<E const&&>(error);
    }

    ////////////////////////////////////////////////////////////////////////////
    //////////////////////////////// fn: value_or //////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    //
    template <typename Arg>
    constexpr T value_or(Arg&& fallback) const& 
    requires (!same_type<T, void> && is_copy_constructible<T> && is_convertible_from<T, Arg&&>)
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
            return static_cast<Arg&&>(fallback);
        }
    }

    template <typename Arg>
    constexpr T value_or(Arg&& fallback) && 
    requires (!same_type<T, void> && is_convertible_from<T, Arg&&>)
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
            return static_cast<Arg&&>(fallback);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ///////////////////////////// fn: value_or_call ////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    // fallback: (none) -> T
    template <typename Fn>
    constexpr T value_or_call(Fn&& fallback) const& 
    requires (
        is_callable<Fn&&, T()> && 
        (same_type<T, void> || is_copy_constructible<T>))
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
            return fallback();
        }
    }

    // fallback: (none) -> T
    template <typename Fn>
    constexpr T value_or_call(Fn&& fallback) && 
    requires (is_callable<Fn&&, T()>)
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
            return fallback();
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

    // mapper: T -> T'
    template <
        typename Fn,
        typename Out = return_type<Fn&&, Sig<_, Ref<T const>>>
    >
    constexpr Opt<Out, E> map(Fn&& mapper) const& 
    requires (!same_type<Out, NAUR> && (same_type<T, void> || is_copy_constructible<T>))
    {
        if (isOK())
        {
            if constexpr (!same_type<Out, void> && !same_type<T, void> && !is_lvalue_reference<T>)
                return { True{}, mapper(value) };
            else if constexpr (!same_type<Out, void> && !same_type<T, void> && is_lvalue_reference<T>)
                return { True{}, mapper(*ptr) };
            else if constexpr (!same_type<Out, void> && same_type<T, void>)
                return { True{}, mapper() };
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

    // mapper: T -> T'
    template <
        typename Fn,
        typename Out = return_type<Fn&&, Sig<_, Rvalue<T>>>
    >
    constexpr Opt<Out, E> map(Fn&& mapper) && 
    requires (!same_type<Out, NAUR>)
    {
        if (isOK())
        {
            if constexpr (!same_type<Out, void> && !same_type<T, void> && !is_lvalue_reference<T>)
                return { True{}, mapper(static_cast<T&&>(value)) };
            else if constexpr (!same_type<Out, void> && !same_type<T, void> && is_lvalue_reference<T>)
                return { True{}, mapper(*ptr) };
            else if constexpr (!same_type<Out, void> && same_type<T, void>)
                return { True{}, mapper() };
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

    // mapper: E -> E'
    template <
        typename Fn,
        typename Out = return_type<Fn&&, Sig<_, Ref<E const>>>
    >
    constexpr Opt<T, Out> map_err(Fn&& mapper) const& 
    requires (!same_type<Out, NAUR> && (same_type<T, void> || is_copy_constructible<T>))
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
                return { False{}, mapper(error) };
            else if constexpr (!same_type<Out, void> && same_type<E, void>)
                return { False{}, mapper() };
            else
                return { False{} };
        }
    }

    // mapper: E -> E'
    template <
        typename Fn,
        typename Out = return_type<Fn&&, Sig<_, Rvalue<E>>>
    >
    constexpr Opt<T, Out> map_err(Fn&& mapper) && 
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
                return { False{}, mapper(static_cast<E&&>(error)) };
            else if constexpr (!same_type<Out, void> && same_type<E, void>)
                return { False{}, mapper() };
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

    // chain: T -> Opt<T', E>
    template <
        typename Fn,
        typename Out = return_type<Fn&&, Sig<_, Ref<T const>>>
    >
    constexpr Out and_then(Fn&& chain) const&
    requires (
        !same_type<typename AsOpt<Out>::val, NAUR> && 
        same_type<typename AsOpt<Out>::err, E> &&
        (same_type<T, void> || is_copy_constructible<T>))
    {
        if (isOK())
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
                return chain(value);
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return chain(*ptr);
            else
                return chain();
        }
        else
        {
            if constexpr (!same_type<E, void>)
                return { False{}, error };
            else
                return { False{} };
        }
    }

    // chain: T -> Opt<T', E>
    template <
        typename Fn,
        typename Out = return_type<Fn&&, Sig<_, Rvalue<T>>>
    >
    constexpr Out and_then(Fn&& chain) &&
    requires (
        !same_type<typename AsOpt<Out>::val, NAUR> && 
        same_type<typename AsOpt<Out>::err, E>)
    {
        if (isOK())
        {
            if constexpr (!same_type<T, void> && !is_lvalue_reference<T>)
                return chain(static_cast<T&&>(value));
            else if constexpr (!same_type<T, void> && is_lvalue_reference<T>)
                return chain(*ptr);
            else
                return chain();
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

    // err_chain: E -> Opt<T, E'>
    template <
        typename Fn,
        typename Out = return_type<Fn&&, Sig<_, Ref<E const>>>
    >
    constexpr Out or_else(Fn&& err_chain) const&
    requires (
        same_type<typename AsOpt<Out>::val, T> &&
        !same_type<typename AsOpt<Out>::err, NAUR> &&
        (same_type<T, void> || is_copy_constructible<T>))
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
                return err_chain(error);
            else
                return err_chain();
        }
    }

    // err_chain: E -> Opt<T, E'>
    template <
        typename Fn,
        typename Out = return_type<Fn&&, Sig<_, Rvalue<E>>>
    >
    constexpr Out or_else(Fn&& err_chain) &&
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
                return err_chain(static_cast<E&&>(error));
            else
                return err_chain();
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////// fn: filter ///////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    // predicate: T -> bool
    template <typename Fn>
    constexpr Opt filter(Fn&& predicate) const&
    requires (
        same_type<E, void> && 
        is_callable<Fn&&, Sig<bool, Ref<T const>>> &&
        !same_type<T, void> && is_copy_constructible<T>)
    {
        if constexpr (!is_lvalue_reference<T>)
        {
            if (isOK() && predicate(value)) 
                return { True{}, value };
            else
                return { False{} };
        }
        else
        {
            if (isOK() && predicate(*ptr)) 
                return { True{}, *ptr };
            else
                return { False{} };
        }
    }

    // predicate: T -> bool
    template <typename Fn>
    constexpr Opt filter(Fn&& predicate) &&
    requires (
        same_type<E, void> && 
        is_callable<Fn&&, Sig<bool, Ref<T const>>> &&
        !same_type<T, void>)
    {
        if constexpr (!is_lvalue_reference<T>)
        {
            if (isOK() && predicate(value)) 
                return { True{}, static_cast<T&&>(value) };
            else
                return { False{} };
        }
        else
        {
            if (isOK() && predicate(*ptr)) 
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

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

_MNC_END

#undef _MNC_BEGIN
#undef _MNC_END
