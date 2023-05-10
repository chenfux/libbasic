#ifndef __LIBBASIC_BYTE_VIEW_H__
#define __LIBBASIC_BYTE_VIEW_H__

#include "byte.h"
#include "type_traits.h"
#include <concepts>

// clang-format off

namespace basic
{

template <typename _Tp>
requires is_underlying_byte_v<_Tp> class basic_byte_view;

namespace __detail {
	template<typename _Tp>
    inline constexpr bool __is_byte_view = false;
    template<typename _Tp>
    inline constexpr bool __is_byte_view<basic_byte_view<_Tp>> = true;

	template <typename _Tp>
	inline constexpr bool __is_class_or_enum_v
    	= std::is_class_v<_Tp> || std::is_union_v<_Tp> || std::is_enum_v<_Tp>;

	struct _Decay_copy final
	{
		template <typename _Tp>
		constexpr std::decay_t<_Tp>
		operator() (_Tp &&__t) const
			noexcept (std::is_nothrow_convertible_v<_Tp, std::decay_t<_Tp> >)
		{
			return std::forward<_Tp> (__t);
		}
	} inline constexpr __decay_copy{};

	template <typename _Tp>
	concept __byte_view_like = __is_byte_view<std::remove_cvref_t<_Tp> >;

	template <typename _Tp> concept __has_member_as_byte_view = requires (_Tp & __t)
	{ 
		{ __decay_copy (__t.as_byte_view ()) } -> __byte_view_like; 
	};

	template <typename _Tp>
	concept __has_adl_as_byte_view
		= __is_class_or_enum_v<std::remove_reference_t<_Tp> > && requires (_Tp & __t)
	{
		{ __decay_copy (as_byte_view (__t)) } -> __byte_view_like;
	};

}

template <typename _Tp>
requires is_underlying_byte_v<_Tp>
class basic_byte_view
    : public std::ranges::view_interface<basic_byte_view<_Tp> >
{

	template <typename _Up>
	static constexpr _Tp* __byte_cast(_Up* __from) {
		return static_cast<_Tp*>(static_cast<void*>(__from));
	}
	template <typename _Up>
	static constexpr const _Tp* __byte_cast(const _Up* __from) {
		return static_cast<const _Tp*>(static_cast<const void*>(__from));
	}

	//TEST:
	//1. Is _Up trivially copyable.
	//2. Is _Up more cv-qualified than _Tp.
	//3. Is _Up a char types and _Tp is the std::byte.

	template <typename _Up>
	static inline constexpr bool __is_byte_compatible_v = std::is_trivially_copyable_v<_Up>
		&& std::is_convertible_v<decltype(__byte_cast (std::declval<_Up*> ())), _Tp*>;

public:
	using value_type = _Tp;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using pointer = value_type *;
	using const_pointer = const value_type *;
	using reference = value_type &;
	using const_reference = const value_type &;
	using iterator = pointer;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_iterator = const_pointer;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	constexpr
	basic_byte_view () noexcept 
	: _M_ptr (nullptr), _M_size (0)
	{ }

	template<typename _Up>
	requires __is_byte_compatible_v<_Up>
	constexpr
	basic_byte_view (_Up* __ptr, size_type __n) noexcept
	: _M_ptr (__byte_cast(__ptr)), _M_size (__n * sizeof(_Up))
	{ }

	template <std::contiguous_iterator _It>
	requires __is_byte_compatible_v<iter_value_type<_It>> 
	constexpr
	basic_byte_view (_It __it, size_type __n) noexcept
	: _M_ptr (__byte_cast(std::to_address (__it))), 
		_M_size (__n * sizeof(iter_value_type<_It>))
	{ }

  	template <std::contiguous_iterator _It, std::sized_sentinel_for<_It> _End>
    requires __is_byte_compatible_v<iter_value_type<_It>>
    	&& (!std::is_convertible_v<_End, size_type>)
	constexpr
	basic_byte_view (_It __first, _End __last)
          noexcept (noexcept (__last - __first))
    : _M_ptr (__byte_cast(std::to_address (__first))), 
		_M_size (static_cast<size_type> (__last - __first) * sizeof(iter_value_type<_It>))
  	{ }

	template <typename _Up, std::size_t _Len>
	requires __is_byte_compatible_v<_Up>
  	constexpr
    basic_byte_view (_Up(&__arr)[_Len]) noexcept
	: basic_byte_view (__arr, _Len)
  	{ }

  	template <typename _Up>
    requires (!__detail::__is_byte_view<std::remove_cvref_t<_Up> >)
		&& (!std::is_array_v<std::remove_cvref_t<_Up> >)
		&& std::ranges::contiguous_range<_Up> 
		&& std::ranges::sized_range<_Up>
		&& __is_byte_compatible_v<range_iter_value_type<_Up>>
    constexpr explicit 
	basic_byte_view (_Up &&__c)
	noexcept (noexcept (std::ranges::data (__c)) 
		&& noexcept (std::ranges::size (__c)))
    : basic_byte_view (std::ranges::data (__c), std::ranges::size (__c))
  	{ }

	constexpr
	basic_byte_view(const basic_byte_view& __x, size_t __n) noexcept
	: _M_ptr(__x._M_ptr), _M_size(std::min(__x._M_size, __n))
	{ }

  	constexpr 
  	basic_byte_view (const basic_byte_view &) noexcept = default;

  	template <typename _Up>
	requires (!std::is_same_v<_Up, _Tp> && __is_byte_compatible_v<_Up>)
    constexpr explicit
	basic_byte_view (const basic_byte_view<_Up> &__x) noexcept 
	: _M_ptr (__x._M_ptr), _M_size (__x._M_size)
  	{ }

  	~basic_byte_view () noexcept = default;

	constexpr 
	basic_byte_view &
	operator= (const basic_byte_view &) noexcept = default;

	constexpr const_pointer
	data () const noexcept
	{ return this->_M_ptr; }

	constexpr pointer
	data () noexcept
	{ return this->_M_ptr; }

	constexpr size_type
	size () const noexcept
	{ return this->_M_size; }

	constexpr bool
	empty () const noexcept
	{ return this->_M_size == 0; }

	constexpr const_iterator
	begin () const noexcept
	{ return this->_M_ptr; }

	constexpr iterator
	begin () noexcept
	{ return this->_M_ptr; }

	constexpr const_iterator
	end () const noexcept
	{ return this->_M_ptr + _M_size; }

	constexpr iterator
	end () noexcept
	{ return this->_M_ptr + this->_M_size; }

	constexpr const_reverse_iterator
	rbegin () const noexcept
	{ return  const_reverse_iterator(this->end ()); }

	constexpr reverse_iterator
	rbegin () noexcept
	{ return  reverse_iterator(this->end ()); }

	constexpr const_reverse_iterator
	rend () const noexcept
	{ return const_reverse_iterator(this->begin ()); }

	constexpr reverse_iterator
	rend () noexcept
	{ return reverse_iterator(this->begin ()); }

	constexpr const_iterator
	cbegin () const noexcept
	{ return const_iterator(this->begin ()); }

	constexpr const_iterator
	cend () const noexcept
	{ return const_iterator(this->end ()); }

	constexpr const_reverse_iterator
	crbegin () const noexcept
	{ return const_reverse_iterator(this->end ()); }

	constexpr const_reverse_iterator
	crend () const noexcept
	{ return const_reverse_iterator(this->begin ()); }

	constexpr basic_byte_view
	subview(size_type __offset, size_type __size) const noexcept
	{
		__glibcxx_assert(__offset <= size());
		__glibcxx_assert(__size <= size());
		__glibcxx_assert(__offset + __size <= size());
		return { this->_M_ptr + __offset, __size };
	}

private:
	pointer _M_ptr;
	size_type _M_size;
};

using byte_view = basic_byte_view<const byte>;
using mutable_byte_view = basic_byte_view<byte>;

namespace __cust_access
{
	struct _As_byte_view
	{
		template <typename _Tp>
		requires __detail::__has_member_as_byte_view<_Tp> 
			|| __detail::__has_adl_as_byte_view<_Tp> 
			|| std::is_constructible_v<mutable_byte_view, _Tp>
			|| std::is_constructible_v<byte_view, _Tp>
		[[nodiscard]] constexpr decltype (auto)
		operator() (_Tp &&__t) const noexcept
		{
			// First, prefer member .as_buffer()
			if constexpr (__detail::__has_member_as_byte_view<_Tp>)
				return std::forward<_Tp> (__t).as_byte_view ();
			// Second, check ADL-found
			else if constexpr (__detail::__has_adl_as_byte_view<_Tp>)
				return as_byte_view (std::forward<_Tp> (__t));
			// Third, check constructible
			else if constexpr (std::is_constructible_v<mutable_byte_view, _Tp>)
				return mutable_byte_view (std::forward<_Tp>(__t));
			else if constexpr (std::is_constructible_v<byte_view, _Tp>)
				return byte_view (std::forward<_Tp>(__t));
			// Finally
			else
				static_assert (dependent_false<_Tp>, "You should never see this.");
		}

		template <std::contiguous_iterator _It, std::sized_sentinel_for<_It> _End>
		requires (!std::is_convertible_v<_End, std::size_t>)
		[[nodiscard]] constexpr auto
		operator() (_It __it, _End __end) const noexcept 
		{
			if constexpr(std::is_const_v<iter_value_type<_It>>)
				return byte_view {__it, __end};
			return mutable_byte_view {__it, __end};
		}

		template <std::contiguous_iterator _It> 
		[[nodiscard]] constexpr auto
		operator() (_It __it, std::size_t __n) const noexcept 
		{
			if constexpr (std::is_const_v<iter_value_type<_It>>)
				return byte_view {__it, __n};
			return mutable_byte_view {__it, __n};
		}

		template <typename _Tp>
		[[nodiscard]] constexpr auto
		operator() (_Tp *__ptr, std::size_t __n) const noexcept
		{
			if constexpr (std::is_const_v<_Tp>)
			return byte_view { __ptr, __n };
			return mutable_byte_view { __ptr, __n };
		}

		template <typename _Tp>
		[[nodiscard]] constexpr auto
		operator() (basic_byte_view<_Tp> __b, std::size_t __n) const noexcept
		{
			return basic_byte_view<_Tp> {__b, __n};
		}
	};
}

inline namespace __cust
{
	inline constexpr __cust_access::_As_byte_view as_byte_view {};
}

template <typename _Tp>
concept as_byte_view_from = requires(_Tp&& __t) {
    as_byte_view(std::forward<_Tp>(__t));
};


}
#endif //__LIBBASIC_BYTE_VIEW_H__