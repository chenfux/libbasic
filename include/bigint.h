#ifndef __LIBCRYPTOCPP_BIGINT_H__
#define __LIBCRYPTOCPP_BIGINT_H__

#include <boost/multiprecision/cpp_int.hpp>

namespace basic
{

using bigint
    = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<> >;
using uint64
    = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<
        64, 64, boost::multiprecision::unsigned_magnitude,
        boost::multiprecision::unchecked, void> >;
using uint128
    = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<
        128, 128, boost::multiprecision::unsigned_magnitude,
        boost::multiprecision::unchecked, void> >;
using uint256
    = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<
        256, 256, boost::multiprecision::unsigned_magnitude,
        boost::multiprecision::unchecked, void> >;
using sint256
    = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<
        256, 256, boost::multiprecision::signed_magnitude,
        boost::multiprecision::unchecked, void> >;
using uint160
    = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<
        160, 160, boost::multiprecision::unsigned_magnitude,
        boost::multiprecision::unchecked, void> >;
using sint160
    = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<
        160, 160, boost::multiprecision::signed_magnitude,
        boost::multiprecision::unchecked, void> >;
using uint512
    = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<
        512, 512, boost::multiprecision::unsigned_magnitude,
        boost::multiprecision::unchecked, void> >;
using sint512
    = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<
        512, 512, boost::multiprecision::signed_magnitude,
        boost::multiprecision::unchecked, void> >;

template <std::size_t _Nu>
using __uint
    = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<
        _Nu * 8, _Nu * 8, boost::multiprecision::unsigned_magnitude,
        boost::multiprecision::unchecked, void> >;

template <std::size_t _Nu>
using __sint
    = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<
        _Nu * 8, _Nu * 8, boost::multiprecision::signed_magnitude,
        boost::multiprecision::unchecked, void> >;

template <typename _Tp>
struct __is_boost_multiprecision_number_impl 
    : public std::false_type
{ };

template <typename... _Args>
struct __is_boost_multiprecision_number_impl<
    boost::multiprecision::number<_Args...> > 
    : public std::true_type
{ };

template <typename _Tp>
using __is_boost_multiprecision_number
    = __is_boost_multiprecision_number_impl<_Tp>;

}
#endif //__LIBCRYPTOCPP_BIGINT_H__