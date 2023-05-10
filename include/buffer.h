#ifndef __LIBBASIC_BUFFER_H__
#define __LIBBASIC_BUFFER_H__

#include "byte.h"
#include "byte_view.h"
#include "type_traits.h"
#include <concepts>
#include <ranges>

namespace basic
{
// clang-format off

namespace __detail {
	template <typename _Tp, typename _Up>
	concept __different_from
    	= !std::same_as<std::remove_cvref_t<_Tp>, std::remove_cvref_t<_Up> >;
}

template <typename _Tp>
concept basic_buffer_underlying = as_byte_view_from<_Tp>
	&& is_underlying_byte_v<range_iter_value_type<_Tp>>
    && requires (const _Tp &__x, _Tp &__t, std::size_t __n)
{
  { __x.max_size () } -> std::same_as<std::size_t>;
  { __x.capacity () } -> std::same_as<std::size_t>;
  __t.resize (__n);
  __t.erase (std::ranges::begin (__t), std::ranges::end (__t));
};

template <basic_buffer_underlying _Tp>
class basic_buffer_adaptor
{
	static void _S_test (_Tp &);
	static void _S_test (_Tp &&) = delete;

public:
	template <__detail::__different_from<default_basic_buffer_adaptor> _Up>
	requires std::convertible_to<_Up, _Tp &> 
		&& requires { _S_test (std::declval<_Up> ()); }
	constexpr explicit
	default_basic_buffer_adaptor (_Up &&__t)
	noexcept (noexcept (static_cast<_Tp &> (std::declval<_Up> ())))
	: _M_uptr (std::addressof(static_cast<_Tp &> (std::forward<_Up> (__t)))),
		_M_size (std::ranges::size(__t))
	{ }

	constexpr decltype(auto)
	base () const noexcept
	{ return *_M_uptr; }

	constexpr auto
	size () const noexcept
	{ return _M_size; }

	constexpr auto
	data () const noexcept
	{ return as_byte_view(*_M_uptr); }

	constexpr auto
	empty () const noexcept
	{ return _M_size == 0; }

	constexpr auto
	begin () noexcept
	{ return std::ranges::begin (*_M_uptr); }

	constexpr auto
	begin () const noexcept
	{ return std::ranges::begin (*_M_uptr); }

	constexpr auto
	end () noexcept
	{ return std::ranges::end (*_M_uptr); }

	constexpr auto
	end () const noexcept
	{ return std::ranges::end (*_M_uptr); }

	constexpr auto
	max_size () const noexcept
	{ return _M_uptr->max_size (); }

	constexpr auto
	capacity () const noexcept
	{ return _M_uptr->capacity (); }

	constexpr auto
	prepare (std::size_t __n) 
	{
		__glibcxx_assert (_M_size + __n <= max_size ());
		_M_uptr->resize (_M_size + __n);
		return as_byte_view (*_M_uptr).subview(_M_size, __n);
	}

	constexpr void
	commit (std::size_t __n)
	{
		_M_size += std::min (__n, std::ranges::size(*_M_uptr) - _M_size);
		_M_uptr->resize (_M_size);
	}

	constexpr void
	consume (std::size_t __n)
	{
		std::size_t __m = std::min (__n, _M_size);
		auto __begin = std::ranges::begin (*_M_uptr);
		_M_uptr->erase (__begin, __begin + __m);
		_M_size -= __m;
	}

private:
	_Tp* _M_uptr;
	std::size_t _M_size;
};

template <basic_buffer_underlying _Tp>
class basic_buffer
{
	using _Adaptor_type = basic_buffer_adaptor<_Tp>;
public:
  	using value_type = range_iter_value_type<_Adaptor_type>;
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

public:
  template<typename _Tp>
  requires (basic_buffer_underlying<std::remove_cvref_t<_Tp>> 
    && !std::same_as<basic_buffer, std::remove_cvref_t<_Tp>>)
  basic_buffer(_Tp&& __t) 
  noexcept(std::is_nothrow_constructible_v<_Adaptor, _Tp>)
  	: _M_adaptor(std::forward<_Tp>(__t))
  { }

  constexpr decltype (auto)
  base () const noexcept 
  { return _M_adaptor; }

  constexpr auto
  size () const noexcept
  { return std::ranges::size (_M_adaptor); }

  constexpr auto
  data () noexcept
  { return std::ranges::data (_M_adaptor); }

  constexpr auto
  data () const noexcept
  { return std::ranges::data (_M_adaptor); }

  constexpr auto
  empty () const
  { return std::ranges::empty (_M_adaptor); }

  constexpr auto
  begin () noexcept
  { return std::ranges::begin (_M_adaptor); }

  constexpr auto
  begin () const noexcept
  { return std::ranges::begin (_M_adaptor); }

  constexpr auto
  end () noexcept
  { return std::ranges::end (_M_adaptor); }

  constexpr auto
  end () const noexcept
  { return std::ranges::end (_M_adaptor); }

  constexpr auto
  capacity () const noexcept
  { return _M_adaptor.capacity (); }

  constexpr auto
  max_size () const noexcept
  { return _M_adaptor.max_size (); }

  constexpr decltype (auto) 
  prepare (std::size_t __n)
  { return _M_adaptor.prepare (__n); }

  constexpr void
  commit (std::size_t __n)
  { _M_adaptor.commit (__n); }

  constexpr void
  consume (std::size_t __n)
  { _M_adaptor.consume (__n); }

  constexpr auto
  rbegin () const noexcept
  { return std::ranges::rbegin(_M_adaptor); }

  constexpr auto
  rbegin () noexcept
  { return std::ranges::rbegin(_M_adaptor); }

  constexpr auto
  rend () const noexcept
  { return std::ranges::rend (_M_adaptor); }

  constexpr auto
  rend () noexcept
  { return std::ranges::rend (_M_adaptor); }

  constexpr auto
  cbegin () const noexcept
  { return std::ranges::cbegin (_M_adaptor); }

  constexpr auto
  cend () const noexcept
  { return std::ranges::cend (_M_adaptor); }

  constexpr auto
  crbegin () const noexcept
  { return std::ranges::crbegin(_M_adaptor); }

  constexpr auto
  crend () const noexcept
  { return std::ranges::crend(_M_adaptor); }

private:
   _Adaptor_type _M_adaptor;
};

template<typename _Tp>
basic_buffer(_Tp&&) -> basic_buffer<std::remove_reference_t<_Tp>>;

} // namespace basic

#endif // __LIBBASIC_BUFFER_H__