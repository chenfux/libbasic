#ifndef __LIBBASIC_TYPE_TRAITS_H__
#define __LIBBASIC_TYPE_TRAITS_H__

#include <ranges>
#include <type_traits>

namespace basic
{
    // Casting an `int` to a `char` or a `float` to an `int` can be done
    // implicitly, which means that std::is_convertible_v<int, char> is true.
    // However, this may not always be what we want.

    // The std::span (gcc 12.2.0) uses __is_array_convertible_v to ensure that the following code is invalid:
    //
    // std::array<int, 2> arr = {1, 2};
    // std::span<char> s(arr);

    template <typename _From, typename _To>
    using is_array_convertible = std::is_convertible<_From (*)[], _To (*)[]>;

    template <typename _From, typename _To>
    inline constexpr bool is_array_convertible_v
        = is_array_convertible<_From, _To>::value;

    template <typename _Range>
    using range_iter_value_type
        = std::remove_reference_t<std::ranges::range_reference_t<_Range> >;

    template <typename _It>
    using iter_value_type = std::remove_reference_t<std::iter_reference_t<_It> >;

    //6.8 Types
    //  2 For any object (other than a potentially-overlapping subobject) of trivially copyable type T, whether or not 
    //    the object holds a valid value of type T, the underlying bytes (6.7.1) making up the object can be copied into
    //    an array of char, unsigned char, or std::byte (17.2.1).37 If the content of that array is copied back into 
    //    the object, the object shall subsequently hold its original value. 
	//6.8.1 Fundamental types
	//  Type char is a distinct type that has an implementation-defined choice of “signed char” or “unsigned char” as its underlying type. 
	//  The values of type char can represent distinct codes for all members of the implementation’s basic character set. 
	//  The three types char, signed char, and unsigned char are collectively called ordinary character types. 
	//  The ordinary character types and char8_t are collectively called narrow character types.
	//7.6.2.4 Sizeof
	//  The result of sizeof applied to any of the narrow character types is 1
	template<typename _Tp>
    struct __is_underlying_byte_impl
		: std::integral_constant<bool, sizeof(_Tp) == 1 && std::is_integral<_Tp>::value> 
	{ };

  	template<>
    struct __is_underlying_byte_impl<std::byte>
        : std::true_type 
	{ };

    template <typename _Tp>
    struct is_underlying_byte
        : public __is_underlying_byte_impl<std::remove_cvref_t<_Tp>> 
    { };

    template <typename _Tp>
    inline constexpr bool is_underlying_byte_v 
        = is_underlying_byte<_Tp>::value;

    template <typename> inline constexpr bool dependent_false = false;

} // namespace basic
#endif //__LIBBASIC_TYPE_TRAITS_H__