#ifndef __LIBBASIC_RLP_H__
#define __LIBBASIC_RLP_H__

#include "bigint.h"
#include "bytes.h"
#include <ranges>

namespace basic
{

namespace __detail
{

template <typename _Tp>
requires std::is_unsigned_v<_Tp> constexpr std::size_t
__unsigned_to_bytes_len (_Tp __val)
{
  std::size_t __n = 0;
  for (; __val != 0; __val >>= 8)
    ++__n;
  return __n;
}

template <typename _Tp, typename _Bytes>
requires std::is_unsigned_v<_Tp> constexpr void
__unsigned_to_bytes (_Tp __val, _Bytes &__result)
{
  std::size_t __n = __unsigned_to_bytes_len (__val);
  __result.resize (__n);
  for (; __n > 0 && __val != 0; --__n, __val >>= 8)
    __result[__n - 1] = __val & 0xff;
}

template <typename _Bytes, typename _Tp>
requires std::is_unsigned_v<_Tp> constexpr void
__bytes_to_unsigned (const _Bytes &__bytes, _Tp &__result)
{
  __result = 0;
  for (std::size_t __i = 0; __i < __bytes.size (); ++__i)
    __result = ((__result << 8) | __bytes[__i]);
}
}

template <typename _Byte> class rlp_item 
  : private std::span<_Byte>
{
  typedef std::span<_Byte> _Base;

public:
  typedef typename _Base::value_type value_type;
  typedef typename _Base::size_type size_type;
  typedef typename _Base::difference_type difference_type;
  typedef typename _Base::pointer pointer;
  typedef typename _Base::const_pointer const_pointer;
  typedef typename _Base::reference reference;
  typedef typename _Base::const_reference const_reference;
  typedef typename _Base::iterator iterator;

public:
  using _Base::_Base;

  using _Base::back;
  using _Base::begin;
  using _Base::data;
  using _Base::end;
  using _Base::front;
  using _Base::rbegin;
  using _Base::rend;
  using _Base::operator[];
  using _Base::empty;
  using _Base::size;

  value_type
  prefix () const noexcept
  {
    return _Base::front ();
  }

  size_type
  length () const
  {
    return _M_position ().second;
  }

  size_type
  offset () const
  {
    return _M_position ().first;
  }

  std::span<value_type>
  payload () const
  {
    if (empty ())
      return {};
    std::pair<size_type, size_type> __pos = _M_position ();
    iterator __iter = _Base::begin ();
    return std::span<value_type>{ __iter + __pos.first,
                                  __iter + __pos.first + __pos.second };
  }

  size_type
  items_size () const
  {
    return _M_split_list ().size ();
  }

  rlp_item
  sub_item (size_type __i) const
  {
    std::vector<std::pair<iterator, size_type> > __items = _M_split_list ();
    std::size_t __n = __items.size ();
    if (__i >= __n)
      {
        throw std::bad_cast ();
      }
    const std::pair<iterator, value_type> &__sub_item = __items[__i];
    return rlp_item (__sub_item.first, __sub_item.second);
  }

  bool
  is_list () const noexcept
  {
    return _M_is_list ();
  }

  int
  compare (const rlp_item &__x) const
  {
    return data () == __x.data () && size () == __x.size ();
  }

  template <typename _Tp>
  _Tp
  to_value () const
  {
    _Tp __result;
    _M_to_value (__result);
    return __result;
  }

private:
  template <std::ranges::input_range _Range>
  void
  _M_to_value (_Range &__result) const
  {
    if (!_M_is_list ())
      {
        throw std::bad_cast ();
      }
    std::vector<std::pair<iterator, value_type> > __items = _M_split_list ();
    typename _Range::iterator __begin = std::ranges::begin (__result),
                              __end = std::ranges::end (__result);
    for (std::size_t __i = 0; __i < __items.size () && __begin != __end;
         ++__i, ++__begin)
      {
        sub_item (__i).template to_value<std::ranges::range_value_t<_Range> > (
            *__begin);
      }
  }

  template <typename _Tp>
  requires (std::is_unsigned_v<_Tp>
            || __is_boost_multiprecision_number<_Tp>::value) constexpr
      void _M_to_value (_Tp &__result) const
  {
    if (empty ())
      {
        throw std::bad_cast ();
      }

    if (!_M_is_list ())
      {
        std::span<value_type> __payload = payload ();
        if constexpr (std::is_unsigned_v<_Tp>)
          __detail::__bytes_to_unsigned (__payload, __result);
        else
          boost::multiprecision::import_bits (__result, __payload.begin (),
                                              __payload.end (), 8);
      }
    else
      {
        throw std::bad_cast ();
      }
  }

  void
  _M_to_value (std::string &__str) const
  {
    if (_M_is_list ())
      {
        throw std::bad_cast ();
      }
    __str.clear ();
    std::span<value_type> __payload = payload ();
    __str.assign (__payload.begin (), __payload.begin () + length ());
  }

  bool
  _M_is_list () const noexcept
  {
    return !empty () && _Base::front () >= 0xc0;
  }

  std::vector<std::pair<iterator, size_type> >
  _M_split_list () const
  {
    std::vector<std::pair<iterator, size_type> > __result;
    if (_M_is_list ())
      {
        std::span<value_type> __payload = payload ();
        size_type __n = __payload.size ();
        while (__n != 0 && __n >= __payload.size ())
          {
            std::pair<size_type, size_type> __pos
                = _M_position (__payload.data (), __payload.size ());
            std::size_t __offlen = __pos.first + __pos.second;
            __result.push_back (std::make_pair (__payload.begin (), __offlen));
            __payload = std::span<value_type>{ __payload.begin () + __offlen,
                                               __payload.end () };
            __n -= __offlen;
          }
      }
    return __result;
  }

  std::pair<size_type, size_type>
  _M_position () const
  {
    return _M_position (_Base::data (), _Base::size ());
  }

  std::pair<size_type, size_type>
  _M_position (const_pointer __ptr, size_type __size) const
  {
    if (__size == 0)
      return std::make_pair (0, 0);
    _Byte __prefix = *__ptr;
    if (__prefix < 0x80)
      return std::make_pair (0, 1);
    if (__prefix <= 0xb7)
      {
        std::size_t __strlen = __prefix - 0x80;
        assert (__size > __strlen);
        return std::make_pair (1, __strlen);
      }
    if (__prefix <= 0xbf)
      {
        std::size_t __nstrlen = __prefix - 0xb7;
        assert (__size > __nstrlen);
        std::size_t __strlen;
        __detail::__bytes_to_unsigned (
            std::span<const _Byte> (__ptr + 1, __ptr + 1 + __nstrlen),
            __strlen);
        assert (__size > __nstrlen + __strlen);
        return std::make_pair (1 + __nstrlen, __strlen);
      }
    if (__prefix <= 0xf7)
      {
        std::size_t __nstrlen = __prefix - 0xc0;
        assert (__size > __nstrlen);
        return std::make_pair (1, __nstrlen);
      }
    if (__prefix <= 0xff)
      {
        std::size_t __nstrlen = __prefix - 0xf7;
        assert (__size > __nstrlen);
        std::size_t __strlen;
        __detail::__bytes_to_unsigned (
            std::span<const _Byte> (__ptr + 1, __ptr + 1 + __nstrlen),
            __strlen);
        assert (__size > __nstrlen + __strlen);
        return std::make_pair (1 + __nstrlen, __strlen);
      }
  }

  template <typename> friend class rlp_item_iterator;
};

template <typename _Byte>
inline bool
operator== (const rlp_item<_Byte> &__x, const rlp_item<_Byte> &__y)
{
  return __x.compare (__y) == 0;
}

template <typename _Byte>
inline bool
operator!= (const rlp_item<_Byte> &__x, const rlp_item<_Byte> &__y)
{
  return __x.compare (__y) != 0;
}

template <typename _Byte = byte>
class rlp_item_iterator 
  : private std::span<_Byte>
{
  typedef std::span<_Byte> _Base;
  typedef rlp_item<_Byte> _Rlp_item_type;

public:
  typedef std::input_iterator_tag iterator_category;
  typedef _Rlp_item_type value_type;
  typedef std::ptrdiff_t difference_type;
  typedef const value_type *pointer;
  typedef const value_type &reference;

public:
  template <typename... _Args>
  rlp_item_iterator (_Args &&...__args)
      : _Base (std::forward<_Args> (__args)...), _M_value ()
  {
    _M_iter = _Base::begin ();
    _M_next ();
  }

  rlp_item_iterator (const _Rlp_item_type &__item)
      : _Base (__item.begin (), __item.end ()), _M_value ()
  {
    _M_iter = _Base::begin ();
    _M_next ();
  }

  rlp_item_iterator &operator= (const rlp_item_iterator &) = default;
  ~rlp_item_iterator () = default;

  const value_type &
  operator* () const
  {
    return _M_value;
  }

  const value_type *
  operator->() const
  {
    return std::addressof (_M_value);
  }

  rlp_item_iterator &
  operator++ ()
  {
    _M_next ();
    return *this;
  }

  rlp_item_iterator
  operator++ (int)
  {
    rlp_item_iterator __tmp = *this;
    ++*this;
    return __tmp;
  }

private:
  void
  _M_next ()
  {
    typename _Base::iterator __end = _Base::end ();
    if (_M_iter != __end)
      {
        typedef typename _Base::size_type _Size_type;
        // offset, length
        std::pair<_Size_type, _Size_type> __pos
            = _Rlp_item_type (_M_iter, __end)._M_position ();
        std::size_t __offlen = __pos.first + __pos.second;
        _M_value = _Rlp_item_type (_M_iter, _M_iter + __offlen);
        _M_iter += __offlen;
      }
    else
      {
        _M_done = true;
      }
  }

public:
  bool
  _M_equal (const rlp_item_iterator &__x) const
  {
    if (_M_done && __x._M_done)
      return true;
    return _Base::data () == __x.data () && _Base::size () == __x.size ()
           && _M_iter == __x._M_iter && _M_value == __x._M_value;
  }

  friend bool
  operator== (const rlp_item_iterator &__x, const rlp_item_iterator &__y)
  {
    return __x._M_equal (__y);
  }

  friend bool
  operator!= (const rlp_item_iterator &__x, const rlp_item_iterator &__y)
  {
    return !__x._M_equal (__y);
  }

private:
  _Rlp_item_type _M_value;
  typename _Base::iterator _M_iter;
  bool _M_done;
};

template <typename _Byte, typename _Alloc> class _Rlp_buffer_base
{
protected:
  typedef std::vector<_Byte, _Alloc> _Buffer_type;

public:
  typedef typename _Buffer_type::value_type value_type;
  typedef typename _Buffer_type::pointer pointer;
  typedef typename _Buffer_type::const_pointer const_pointer;
  typedef typename _Buffer_type::reference reference;
  typedef typename _Buffer_type::const_reference const_reference;
  typedef typename _Buffer_type::size_type size_type;
  typedef typename _Buffer_type::difference_type difference_type;
  typedef typename _Buffer_type::iterator iterator;
  typedef typename _Buffer_type::const_iterator const_iterator;
  typedef typename _Buffer_type::reverse_iterator reverse_iterator;
  typedef typename _Buffer_type::const_reverse_iterator const_reverse_iterator;
  typedef typename _Buffer_type::allocator_type allocator_type;

  constexpr _Rlp_buffer_base () = default;
  constexpr explicit _Rlp_buffer_base (const _Alloc &__alloc) noexcept
      : _M_rlp_encoder (__alloc)
  {
  }
  constexpr explicit _Rlp_buffer_base (size_type __count,
                                       const _Alloc &__alloc = _Alloc ())
      : _M_rlp_encoder (__count, __alloc)
  {
  }

  constexpr const_reference
  at (size_type __pos) const
  {
    return _M_get_buffer ().at (__pos);
  }

  constexpr const_reference
  operator[] (size_type __pos) const
  {
    return _M_get_buffer ()[__pos];
  }

  constexpr const_reference
  back () const
  {
    return _M_get_buffer ().back ();
  }
  constexpr const_reference
  front () const
  {
    return _M_get_buffer ().front ();
  }

  constexpr iterator
  begin () noexcept
  {
    return _M_get_buffer ().begin ();
  }

  constexpr const_iterator
  begin () const noexcept
  {
    return _M_get_buffer ().begin ();
  }

  constexpr iterator
  end () noexcept
  {
    return _M_get_buffer ().end ();
  }

  constexpr const_iterator
  end () const noexcept
  {
    return _M_get_buffer ().end ();
  }

  constexpr iterator
  rbegin () noexcept
  {
    return _M_get_buffer ().rbegin ();
  }

  constexpr const_iterator
  rbegin () const noexcept
  {
    return _M_get_buffer ().rbegin ();
  }

  constexpr iterator
  rend () noexcept
  {
    return _M_get_buffer ().rend ();
  }

  constexpr const_iterator
  rend () const noexcept
  {
    return _M_get_buffer ().rend ();
  }

  constexpr size_type
  size () const noexcept
  {
    return _M_get_buffer ().size ();
  }

  constexpr size_type
  empty () const noexcept
  {
    return _M_get_buffer ().empty ();
  }

  constexpr value_type *
  data () noexcept
  {
    return _M_get_buffer ().data ();
  }
  constexpr const value_type *
  data () const noexcept
  {
    return _M_get_buffer ().data ();
  }

  constexpr allocator_type
  get_allocator () const noexcept
  {
    return _M_get_buffer ().get_allocator ();
  }

  constexpr void
  clear () noexcept
  {
    return _M_get_buffer ().clear ();
  }

protected:
  class _Rlp_encoder_base
  {
  public:
    constexpr _Rlp_encoder_base () = default;
    constexpr explicit _Rlp_encoder_base (const _Alloc &__alloc) noexcept
        : _M_buffer (__alloc)
    {
    }
    constexpr explicit _Rlp_encoder_base (size_type __count,
                                          const _Alloc &__alloc = _Alloc ())
        : _M_buffer (__count, __alloc)
    {
    }

    constexpr void
    _M_append (const _Buffer_type &__bytes)
    {
      _M_buffer.insert (_M_buffer.end (), __bytes.begin (), __bytes.end ());
    }

    constexpr void
    _M_do_encode (value_type __c)
    {
      _M_buffer.push_back (__c);
    }

    constexpr void
    _M_do_encode (const _Buffer_type &__bytes, _Byte __prefix1,
                  _Byte __prefix2)
    {
      std::size_t __n = __bytes.size ();
      if (__n < 56)
        {
          _M_buffer.push_back (static_cast<_Byte> (__prefix1 + __n));
          _M_buffer.insert (_M_buffer.end (), __bytes.begin (),
                            __bytes.end ());
        }
      else
        {
          _Buffer_type __nstrlen;
          __detail::__unsigned_to_bytes (__n, __nstrlen);
          std::size_t __len = __nstrlen.size ();
          if (__len + __prefix2 > 0xff)
            throw std::runtime_error ("Too large RLP");
          _M_buffer.push_back (static_cast<_Byte> (__len + __prefix2));
          _M_buffer.insert (_M_buffer.end (), __nstrlen.begin (),
                            __nstrlen.end ());
          _M_buffer.insert (_M_buffer.end (), __bytes.begin (),
                            __bytes.end ());
        }
    }

  private:
    friend class _Rlp_buffer_base;
    _Buffer_type _M_buffer;
  };

  class _Rlp_encoder : public _Rlp_encoder_base
  {
  public:
    using _Rlp_encoder_base::_M_do_encode;
    using _Rlp_encoder_base::_Rlp_encoder_base;

    constexpr void
    _M_do_rlp_data_encode (const _Buffer_type &__bytes)
    {
      std::size_t __n = __bytes.size ();
      if (__n == 1 && __bytes.front () < 0x80)
        _M_do_encode (__bytes.front ());
      else
        _M_do_encode (__bytes, 0x80, 0xb7);
    }

    constexpr void
    _M_do_rlp_list_encode (const _Buffer_type &__bytes)
    {
      _M_do_encode (__bytes, 0xc0, 0xf7);
    }
  };

  constexpr _Buffer_type &
  _M_get_buffer ()
  {
    return _M_rlp_encoder._M_buffer;
  }

  constexpr const _Buffer_type &
  _M_get_buffer () const
  {
    return _M_rlp_encoder._M_buffer;
  }

  constexpr _Rlp_encoder &
  _M_get_rlp_encoder ()
  {
    return _M_rlp_encoder;
  }

private:
  _Rlp_encoder _M_rlp_encoder;
};

template <typename _Byte = byte, typename _Alloc = std::allocator<_Byte> >
class rlp_buffer : public _Rlp_buffer_base<_Byte, _Alloc>
{
  typedef _Rlp_buffer_base<_Byte, _Alloc> _Base;
  typedef typename _Base::_Buffer_type _Buffer_type;

  using _Base::_M_get_rlp_encoder;

public:
  constexpr rlp_buffer &
  put (bigint __val)
  {
    _Buffer_type __buffer;
    if (!__val)
      _M_get_rlp_encoder ()._M_do_rlp_data_encode (__buffer);
    else
      {
        boost::multiprecision::export_bits (__val,
                                            std::back_inserter (__buffer), 8);
        _M_get_rlp_encoder ()._M_do_rlp_data_encode (__buffer);
      }
    return *this;
  }

  template <typename _Tp>
  requires std::is_unsigned_v<_Tp> constexpr rlp_buffer &
  put (_Tp __val)
  {
    _Buffer_type __buffer;
    __detail::__unsigned_to_bytes (__val, __buffer);
    _M_get_rlp_encoder ()._M_do_rlp_data_encode (__buffer);
    return *this;
  }

  constexpr rlp_buffer &
  put (const _Buffer_type &__bytes)
  {
    _M_get_rlp_encoder ()._M_do_rlp_data_encode (__bytes);
    return *this;
  }

  constexpr rlp_buffer &
  put (const std::string &__str)
  {
    std::size_t __prefix_offset = 0;
    if (_M_has_hex_prefixed (__str))
      __prefix_offset = 2;
    _Buffer_type __buffer (__str.begin () + __prefix_offset, __str.end ());
    _M_get_rlp_encoder ()._M_do_rlp_data_encode (__buffer);
    return *this;
  }

  constexpr rlp_buffer &
  put (const char *__str)
  {
    if (__str == nullptr)
      return put (_Buffer_type ());
    return put (std::string (__str));
  }

  template <typename _Range>
  requires std::ranges::input_range<_Range> constexpr rlp_buffer &
  putl (_Range &&__range)
  {
    using __iterator = std::ranges::iterator_t<_Range>;
    rlp_buffer __rlp_buffer;
    __iterator __begin = std::ranges::begin (__range),
               __end = std::ranges::end (__range);
    for (; __begin != __end; ++__begin)
      {
        __rlp_buffer.put (*__begin);
      }

    _M_get_rlp_encoder ()._M_do_rlp_list_encode (
        __rlp_buffer._M_get_buffer ());
    return *this;
  }

  constexpr rlp_buffer &
  putl (const _Buffer_type &__rawlist)
  {
    _M_get_rlp_encoder ()._M_do_rlp_list_encode (__rawlist);
    return *this;
  }

  constexpr rlp_buffer &
  concat (const rlp_buffer &__raw)
  {
    _M_get_rlp_encoder ()._M_append (__raw._M_get_buffer ());
    return *this;
  }

  constexpr rlp_buffer &
  concat (const _Buffer_type &__raw)
  {
    _M_get_rlp_encoder ()._M_append (__raw);
    return *this;
  }

private:
  constexpr bool
  _M_has_hex_prefixed (const std::string &__str) const noexcept
  {
    return __str.length () >= 2 && __str[0] == '0' && __str[1] == 'x';
  }
};

} // namespace basic

#endif //__LIBBASIC_RLP_H__