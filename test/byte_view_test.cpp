
#include "byte_view.h"
#include <cstring>
#include <gtest/gtest.h>

using namespace basic;

TEST (ByteViewTest, IsByteLike)
{
  static_assert (is_byte_like_v<unsigned char>);
  static_assert (is_byte_like_v<char>);
  static_assert (is_byte_like_v<signed char>);
  static_assert (!is_byte_like_v<int>);
  static_assert (is_byte_like_v<std::byte>);
}

TEST (ByteViewTest, BasicByteViewConstructor)
{
  unsigned char data[] = { 0x01, 0x02, 0x03, 0x04 };
  mutable_byte_view view1 (data, sizeof (data));
  byte_view view2 (data, sizeof (data));

  EXPECT_EQ (view1.size (), view2.size ());
  EXPECT_EQ ((const void*)view1.data (), (const void*)data);
  EXPECT_EQ ((const void*)view2.data (), (const void*)data);
  EXPECT_TRUE (std::equal (view1.begin (), view1.end (), view2.begin ()));

  std::vector<unsigned char> vec = { 0x05, 0x06, 0x07, 0x08 };
  const auto &const_vec = vec;
  mutable_byte_view view3 (vec);
  byte_view view4 (const_vec);

  EXPECT_EQ (view3.size (), view4.size ());
  EXPECT_EQ ((const void*)view3.data (), (const void*)vec.data ());
  EXPECT_EQ ((const void*)view4.data (), (const void*)vec.data ());
  EXPECT_TRUE (std::equal (view3.begin (), view3.end (), view4.begin ()));
}

TEST (ByteViewTest, SubviewFunction)
{
  unsigned char data[] = { 0x01, 0x02, 0x03, 0x04 };
  mutable_byte_view view1 (data, sizeof (data));

  auto subview1 = view1.subview (1, 2);
  EXPECT_EQ (subview1.size (), 2);
  EXPECT_EQ (subview1.data (), view1.data () + 1);
  EXPECT_EQ (subview1[0], std::byte{0x02});
  EXPECT_EQ (subview1[1], std::byte{0x03});
}

TEST (ByteViewTest, AsByteViewFunctionObject)
{
  int arr[] = { 0x0a, 0x0b, 0x0c };
  auto view5 = as_byte_view (arr);

  EXPECT_EQ (view5.size (), sizeof (arr));
  EXPECT_EQ (std::memcmp (view5.data (), arr, sizeof (arr)), 0);

  std::vector<int> vec2 = { 0x0d, 0x0e };
  const auto &const_vec2 = vec2;
  auto view6 = as_byte_view (vec2);
  auto view7 = as_byte_view (const_vec2);

  EXPECT_EQ (view6.size (), sizeof (int) * vec2.size ());
  EXPECT_EQ (view7.size (), sizeof (int) * const_vec2.size ());
  EXPECT_TRUE (std::equal (view6.begin (), view6.end (), view7.begin ()));
}

TEST (ByteViewTest, AsByteViewWithIteratorsAndCount)
{
  char str[] = "Hello";
  auto view8 = as_byte_view (str, 3);

  EXPECT_EQ (view8.size (), 3);
  EXPECT_EQ (view8[0], std::byte{'H'});
  EXPECT_EQ (view8[1], std::byte{'e'});
  EXPECT_EQ (view8[2], std::byte{'l'});

  auto view9 = as_byte_view (str + 1, 4);

  EXPECT_EQ (view9.size (), 4);
  EXPECT_EQ (view9[0], std::byte{'e'});
  EXPECT_EQ (view9[1], std::byte{'l'});
  EXPECT_EQ (view9[2], std::byte{'l'});
  EXPECT_EQ (view9[3], std::byte{'o'});

  std::vector<unsigned int> vec3 = { 0xdeadbeef, 0xcafebabe };
  auto view10 = as_byte_view (vec3.begin (), vec3.end ());

  EXPECT_EQ (view10.size (), sizeof (unsigned int) * vec3.size ());
}

TEST (ByteViewTest, SubviewConstructorWithBasicByteView)
{
  unsigned char data[] = { 0x01, 0x02, 0x03, 0x04 };
  mutable_byte_view view1 (data, sizeof (data));

  auto subview2 = as_byte_view (view1, 2);
  EXPECT_EQ (subview2.size (), 2);
  EXPECT_EQ (subview2[0], std::byte{0x01});
  EXPECT_EQ (subview2[1], std::byte{0x02});
}

TEST (ByteViewTest, InvalidSubviewConstruction)
{
  // unsigned char data[] = { 0x01, 0x02, 0x03, 0x04 };
  // mutable_byte_view view1 (data, sizeof (data));

  // bool threw_exception = false;
  // try
  //   {
  //     auto subview3 = view1.subview (5, 2); // invalid offset
  //   }
  // catch (std::exception &)
  //   { // Expected
  //     threw_exception = true;
  //   }
  // EXPECT_TRUE (threw_exception);

  // threw_exception = false;
  // try
  //   {
  //     auto subview4 = view1.subview (1, 10); // invalid size
  //   }
  // catch (std::exception &)
  //   {
  //     threw_exception = true;
  //   }
  // EXPECT_TRUE (threw_exception);
}