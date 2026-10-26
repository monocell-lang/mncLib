#pragma once


#define _MNC_BEGIN namespace mnc {
#define _MNC_END }

_MNC_BEGIN

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef float f32;
typedef double f64;
typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned long long ulong;
typedef unsigned short ushort;
typedef char32_t wchar;

#ifdef __SIZEOF_FLOAT128__
typedef __float128 f128;
typedef __float128 quad;
#endif

consteval i8 operator""_i8(unsigned long long _)  { return static_cast<i8>(_); }
consteval i16 operator""_i16(unsigned long long _) { return static_cast<i16>(_); }
consteval i32 operator""_i32(unsigned long long _) { return static_cast<i32>(_); }
consteval i64 operator""_i64(unsigned long long _) { return static_cast<i64>(_); }
consteval u8 operator""_u8(unsigned long long _)  { return static_cast<u8>(_); }
consteval u16 operator""_u16(unsigned long long _) { return static_cast<u16>(_); }
consteval u32 operator""_u32(unsigned long long _) { return static_cast<u32>(_); }
consteval u64 operator""_u64(unsigned long long _) { return static_cast<u64>(_); }

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region special types

/*
*   class: True
*   - Cùng với False, là tag ban đầu sinh ra để phục vụ các class Option<T> và Result<T, E>
*   - Sau này được sử dụng cho cả kiểu đại diện cho giá trị bool ở các type trait
*   nơi việc sử dụng bool gây bất tiện
*
*   Sử dụng:
*   - Dùng chủ yếu trong các class Option<T> và Result<T, E>
*/

struct True {
    explicit True() = default;
    static constexpr bool value = true;
};

/*
*   class: False
*   - Cùng với True, là tag ban đầu sinh ra để phục vụ các class Option<T> và Result<T, E>
*   - Sau này được sử dụng cho cả kiểu đại diện cho giá trị bool ở các type trait
*   nơi việc sử dụng bool gây bất tiện
*
*   Sử dụng:
*   - Dùng chủ yếu trong các class Option<T> và Result<T, E>
*/

struct False {
    explicit False() = default;
    static constexpr bool value = false;
};


_MNC_END
/*
*   class: _
*   - Một class đại diện cho việc bỏ qua type
*
*   Sử dụng:
*   - Thường thấy trong việc xác định trait "is_callable<T, Signature = Ret(Args...)>".
*   Trong đó, Signature có thể có Ret = _ để bỏ qua việc xác định kiểu trả về
*   - Sử dụng trong hàm Vena::match()
*
*   Lưu ý:
*   - không sử dụng _ cho mục để khai báo biến vì có thể làm rối loạn logic 
*   của các thành phần khác
*/

struct _ {
    explicit _() = default;
};
_MNC_BEGIN

/*
*   class: NAUR
*   - Biểu thị việc kết quả tính toán kiểu dữ liệu trả về lỗi
*
*   Sử dụng:
*   - Thường thấy trong type trait
*   - Có tác dụng thay thế lỗi cứng không thể khắc phục giống như "enable_if<false>"
*
*   Lưu ý:
*   - Không sử dụng NAUR để tạo object
*   - Nó chỉ đóng vai trò như cái nhãn "lỗi không thể xác định kiểu dữ liệu"
*/

struct NAUR;

/*
*   class: PACKAGE
*   - Một class đặc biệt được dùng để chứa mã nguồn nội bộ
*   - Cho phép giấu chi tiết thực thi bên trong trường private
*
*   Sử dụng:
*   - Tạo một specialization mới của PACKAGE
*   - Mỗi PACKAGE nên có một ID riêng (thường tạo ra bởi "ID"_hash)
*
*   Lưu ý:
*   - Không sử dụng PACKAGE để tạo object
*   - Nó chỉ có vai trò như một namesapce có tính đóng gói và template toàn cục
*/

template <ulong ID>
class PACKAGE;

#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region same_type


// same type
template <typename T, typename S>
struct struct_same_type {
    static constexpr bool value = false;
};

template <typename T>
struct struct_same_type<T, T> {
    static constexpr bool value = true;
};


template <typename... Ts>
static constexpr bool same_type = true;

template <typename T, typename... U>
static constexpr bool same_type<T, U...> = (struct_same_type<T, U>::value && ...);


// different types
template <typename... Ts>
static constexpr bool different_types = false;

template <typename First, typename... Rest>
static constexpr bool different_types<First, Rest...> = (!same_type<First, Rest> && ...) && different_types<Rest...>;

template <typename First, typename Rest>
static constexpr bool different_types<First, Rest> = !same_type<First, Rest>;


// is_specialization_of
template <typename T, template <typename...> typename Template_Type>
static constexpr bool is_specialization_of = false;

template <typename... Ts, template <typename...> typename Template_Type>
static constexpr bool is_specialization_of<Template_Type<Ts...>, Template_Type> = true;


#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region enable_if


/*  
*   class: template_pass
*   - class được enable_if trả về khi điều kiện bên trong được thỏa mãn
*   
*   Lưu ý:
*   - Không dùng template_pass để tạo object
*   - Nó chỉ đóng vai trò như cái nhãn "điều kiện đã thỏa mãn"
*   => Nếu enable_if trả về int thì chúng ta sẽ dùng int thay thế cho template_pass
*/
struct template_pass;

template <bool condition>
struct struct_enable_if;

template <>
struct struct_enable_if <true> {
    using type = template_pass;
};

template <bool condition>
using enable_if = typename struct_enable_if<condition>::type;


#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region conditional


// type_if
template <bool Condition, typename Type>
struct struct_type_if;

template <typename Type>
struct struct_type_if <true, Type> {
    using type = Type;
};

template <bool Condition, typename Type>
using type_if = typename struct_type_if<Condition, Type>::type;


// switch type
template <bool Condition, typename Type_If_True, typename Type_If_False>
struct struct_switch_type;

template <typename Type_If_True, typename Type_If_False>
struct struct_switch_type<true, Type_If_True, Type_If_False> {
    using type = Type_If_True;
};

template <typename Type_If_True, typename Type_If_False>
struct struct_switch_type<false, Type_If_True, Type_If_False> {
    using type = Type_If_False;
};

template <bool Condition, typename Type_If_True, typename Type_If_False>
using switch_type = typename struct_switch_type<Condition, Type_If_True, Type_If_False>::type;


#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region basic remove


template <typename T>
struct struct_remove_lvalue_reference {
    using type = T;
};

template <typename T>
struct struct_remove_lvalue_reference<T&> {
    using type = T;
};

template <typename T>
using remove_lvalue_reference = typename struct_remove_lvalue_reference<T>::type;

template <typename T>
struct struct_remove_rvalue_referece {
    using type = T;
};

template <typename T>
struct struct_remove_rvalue_referece<T&&> {
    using type = T;
};

template <typename T>
using remove_rvalue_referece = typename struct_remove_rvalue_referece<T>::type;

template <typename T>
struct struct_remove_reference {
    using type = T;
};

template <typename T>
struct struct_remove_reference<T&> {
    using type = T;
};

template <typename T>
struct struct_remove_reference<T&&> {
    using type = T;
};

template <typename T>
using remove_reference = typename struct_remove_reference<T>::type;

template <typename T>
struct struct_remove_const {
    using type = T;
};

template <typename T>
struct struct_remove_const<T const> {
    using type = T;
};

template <typename T>
using remove_const = typename struct_remove_const<T>::type;

template <typename T>
struct struct_remove_volatile {
    using type = T;
};

template <typename T>
struct struct_remove_volatile<T volatile> {
    using type = T;
};

template <typename T>
using remove_volatile = typename struct_remove_volatile<T>::type;

template <typename T>
using remove_const_volatile = remove_const<remove_volatile<T>>;

template <typename T>
using remove_const_reference = remove_const<remove_reference<T>>;

template <typename T>
struct struct_remove_pointer {
    using type = T;
};

template <typename T>
struct struct_remove_pointer<T*> {
    using type = T;
};

template <typename T>
using remove_pointer = typename struct_remove_pointer<T>::type;

template <typename T>
struct struct_remove_class_ptr {
    using type = T;
};

template <typename T, typename class_type>
struct struct_remove_class_ptr<T class_type::*> {
    using type = T;
};

template <typename T>
using remove_class_ptr = typename struct_remove_class_ptr<T>::type;

template <typename T>
struct struct_remove_array {
    using type = T;
};

template <typename T>
struct struct_remove_array<T[]> {
    using type = T;
};

template <typename T, int N>
struct struct_remove_array<T[N]> {
    using type = T;
};

template <typename T>
using remove_array = typename struct_remove_array<T>::type;

template <typename T>
struct struct_remove_param {
    using type = T;
};

template <typename T, typename... Arg>
struct struct_remove_param<T(Arg...)> {
    using type = T;
};

template <typename T, typename... Arg>
struct struct_remove_param<T(Arg..., ...)> {
    using type = T;
};

template <typename T>
using remove_param = typename struct_remove_param<T>::type;



#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region decay


template <typename T>
struct struct_decay {
    using type = remove_const_volatile<T>;
};

template <typename T>
struct struct_decay<T&> {
    using type = typename struct_decay<T>::type;
};

template <typename T>
struct struct_decay<T&&> {
    using type = typename struct_decay<T>::type;
};

template <typename T>
struct struct_decay<T[]> {
    using type = typename struct_decay<T*>::type;
};

template <typename T, int N>
struct struct_decay<T[N]> {
    using type = typename struct_decay<T*>::type;
};

template <typename Ret, typename... Arg>
struct struct_decay<Ret(Arg...)> {
    using type = typename struct_decay<Ret(*)(Arg...)>::type;
};

template <typename Ret, typename... Arg>
struct struct_decay<Ret(Arg..., ...)> {
    using type = typename struct_decay<Ret(*)(Arg..., ...)>::type;
};

template <typename T>
using decay = typename struct_decay<T>::type;


#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region basic traits

template <typename T>
static constexpr bool is_signed_int =
same_type<T, signed char>  || 
same_type<T, signed short> || 
same_type<T, signed int>   || 
same_type<T, signed long>  || 
same_type<T, signed long long>;

template <typename T>
static constexpr bool is_unsigned_int =
same_type<T, unsigned char>  ||
same_type<T, unsigned short> || 
same_type<T, unsigned int>   || 
same_type<T, unsigned long>  || 
same_type<T, unsigned long long>;

template <typename T>
static constexpr bool is_float =
same_type<T, float>  || 
same_type<T, double> || 
same_type<T, long double>;

template <typename T>
static constexpr bool is_bool = same_type<T, bool>;

template <typename T>
static constexpr bool is_char = 
same_type<T, char>      ||
same_type<T, wchar_t>   ||
same_type<T, char16_t>  ||
same_type<T, char32_t>;

template <typename T>
static constexpr bool is_void = same_type<T, void>;

template <typename T>
static constexpr bool is_int = is_signed_int<T> || is_unsigned_int<T>;

template <typename T>
struct struct_is_lvalue_reference {
    static constexpr bool value = false;
};

template <typename T>
struct struct_is_lvalue_reference<T&> {
    static constexpr bool value = true;
};

template <typename T>
static constexpr bool is_lvalue_reference = struct_is_lvalue_reference<T>::value;

template <typename T>
struct struct_is_const_lvalue_reference {
    static constexpr bool value = false;
};

template <typename T>
struct struct_is_const_lvalue_reference<T const&> {
    static constexpr bool value = true;
};

template <typename T>
static constexpr bool is_const_lvalue_reference = struct_is_const_lvalue_reference<T>::value;

template <typename T>
struct struct_is_nonconst_lvalue_reference {
    static constexpr bool value = false;
};

template <typename T>
struct struct_is_nonconst_lvalue_reference<T&> {
    static constexpr bool value = true;
};

template <typename T>
struct struct_is_nonconst_lvalue_reference<T const&> {
    static constexpr bool value = false;
};

template <typename T>
static constexpr bool is_nonconst_lvalue_reference = struct_is_nonconst_lvalue_reference<T>::value;

template <typename T>
struct struct_is_rvalue_reference {
    static constexpr bool value = false;
};

template <typename T>
struct struct_is_rvalue_reference<T&&> {
    static constexpr bool value = true;
};

template <typename T>
static constexpr bool is_rvalue_reference = struct_is_rvalue_reference<T>::value;

template <typename T>
struct struct_is_const_rvalue_reference {
    static constexpr bool value = false;
};

template <typename T>
struct struct_is_const_rvalue_reference<T const&&> {
    static constexpr bool value = true;
};

template <typename T>
static constexpr bool is_const_rvalue_reference = struct_is_const_rvalue_reference<T>::value;

template <typename T>
struct struct_is_nonconst_rvalue_reference {
    static constexpr bool value = false;
};

template <typename T>
struct struct_is_nonconst_rvalue_reference<T&&> {
    static constexpr bool value = true;
};

template <typename T>
struct struct_is_nonconst_rvalue_reference<T const&&> {
    static constexpr bool value = false;
};

template <typename T>
static constexpr bool is_nonconst_rvalue_reference = struct_is_nonconst_rvalue_reference<T>::value;

template <typename T>
static constexpr bool is_reference = is_lvalue_reference<T> || is_rvalue_reference<T>;

template <typename T>
static constexpr bool is_const_reference = is_const_lvalue_reference<T> || is_const_rvalue_reference<T>;

template <typename T>
static constexpr bool is_nonconst_reference = is_nonconst_lvalue_reference<T> || is_nonconst_rvalue_reference<T>;

template <typename T>
struct struct_is_const {
    static constexpr bool value = false;
};

template <typename T>
struct struct_is_const<T const> {
    static constexpr bool value = true;
};

template <typename T>
static constexpr bool is_const = struct_is_const<T>::value;

template <typename T>
struct struct_is_volatile {
    static constexpr bool value = false;
};

template <typename T>
struct struct_is_volatile<T volatile> {
    static constexpr bool value = true;
};

template <typename T>
static constexpr bool is_volatile = struct_is_volatile<T>::value;

template <typename T>
struct struct_is_pointer {
    static constexpr bool value = false;
};

template <typename T>
struct struct_is_pointer<T*> {
    static constexpr bool value = true;
};

template <typename T>
static constexpr bool is_pointer = struct_is_pointer<remove_const_volatile<T>>::value;

template <typename T>
struct struct_is_const_pointer {
    static constexpr bool value = false;
};

template <typename T>
struct struct_is_const_pointer<T const*> {
    static constexpr bool value = true;
};

template <typename T>
static constexpr bool is_const_pointer = struct_is_const_pointer<remove_const_volatile<T>>::value;

template <typename T>
struct struct_is_nonconst_pointer {
    static constexpr bool value = false;
};

template <typename T>
struct struct_is_nonconst_pointer<T*> {
    static constexpr bool value = true;
};

template <typename T>
struct struct_is_nonconst_pointer<T const*> {
    static constexpr bool value = false;
};

template <typename T>
static constexpr bool is_nonconst_pointer = struct_is_nonconst_pointer<remove_const_volatile<T>>::value;

template <typename T>
struct struct_is_class_ptr {
    static constexpr bool value = false;
};

template <typename T, typename class_type>
struct struct_is_class_ptr<T class_type::*> {
    static constexpr bool value = true;
};

template <typename T>
static constexpr bool is_class_ptr = struct_is_class_ptr<T>::value;

template <typename T>
struct struct_is_array {
    static constexpr bool value = false;
};

template <typename T, int N>
struct struct_is_array<T[N]> {
    static constexpr bool value = true;
};

template <typename T>
static constexpr bool is_array = struct_is_array<T>::value;

template <typename T>
struct struct_is_unbounded_array {
    static constexpr bool value = false;
};

template <typename T>
struct struct_is_unbounded_array<T[]> {
    static constexpr bool value = true;
};

template <typename T>
static constexpr bool is_unbounded_array = struct_is_unbounded_array<T>::value;

template <typename T>
struct struct_is_function {
    static constexpr bool value = false;
};

template <typename Ret, typename... Param>
struct struct_is_function<Ret(Param...)> {
    static constexpr bool value = true;
};

template <typename T>
static constexpr bool is_function = struct_is_function<T>::value;

template <typename T>
static constexpr bool is_class = __is_class(T);

template <typename T>
static constexpr bool is_struct = __is_class(T);

template <typename T>
static constexpr bool is_aggregate_struct = __is_aggregate(T);

template <typename T>
static constexpr bool is_empty_class = __is_empty(T);

template <typename T>
static constexpr bool is_empty = __is_empty(T);

template <typename T>
static constexpr bool is_enum = __is_enum(T);

template <typename T>
static constexpr bool is_union = __is_union(T);

template <typename T>
static constexpr bool is_polymorphic_class = __is_polymorphic(T);

template <typename T>
static constexpr bool is_inheritable_class = !__is_final(T);


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region basic add

template <typename T>
constexpr T const& add_const(T& value) { return value; }

template <typename T>
constexpr T const&& add_const(T&& value) { return value; }

template <typename T>
constexpr T const& add_volatile(T& value) { return value; }

template <typename T>
constexpr T const&& add_volatile(T&& value) { return value; }


template <typename T, typename = template_pass>
struct struct_lvalue_reference {
    using type = T&;
};

template <typename T>
struct struct_lvalue_reference<T, enable_if<same_type<remove_const<T>, void>>> {
    using type = T;
};

template <typename T>
using lvalue_reference = typename struct_lvalue_reference<T>::type;

template <typename T, typename = template_pass>
struct struct_rvalue_reference {
    using type = T&&;
};

template <typename T>
struct struct_rvalue_reference<T, enable_if<same_type<remove_const<T>, void>>> {
    using type = T;
};

template <typename T>
using rvalue_reference = typename struct_rvalue_reference<T>::type;


#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region function mod


template <typename T>
struct struct_remove_fn_mod {
    using type = T;
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg...) const> {
    using type = Ret(Arg...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg...) volatile> {
    using type = Ret(Arg...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg...) const volatile> {
    using type = Ret(Arg...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg...) &> {
    using type = Ret(Arg...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg...) const &> {
    using type = Ret(Arg...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg...) volatile &> {
    using type = Ret(Arg...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg...) const volatile &> {
    using type = Ret(Arg...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg...) &&> {
    using type = Ret(Arg...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg...) const &&> {
    using type = Ret(Arg...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg...) volatile &&> {
    using type = Ret(Arg...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg...) const volatile &&> {
    using type = Ret(Arg...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg..., ...) const> {
    using type = Ret(Arg..., ...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg..., ...) volatile> {
    using type = Ret(Arg..., ...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg..., ...) const volatile> {
    using type = Ret(Arg..., ...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg..., ...) &> {
    using type = Ret(Arg..., ...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg..., ...) const &> {
    using type = Ret(Arg..., ...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg..., ...) volatile &> {
    using type = Ret(Arg..., ...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg..., ...) const volatile &> {
    using type = Ret(Arg..., ...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg..., ...) &&> {
    using type = Ret(Arg..., ...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg..., ...) const &&> {
    using type = Ret(Arg..., ...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg..., ...) volatile &&> {
    using type = Ret(Arg..., ...);
};

template <typename Ret, typename... Arg>
struct struct_remove_fn_mod<Ret(Arg..., ...) const volatile &&> {
    using type = Ret(Arg..., ...);
};

template <typename T>
using remove_fn_mod = typename struct_remove_fn_mod<T>::type;



#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region is_fn


template <typename T>
struct struct_is_fn {
    static constexpr bool value = false;
};

template <typename Ret, typename... Arg>
struct struct_is_fn<Ret(Arg...)> {
    static constexpr bool value = true;
};

template <typename Ret, typename... Arg>
struct struct_is_fn<Ret(Arg..., ...)> {
    static constexpr bool value = true;
};

template <typename T>
static constexpr bool is_fn = struct_is_fn<T>::value;

template <typename T>
static constexpr bool is_extended_fn = struct_is_fn<remove_fn_mod<T>>::value;


template <typename T>
struct struct_is_pointer_to_field {
    static constexpr bool value = false;
};

template <typename T, typename class_type>
struct struct_is_pointer_to_field<T class_type::*> {
    static constexpr bool value = !is_extended_fn<T>;
};

template <typename T>
static constexpr bool is_pointer_to_field = struct_is_pointer_to_field<T>::value;


#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region assert_exression


/*  
*   class: assert_exression
*   - Dùng để thực hiện các kiểm định SFINAE
*   - Trong đó, Mỗi kiểm định kiểm tra tính hợp lệ (validity) của một biểu thức tại thời điểm biến dịch (compile-time)
*   - Nếu tất cả biểu thức đều hợp lệ, trả về class template_pass (enable_if cũng trả về template_pass)
*
*   Sử dụng:
*   - Thường đi chung với decltype(...) và declval<T>().
*
*   Cơ chế:
*   - Nếu mỗi biểu thức hợp lệ, decltype sẽ truy được kiểu của nó
*   - Lúc đó, class assert_exression sẽ gom các kiểu đó lại thành class template_pass (nghĩa là từ "type gì" thành "có type")
*   => Nếu muốn biểu thức trả về kiểu cụ thể thì đừng sử dụng class assert_exression
*
*   Lưu ý:
*   - Không sử dụng assert_exression để khởi tạo object
*/

template <typename... Req>
using assert_exression = template_pass;


/*  
*   fn: declval<T>()
*   - Dùng để tạo một giá trị của T ngay lập tức mà không cần biết giá trị đó là gì
*
*   Lưu ý:
*   - Không gọi declval<T>() trực tiếp. Chỉ gọi nó trong decltype(...)
*
*   Ghi chú:
*   - Hàm declval<T>() trước đây có kiểu trả về là T&&. Sau sự kiện "Void Fix" xảy ra do void&& bị cấm,
*   kiểu trả về của hàm đã được sửa lại sao cho trả về void ngay khi T = void
*/

template <typename T>
T&& declval() noexcept;


#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region is_castible


template <typename Dest, typename Source, typename = template_pass>
struct struct_is_castible_from {
    static constexpr bool value = false;
};

template <typename Dest, typename Source>
struct struct_is_castible_from<Dest, Source, assert_exression<decltype(static_cast<Dest>(declval<Source>()))>> {
    static constexpr bool value = true;
};

template <typename Dest, typename Source>
static constexpr bool is_castible_from = struct_is_castible_from<Dest, Source>::value;

template <typename Dest, typename Source, typename = template_pass>
struct struct_is_implicitly_castible_from {
    static constexpr bool value = false;
};

template <typename Dest, typename Source>
struct struct_is_implicitly_castible_from<Dest, Source, assert_exression<decltype(declval<void(*)(Dest)>()(declval<Source>()))>> {

    // Cách đúng là kiểm tra "Dest x = declval<Source>()"
    // NHƯNG không thể ghi decltype(Dest x = declval<Source>())
    // => Dùng hàm là cách duy nhất để lách luật
    
    static constexpr bool value = true;
};

template <typename Dest, typename Source>
static constexpr bool is_implicitly_castible_from = struct_is_implicitly_castible_from<Dest, Source>::value;

template <typename Dest, typename Source>
static constexpr bool is_convertible_from = is_implicitly_castible_from<Dest, Source>;

template <typename Derived, typename Base>
static constexpr bool is_derived_from = __is_base_of(Base, Derived);

template <typename Base, typename Derived>
static constexpr bool is_base_of = __is_base_of(Base, Derived);

#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region is_callable


template <typename T, typename Signature, typename = template_pass>
struct struct_is_callable { 
    static constexpr bool value = false;
    using return_type = NAUR;
};

template <typename T, typename Ret, typename... Args> 
struct struct_is_callable<T, Ret(Args...), enable_if<same_type<decltype(declval<T>()(declval<Args>()...)), Ret>>> { 
    static constexpr bool value = true;
    using return_type = Ret;
};

template <typename T, typename... Args> 
struct struct_is_callable<T, _(Args...), assert_exression<decltype(declval<T>()(declval<Args>()...))>> { 
    static constexpr bool value = true;
    using return_type = decltype(declval<T>()(declval<Args>()...));
};

template <typename T, typename Signature>
static constexpr bool is_callable = struct_is_callable<T, Signature>::value;

template <typename T, typename Signature>
using return_type = typename struct_is_callable<T, Signature>::return_type;


#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region is_constructible

template <typename T, typename = template_pass>
struct struct_is_implicitly_default_constructible {
    static constexpr bool value = false;
    static constexpr bool value_noexcept = false;
};

template <typename T>
struct struct_is_implicitly_default_constructible<T, assert_exression<decltype(declval<void(*)(T)>()({}))>> {
    static constexpr bool value = true;
    static constexpr bool value_noexcept = noexcept(declval<void(*)(T)>()({}));
};

template <typename T>
static constexpr bool is_implicitly_default_constructible = struct_is_implicitly_default_constructible<T>::value;

template <typename T>
static constexpr bool is_noexcept_implicitly_default_constructible = struct_is_implicitly_default_constructible<T>::value_noexcept;


template <typename T, typename... Args>
static constexpr bool is_constructible_from = __is_constructible(T, Args...);

template <typename T>
static constexpr bool is_default_constructible = __is_constructible(T);

template <typename T>
static constexpr bool is_copy_constructible = __is_constructible(T, T const&);

template <>
inline constexpr bool is_copy_constructible<void> = __is_constructible(void, void);

template <>
inline constexpr bool is_copy_constructible<void const> = __is_constructible(void const, void const);

template <typename T>
static constexpr bool is_move_constructible = __is_constructible(T, T&&);

template <>
inline constexpr bool is_move_constructible<void> = __is_constructible(void, void);

template <typename T, typename S>
static constexpr bool is_assignable_from = __is_assignable(T&, S);

template <typename T>
static constexpr bool is_copy_assignable = __is_assignable(T&, T const&);

template <>
inline constexpr bool is_copy_assignable<void> = __is_assignable(void, void);

template <typename T>
static constexpr bool is_move_assignable = __is_assignable(T&, T&&);

template <>
inline constexpr bool is_move_assignable<void> = __is_assignable(void, void);

template <typename T>
static constexpr bool is_copy_able = is_copy_constructible<T> && is_copy_assignable<T>;

template <typename T>
static constexpr bool is_move_able = is_move_constructible<T> && is_move_assignable<T>;

template <typename T>
static constexpr bool is_aggregate = __is_aggregate(T);


#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region is_constructible

template <typename T, typename... Args>
static constexpr bool is_noexcept_constructible_from = __is_nothrow_constructible(T, Args...);

template <typename T>
static constexpr bool is_noexcept_default_constructible = __is_nothrow_constructible(T);

template <typename T>
static constexpr bool is_noexcept_copy_constructible = __is_nothrow_constructible(T, T const&);

template <typename T>
static constexpr bool is_noexcept_move_constructible = __is_nothrow_constructible(T, T&&);

template <typename T>
static constexpr bool is_noexcept_copy_assignable = __is_nothrow_assignable(T&, T const&);

template <typename T>
static constexpr bool is_noexcept_move_assignable = __is_nothrow_assignable(T&, T&&);

template <typename T>
static constexpr bool is_noexcept_destructible = noexcept(declval<T&>().~T());

template <>
inline constexpr bool is_noexcept_destructible<void> = false;

template <typename T>
static constexpr bool is_noexcept_resource = 
(!is_default_constructible<T> || is_noexcept_default_constructible<T>) &&
(!is_copy_constructible<T>    || is_noexcept_copy_constructible<T>)    &&
(!is_move_constructible<T>    || is_noexcept_move_constructible<T>)    &&
(!is_copy_assignable<T>       || is_noexcept_copy_assignable<T>)       &&
(!is_move_assignable<T>       || is_noexcept_move_assignable<T>)       &&
(/* destructible */ !true     || is_noexcept_destructible<T>);

#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region is_trivials

template <typename T, typename... Args>
static constexpr bool is_trivially_constructible_from = __is_trivially_constructible(T, Args...);

template <typename T>
static constexpr bool is_trivially_default_constructible = __is_trivially_constructible(T);

template <typename T>
static constexpr bool is_trivially_copy_constructible = __is_trivially_constructible(T, T const&);

template <>
inline constexpr bool is_trivially_copy_constructible<void> = __is_trivially_constructible(void, void);

template <typename T>
static constexpr bool is_trivially_copy_assignable = __is_trivially_assignable(T&, T const&);

template <>
inline constexpr bool is_trivially_copy_assignable<void> = __is_trivially_assignable(void, void);

template <typename T>
static constexpr bool is_trivially_copy_able = is_trivially_copy_constructible<T> && is_trivially_copy_assignable<T>;

template <typename T>
static constexpr bool is_trivially_move_constructible = __is_trivially_constructible(T, T&&);

template <>
inline constexpr bool is_trivially_move_constructible<void> = __is_trivially_constructible(void, void);

template <typename T>
static constexpr bool is_trivially_move_assignable = __is_trivially_assignable(T&, T&&);

template <>
inline constexpr bool is_trivially_move_assignable<void> = __is_trivially_assignable(void, void);

template <typename T>
static constexpr bool is_trivially_move_able = is_trivially_move_constructible<T> && is_trivially_move_assignable<T>;

template <typename T>
struct struct_is_trivially_destructible 
{
#if defined(__has_builtin)  // 1. Try standard GCC/Clang intrinsic

    #if __has_builtin(__is_trivially_destructible)
        static constexpr bool value = __is_trivially_destructible(T);
    #else
        static constexpr bool value = false;
    #endif

    
#elif defined(_MSC_VER)     // 2. Try MSVC builtins

    #if _MSC_VER >= 1900
        static constexpr bool value = __is_trivially_destructible(T);
    #else
        static constexpr bool value = __has_trivial_destructor(T);
    #endif

#else                       // 3. Unknown compiler fallback

    static constexpr bool value = false; 

#endif
};

template <typename T>
static constexpr bool is_trivially_destructible = struct_is_trivially_destructible<T>::value;

#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region has_operator




#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region index sequence


// index sequence
template <int... index>
struct index_sequence {};

struct struct_index_sequence_init {
private:

    struct version_01
    {
        struct A
        {
            template <int... index>
            struct struct_index_sequence_init;

            template <int first_index, int... rest_index>
            struct struct_index_sequence_init<first_index, rest_index...> {
                using type = typename struct_index_sequence_init<first_index - 1, first_index, rest_index...>::type;
            };

            template <int... rest_index>
            struct struct_index_sequence_init<0, rest_index...> {
                using type = index_sequence<0, rest_index...>;
            };

            template <>
            struct struct_index_sequence_init<-1> {
                using type = index_sequence<>;
            };

            template <int size>
            using index_sequence_init = typename struct_index_sequence_init<size - 1>::type;
        };

        struct B
        {
            template <int... index>
            struct struct_index_sequence_init;

            template <int first_index, int... rest_index>
            struct struct_index_sequence_init<first_index, rest_index...> {
                using type = typename struct_index_sequence_init<first_index - 1, first_index, rest_index...>::type;
            };

            template <int... rest_index>
            struct struct_index_sequence_init<-1, rest_index...> {
                using type = index_sequence<rest_index...>;
            };

            template <int size>
            using index_sequence_init = typename struct_index_sequence_init<size - 1>::type;
        };
    };

    struct version_02
    {
        template <int size, int i, int... index>
        struct struct_index_sequence_init {
            using type = typename struct_index_sequence_init<size, i + 1, index..., i>::type;
        };

        template <int size, int... index>
        struct struct_index_sequence_init<size, size, index...> {
            using type = index_sequence<index...>;
        };

        template <int size>
        using index_sequence_init = typename struct_index_sequence_init<size, 0>::type;
    };

    struct version_03
    {
        template <typename left_seq, typename right_seq>
        struct stack_up_sequence;

        template <int... I1, int... I2>
        struct stack_up_sequence<index_sequence<I1...>, index_sequence<I2...>> {
            static constexpr int N = sizeof...(I1);
            using type = index_sequence<I1..., (N + I2)...>;
        };

        template <int size>
        struct struct_index_sequence_init {
            using type = typename stack_up_sequence<
                typename struct_index_sequence_init<size / 2>::type,
                typename struct_index_sequence_init<size - size / 2>::type
            >::type;
        };

        template <>
        struct struct_index_sequence_init<0> {
            using type = index_sequence<>;
        };

        template <>
        struct struct_index_sequence_init<1> {
            using type = index_sequence<0>;
        };

        template <int size>
        using index_sequence_init = typename struct_index_sequence_init<size>::type;
    };

public:

    template <int size>
    using type = version_02::index_sequence_init<size>;
};

template <int size>
using index_sequence_init = struct_index_sequence_init::type<size>;


#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma region variadic

// max size, max align
template <typename... Ts>
static constexpr int max_size = []()
{
    if constexpr (sizeof...(Ts) == 0)
    {
        return 0;
    }
    else
    {
        int sizes[sizeof...(Ts)] = { sizeof(Ts)... };

        int max = 0;
        for (auto i: sizes)
        {
            if (i > max)
                max = i;
        }

        return max;
    }
}();

template <typename... Ts>
static constexpr int max_align = []()
{
    if constexpr (sizeof...(Ts) == 0)
    {
        return 0;
    }
    else
    {
        int aligns[sizeof...(Ts)] = { alignof(Ts)... };

        int max = 0;
        for (auto i: aligns)
        {
            if (i > max)
                max = i;
        }

        return max;
    }
}();


// type_at
struct struct_type_at {
private:

    struct version_01
    {
        template <int index, typename... Types>
        struct struct_type_at {
            using type = NAUR;
            // make instatiation successful
            // later limit index range with enable_if
            
            // (There's nothing unreachable template instatiation. 
            // Every branch, used or unused is instantiated)
        };

        template <int index, typename First_Type, typename... Type_Rest>
        struct struct_type_at<index, First_Type, Type_Rest...> {
            using type = typename struct_type_at<index - 1, Type_Rest...>::type;
        };

        template <typename First_Type, typename... Type_Rest>
        struct struct_type_at<0, First_Type, Type_Rest...> {
            using type = First_Type;
        };

        template <int index, typename... Types>
        using type_at = typename struct_type_at<index, Types...>::type;
    };

    struct version_02_DEMO
    {
        template <int index, typename T>
        struct Type_Holder {};

        template <typename index_sequence, typename... Types>
        struct Type_Indexer;

        template <int... index, typename... Types>
        struct Type_Indexer<index_sequence<index...>, Types...>
        : public Type_Holder<index, Types>... {};

        template <typename... Types>
        using Type_Indexer_init = 
        Type_Indexer<index_sequence_init<sizeof...(Types)>, Types...>;

        template <int index, typename T>
        static auto fn_type_at(Type_Holder<index, T> const&)
        -> T;

        template <int index, typename... Types>  // partial template deduction!
        using type_at = decltype(fn_type_at<index>(Type_Indexer_init<Types...>{}));

        // partial template deduction:
        // fn_type_at<index>(Type_Holder<index, T>)
        // already know: index
        // search for: T in the list of Type_Holder
    };

    struct version_02
    {
        template <int index, typename T>
        struct Type_Holder {
            using type = T;
        };

        template <typename index_sequence, typename... Types>
        struct Type_Indexer;

        template <int... index, typename... Types>
        struct Type_Indexer<index_sequence<index...>, Types...>
        : public Type_Holder<index, Types>..., public Type_Holder<-1, NAUR> {};

        template <typename... Types>
        using Type_Indexer_init = 
        Type_Indexer<index_sequence_init<sizeof...(Types)>, Types...>;

        template <int index, typename T>
        static auto fn_type_at(Type_Holder<index, T> const&)
        -> Type_Holder<index, T>;

        template <int index, typename... Types>
        using type_at = typename decltype(fn_type_at<
            (0 <= index && index < sizeof...(Types))? index : -1
        >(Type_Indexer_init<Types...>{}))::type;
    };

public:

    template <int index, typename... Types>
    using type = typename version_02::type_at<index, Types...>;
};

template <int index, typename... Types>
using type_at = typename struct_type_at::type<index, Types...>;


// type_belongs
template <typename T, typename... Ts>
static constexpr bool type_belongs = (same_type<T, Ts> || ...);


// type_index
template <typename T, typename... Ts>
static constexpr int index_of_type = []()
{
    if constexpr (sizeof...(Ts) > 0)
    {
        bool is_same[sizeof...(Ts)] = { same_type<T, Ts>... };
        for (int i = 0; i < sizeof...(Ts); i++)
        {
            if (is_same[i])
                return i;
        }
        return -1;
    }
    else
    {
        return -1;
    }
}();


// pack
template <typename... Ts>
struct pack {};

// is pack
template <typename T>
static constexpr bool is_Pack = false;

template <typename... Ts>
static constexpr bool is_Pack<pack<Ts...>> = true;


// type at
template <int index, typename pack>
using Pack_type_at = typename pack::template type_at<index>;


// pushback / pushfront
template <typename pack, bool pushback, typename... Items>
struct struct_Pack_push {
    using type = NAUR;
};

template <typename... Pack_Items, typename... Items>
struct struct_Pack_push<pack<Pack_Items...>, true, Items...> {
    using type = pack<Pack_Items..., Items...>;
};

template <typename... Pack_Items, typename... Items>
struct struct_Pack_push<pack<Pack_Items...>, false, Items...> {
    using type = pack<Items..., Pack_Items...>;
};

template <typename pack, typename... Items>
using Pack_pushback = typename struct_Pack_push<pack, true, Items...>::type;

template <typename pack, typename... Items>
using Pack_pushfront = typename struct_Pack_push<pack, false, Items...>::type;


// concat
struct struct_Pack_concat {
private:

    struct version_01
    {
        template <typename Pack_Left, typename Pack_Right>
        struct struct_concat_two {
            using type = NAUR;
        };

        template <typename... Ts_Left, typename... Ts_Right>
        struct struct_concat_two<pack<Ts_Left...>, pack<Ts_Right...>> {
            using type = pack<Ts_Left..., Ts_Right...>;
        };

        template <typename Accumulator, typename... Packs>
        struct struct_Pack_concat;

        template <typename Accumulator, typename Pack_First, typename... Pack_Rest>
        struct struct_Pack_concat<Accumulator, Pack_First, Pack_Rest...> {
            using Next_Accumulator = typename struct_concat_two<Accumulator, Pack_First>::type;
            using type = typename struct_Pack_concat<Next_Accumulator, Pack_Rest...>::type;
        };

        template <typename Accumulator>
        struct struct_Pack_concat<Accumulator> {
            using type = Accumulator;
        };

        template <typename... Packs>
        using type = typename struct_Pack_concat<pack<>, Packs...>::type;
    };

public:

    template <typename... Packs>
    using type = version_01::type<Packs...>;
};

template <typename... Packs>
using Pack_concat = struct_Pack_concat::type<Packs...>;



// cartesian product
struct struct_Pack_cartesian_prod {
private:

    struct version_01
    {
        // Cartesian Product:
        //
        // Examples:
        // 1) { (A), (B) } * { (C), (D) } = { (A, C), (A, D), (B, C), (B, D) }
        // 2) { (A, B), (C, D) } * { (E) } = { (A, B, E), (C, D, E) }
        // 3) { (A, B), (C, D) } * { (E), (F) } = { (A, B, E), (C, D, E), (A, B, F), (C, D, F) }
        //
        // Identity set: { () }
        // Example:
        // 1) { () } * { (A), (B) } = { (A), (B) }
        // 2) { (C), (D) } * { () } = { (C), (D) }
        //
        // Null set: {}
        // Examples:
        // 1) {} * { (A), (B) } = {}
        // 2) { (A), (B) } * {} = {}
        //

        template <typename Packs_Pack_Left, typename Packs_Pack_Right>
        struct cartesian_prod_two {
            using type = NAUR;
        };

        template <typename... Packs_Left, typename... Packs_Right>
        struct cartesian_prod_two<pack<Packs_Left...>, pack<Packs_Right...>> {
            template <typename Pack_Left>
            using Segment = pack<Pack_concat<Pack_Left, Packs_Right>...>;
            using type = Pack_concat<Segment<Packs_Left>...>;
        };

        template <typename... Packs_Packs>
        struct cartesian_prod {
            using type = NAUR;
        };

        template <typename Accumulator, typename Packs_Pack_First, typename... Packs_Pack_Rest>
        struct cartesian_prod<Accumulator, Packs_Pack_First, Packs_Pack_Rest...> {
            using Next_Accumulator = typename cartesian_prod_two<Accumulator, Packs_Pack_First>::type;
            using type = typename cartesian_prod<Next_Accumulator, Packs_Pack_Rest...>::type;
        };

        template <typename Accumulator>
        struct cartesian_prod<Accumulator> {
            using type = Accumulator;
        };

        // Convert { A, B, C }  into  { (A), (B), (C) }
        template <typename pack>
        struct struct_wrap_elem {
            using type = NAUR;
        };

        template <typename... Ts>
        struct struct_wrap_elem<pack<Ts...>> {       //  pack<A, B, C>
            using type = pack<pack<Ts>...>;          // --> pack<pack<A>, pack<B>, pack<C>>
        };

        template <typename pack>
        using wrap_elem = typename struct_wrap_elem<pack>::type;

        template <typename... Packs>
        using Pack_cartesian_prod = typename cartesian_prod<pack<pack<>>, wrap_elem<Packs>...>::type;
    };

    struct version_02
    {
        // { (A0), (A1), (A2) } * { (B0), (B1) } * { (C0), (C1), (C2), (C3) }
        // 
        // result:
        // (A0, B0, C0)  -->  000
        // (A0, B0, C1)  -->  001
        // (A0, B0, C2)  -->  002
        // (A0, B0, C3)  -->  003
        // (A0, B1, C0)  -->  010
        // (A0, B1, C1)  -->  011
        // (A0, B1, C2)  -->  012
        // (A0, B1, C3)  -->  013
        // (A1, B0, C0)  -->  100
        // (A1, B0, C1)  -->  101
        // (A1, B0, C2)  -->  102
        // (A1, B0, C3)  -->  103
        // (A1, B1, C0)  -->  110
        // (A1, B1, C1)  -->  111
        // (A1, B1, C2)  -->  112
        // (A1, B1, C3)  -->  113
        // (A2, B0, C0)  -->  200
        // (A2, B0, C1)  -->  201
        // (A2, B0, C2)  -->  202
        // (A2, B0, C3)  -->  203
        // (A2, B1, C0)  -->  210
        // (A2, B1, C1)  -->  211
        // (A2, B1, C2)  -->  212
        // (A2, B1, C3)  -->  213
        //
        // ==> It's the same thing as multi-modular numbers

        template <bool condition, typename... Packs>
        struct struct_Pack_cartesian_prod {
            using type = NAUR;
        };

        template <typename... Packs>
        struct struct_Pack_cartesian_prod<true, Packs...> 
        {
            static constexpr int DIMENSION = sizeof...(Packs);
            static constexpr int ELEMS_COUNT = (Packs::size() * ... * 1);

            class Modular {
            private:

                int value[sizeof...(Packs)];

            public:

                // { (A0), (A1), (A2) } * { (B0), (B1) } * { (C0), (C1), (C2), (C3) }
                // mods:
                // mod[0] = 4
                // mod[1] = 2
                // mod[2] = 3
                // ==> mod has reversed order compared to Packs::size()

                struct Array {
                public:

                    int value[sizeof...(Packs)];
                    constexpr int operator[](int index) const { return value[index]; }
                };

                static constexpr Array mod = []()
                {
                    Array result = {};  // no default constructor, must be explicitly initialized
                    int reverse_value[sizeof...(Packs)] = { Packs::size()... };
                    for (int i = 0; i < sizeof...(Packs); i++)
                    {
                        result.value[i] = reverse_value[(sizeof...(Packs) - 1) - i];
                    }
                    return result;
                }();

                constexpr Modular(int decimal)
                {
                    decimal %= (Packs::size() * ... * 1);
                    for (int i = 0; i < sizeof...(Packs); i++)
                    {
                        value[i] = decimal % mod[i];
                        decimal /= mod[i];
                    }
                }

                constexpr int get(int index) const
                {
                    return value[index];
                }
            };

            // each pack
            template <int Row, int... Col>
            static auto render_row(index_sequence<Col...>) 
            -> pack<Pack_type_at<Modular(Row).get((sizeof...(Packs) - 1) - Col), type_at<Col, Packs...>>...>;

            // whole array
            template <int... Row>
            static auto render_all(index_sequence<Row...>) 
            -> pack<decltype(render_row<Row>(index_sequence_init<DIMENSION>{}))...>;

            using type = decltype(render_all(index_sequence_init<ELEMS_COUNT>{}));
        };

        template <typename... Packs>
        using Pack_cartesian_prod = typename struct_Pack_cartesian_prod<(is_Pack<Packs> && ...), Packs...>::type;
    };

    struct version_03
    {
        // still use as specific case model:
        // { (A0), (A1), (A2) } * { (B0) , (B1) } * { (C0), (C1), (C2), (C3) }

        template <bool condition, typename... Packs>
        struct struct_Pack_cartesian_prod {
            using type = NAUR;
        };

        template <typename... Packs>
        struct struct_Pack_cartesian_prod<true, Packs...> 
        {
            static constexpr int DIMENSION = sizeof...(Packs);
            static constexpr int ELEMS_COUNT = (Packs::size() * ... * 1);

            // encode pack as array of indices
            // [2, 1, 3]  -->  pack<A2, B1, C3>

            class EncodedPack {
            public:
                int value[DIMENSION];
                constexpr int& operator[](int i) { return value[i]; }
                constexpr int operator[](int i) const { return value[i]; }
            };

            class Array {
            public:
                EncodedPack pack[ELEMS_COUNT];
                constexpr EncodedPack& operator[](int i) { return pack[i]; }
                constexpr EncodedPack operator[](int i) const { return pack[i]; }
            };

            static constexpr Array encoded = []()
            {
                // Fill up array column by column
                // [0, _, _]            [0, 0, _]            [0, 0, 0]
                // [0, _, _]            [0, 0, _]            [0, 0, 1]
                // [0, _, _]            [0, 0, _]            [0, 0, 2]
                // [0, _, _]            [0, 0, _]            [0, 0, 3]
                // [0, _, _]            [0, 1, _]            [0, 1, 0]
                // [0, _, _]            [0, 1, _]            [0, 1, 1]
                // [0, _, _]            [0, 1, _]            [0, 1, 2]
                // [0, _, _]            [0, 1, _]            [0, 1, 3]
                // [1, _, _]            [1, 0, _]            [1, 0, 0]
                // [1, _, _]            [1, 0, _]            [1, 0, 1]
                // [1, _, _]            [1, 0, _]            [1, 0, 2]
                // [1, _, _]    ===>    [1, 0, _]    ===>    [1, 0, 3]
                // [1, _, _]            [1, 1, _]            [1, 1, 0]
                // [1, _, _]            [1, 1, _]            [1, 1, 1]
                // [1, _, _]            [1, 1, _]            [1, 1, 2]
                // [1, _, _]            [1, 1, _]            [1, 1, 3]
                // [2, _, _]            [2, 0, _]            [2, 0, 0]
                // [2, _, _]            [2, 0, _]            [2, 0, 1]
                // [2, _, _]            [2, 0, _]            [2, 0, 2]
                // [2, _, _]            [2, 0, _]            [2, 0, 3]
                // [2, _, _]            [2, 1, _]            [2, 1, 0]
                // [2, _, _]            [2, 1, _]            [2, 1, 1]
                // [2, _, _]            [2, 1, _]            [2, 1, 2]
                // [2, _, _]            [2, 1, _]            [2, 1, 3]

                // In this example:
                // step[0] = 2 * 4 * 1
                // step[1] = 4 * 1
                // step[2] = 1

                Array arr = {};
                constexpr int COLUMNS = sizeof...(Packs);
                int pack_size[sizeof...(Packs)] = { Packs::size()... };

                int step[COLUMNS] = {};
                // compute step
                {
                    int acc = 1;
                    for (int i = (COLUMNS - 1); i >=0; i--)
                    {
                        step[i] = acc;
                        acc *= pack_size[i];
                    }
                }

                // start filling
                for (int column = 0; column < COLUMNS; column++)
                {
                    int index = 0;
                    for (int row = 0; row < ELEMS_COUNT; row += step[column])
                    {
                        for (int i = row; i < (row + step[column]); i++)
                        {
                            arr[i][column] = index;
                        }

                        index++; if (index == pack_size[column]) index = 0;
                    }
                }

                return arr;
            }();

            // each pack
            template <int Row, int... Col>
            static auto decoded_Pack(index_sequence<Col...>) 
            -> pack<Pack_type_at<encoded[Row][Col], type_at<Col, Packs...>>...>;

            // whole array
            template <int... Row>
            static auto render(index_sequence<Row...>) 
            -> pack<decltype(decoded_Pack<Row>(index_sequence_init<DIMENSION>{}))...>;

            using type = decltype(render(index_sequence_init<ELEMS_COUNT>{}));
        };

        template <typename... Packs>
        using Pack_cartesian_prod = typename struct_Pack_cartesian_prod<(is_Pack<Packs> && ...), Packs...>::type;
    };

public:

    template <typename... Packs>
    using type = version_02::Pack_cartesian_prod<Packs...>;
};

template <typename... Packs>
using Pack_cartesian_prod = struct_Pack_cartesian_prod::type<Packs...>;


// filter
template <template <typename> typename Trait, typename pack, typename = template_pass>
struct struct_Pack_filter {
    using type = NAUR;
};

template <template <typename> typename Trait, typename... Ts>
struct struct_Pack_filter<Trait, pack<Ts...>, enable_if<same_type<decltype(Trait<_>::value), const bool>>> {
private:

    struct version_01
    {
        template <typename Accumulator, typename... Types>
        struct filter;

        template <typename Accumulator, typename First, typename... Rest>
        struct filter<Accumulator, First, Rest...> {
            using Next_Accumulator = switch_type<
                Trait<First>::value, 
                Pack_pushback<Accumulator, First>,
                Accumulator
            >;
            using type = typename filter<Next_Accumulator, Rest...>::type;
        };
        
        template <typename Accumulator>
        struct filter<Accumulator> {
            using type = Accumulator;
        };

        using type = typename filter<pack<>, Ts...>::type;
    };

    struct version_02
    {
        struct Table {
            int arr[sizeof...(Ts)] = {};
            int size = 0;
            constexpr int& operator[](int index) { return arr[index]; }
            constexpr int operator[](int index) const { return arr[index]; }
        };

        static constexpr Table type_index = []()
        {
            Table table = {};
            bool has_trait[sizeof...(Ts)] = { Trait<Ts>::value... };

            for (int i = 0; i < sizeof...(Ts); i++)
            {
                if (has_trait[i]) 
                {
                    table[table.size++] = i;
                }
            }

            return table;
        }();

        template <int... i>
        static auto render(index_sequence<i...>) 
        -> pack<type_at<type_index[i], Ts...>...>;

        using type = decltype(render(index_sequence_init<type_index.size>{}));
    };

public:

    using type = typename version_02::type;
};

template <template <typename> typename Trait, typename pack>
using Pack_filter = typename struct_Pack_filter<Trait, pack>::type;


// unique pack
template <typename pack>
struct struct_Pack_filter_unique {
    using type = NAUR;
};

template <typename... Ts>
struct struct_Pack_filter_unique<pack<Ts...>> {
private:

    struct Table {
        int arr[sizeof...(Ts)] = {};
        int size = 0;
        constexpr int& operator[](int index) { return arr[index]; }
        constexpr int operator[](int index) const { return arr[index]; }
    };

    static constexpr Table type_index = []()
    {
        Table table = {};
        int index[sizeof...(Ts)] = { index_of_type<Ts, Ts...>... };
        bool mark_take[sizeof...(Ts)] = {};

        for (int i = 0; i < sizeof...(Ts); i++)
        {
            mark_take[index[i]] = true;
        }

        for (int i = 0; i < sizeof...(Ts); i++)
        {
            if (mark_take[i])
            {
                table[table.size++] = i;
            }
        }

        return table;
    }();

    template <int... i>
    static auto render(index_sequence<i...>) 
    -> pack<type_at<type_index[i], Ts...>...>;

public:

    using type = decltype(render(index_sequence_init<type_index.size>{}));
};

template <typename pack>
using Pack_filter_unique = typename struct_Pack_filter_unique<pack>::type;


// homogeneous pack
template <int count, typename T>
struct struct_Homogeneous_Pack {
private:

    template <int DUMMY, typename Ty>
    using Alter = Ty;

    template <int... index>
    static auto render(index_sequence<index...>)
    -> pack<Alter<index, T>...>;

public:

    using type = decltype(render(index_sequence_init<count>{}));
};

template <int count, typename T>
using Homogeneous_Pack = typename struct_Homogeneous_Pack<count, T>::type;


// is prefix pack
template <typename Prefix_Pack, typename pack>
struct struct_Pack_is_prefix {
    static constexpr bool value = false;
};

template <typename... Prefix, typename... Types>
struct struct_Pack_is_prefix<pack<Prefix...>, pack<Types...>> {
private:

    static constexpr int prefix_padding = (sizeof...(Types) > sizeof...(Prefix))? 
    (sizeof...(Types) - sizeof...(Prefix)) : (0);
    using Padded_Prefix = Pack_concat<pack<Prefix...>, Homogeneous_Pack<prefix_padding, NAUR>>;

    static constexpr int types_padding = (sizeof...(Prefix) > sizeof...(Types))? 
    (sizeof...(Prefix) - sizeof...(Types)) : (0);
    using Padded_Types = Pack_concat<pack<Types...>, Homogeneous_Pack<types_padding, NAUR>>;

    template <typename... Padded_Prefix, typename... Padded_Types>
    static constexpr bool eval(pack<Padded_Prefix...>, pack<Padded_Types...>)
    {
        if (sizeof...(Padded_Types) > sizeof...(Types))
            return false;

        bool value[sizeof...(Padded_Prefix)] = { same_type<Padded_Prefix, Padded_Types>... };
        for (int i = 0; i < sizeof...(Prefix); i++)
        {
            if (!value[i])
                return false;
        }
        return true;
    }

public:

    static constexpr bool value = eval(Padded_Prefix{}, Padded_Types{});
};

template <typename Prefix_Pack, typename pack>
static constexpr bool Pack_is_prefix = struct_Pack_is_prefix<Prefix_Pack, pack>::value;


template <typename T>
struct struct_no_type_deduction {
    using type = T;
};

template <typename T>
using no_type_deduction = typename struct_no_type_deduction<T>::type;

#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

_MNC_END

#undef _MNC_BEGIN
#undef _MNC_END