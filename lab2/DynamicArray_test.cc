#include <gtest/gtest.h>
#include "DynamicArray.h"

TEST(test_empty_creation, DynamicArray_tests)
{
    Array<std::string> arr;
    EXPECT_GT(arr.capacity(), 0);
}

TEST(test_creation_with_capacity, DynamicArray_tests)
{
    Array<std::string> arr(20);
    EXPECT_EQ(arr.capacity(), 20);
}

TEST(test_access_operator, DynamicArray_tests)
{
    Array<std::string> arr;
    arr.insert("00000");
    EXPECT_EQ("00000", arr[0]);
}

TEST(test_insert_at_the_end, DynamicArray_tests)
{
    Array<std::string> arr;
    arr.insert("00000");
    arr.insert("11111111111111");
    arr.insert("22");
    EXPECT_EQ("00000", arr[0]);
    EXPECT_EQ("11111111111111", arr[1]);
    EXPECT_EQ("22", arr[2]);
}

TEST(test_insert_in_the_middle, DynamicArray_tests)
{
    Array<std::string> arr;
    // insert some data
    arr.insert("11111111111111");
    arr.insert("22");
    // insert new data in the middle
    arr.insert(1, "00000");

    EXPECT_EQ("11111111111111", arr[0]);
    EXPECT_EQ("00000", arr[1]);
    EXPECT_EQ("22", arr[2]);
}

TEST(test_remove_from_the_end, DynamicArray_tests)
{
    Array<std::string> arr;
    arr.insert("00000");
    arr.insert("11111111111111");
    arr.insert("22");
    // remove last element
    arr.remove(2);
    EXPECT_EQ(arr.size(), 2);
    EXPECT_EQ("00000", arr[0]);
    EXPECT_EQ("11111111111111", arr[1]);
}

TEST(test_remove_from_the_middle, DynamicArray_tests)
{
    Array<std::string> arr;
    arr.insert("00000");
    arr.insert("11111111111111");
    arr.insert("22");
    // remove middle element
    arr.remove(1);
    EXPECT_EQ(2, arr.size());
    EXPECT_EQ("00000", arr[0]);
    EXPECT_EQ("22", arr[1]);
}

TEST(test_iterator, DynamicArray_tests)
{
    Array<std::string> arr;
    arr.insert("00000");
    arr.insert("11111111111111");
    arr.insert("22");
    auto iter = arr.iterator();
    EXPECT_EQ("00000", iter.get());
    iter.next();
    EXPECT_EQ("11111111111111", iter.get());
    iter.next();
    EXPECT_EQ("22", iter.get());
}

TEST(test_reverse_iterator, DynamicArray_tests)
{
    Array<std::string> arr;
    arr.insert("00000");
    arr.insert("11111111111111");
    arr.insert("22");
    auto iter = arr.reverseIterator();
    EXPECT_EQ("22", iter.get());
    iter.next();
    EXPECT_EQ("11111111111111", iter.get());
    iter.next();
    EXPECT_EQ("00000", iter.get());
}

TEST(test_iterator_loop, DynamicArray_tests)
{
    Array<std::string> arr;
    arr.insert("00000");
    arr.insert("11111111111111");
    arr.insert("22");
    auto iter = arr.iterator();
    for (; iter.has_next(); iter.next());
    EXPECT_EQ("22", iter.get());
}

TEST(test_reverse_iterator_loop, DynamicArray_tests)
{
    Array<std::string> arr;
    arr.insert("00000");
    arr.insert("11111111111111");
    arr.insert("22");
    auto iter = arr.reverseIterator();
    for (; iter.has_next(); iter.next());
    EXPECT_EQ("00000", iter.get());
}

TEST(test_range_loop, DynamicArray_tests)
{
    Array<std::string> arr;
    arr.insert("00000");
    arr.insert("11111111111111");
    arr.insert("22");
    const std::string test[] = {"00000", "11111111111111", "22"};
    uint32_t i = 0u;
    for (auto& element : arr) {
        EXPECT_EQ(test[i++], element);
    }
}

TEST(test_const_range_loop, DynamicArray_tests)
{
    Array<std::string> arr;
    arr.insert("00000");
    arr.insert("11111111111111");
    arr.insert("22");
    const Array<std::string> const_arr{ arr };
    const std::string test[] = {"00000", "11111111111111", "22"};
    uint32_t i = 0u;
    for (auto& element : const_arr) {
        EXPECT_EQ(test[i++], element);
    }
}

TEST(test_realloc, DynamicArray_tests)
{
    Array<std::string> arr(2u);
    arr.insert("ashcd");
    arr.insert("vljgnbw");
    EXPECT_EQ(2u, arr.capacity());
    // realloc here
    arr.insert("oibafhv");
    EXPECT_GT(arr.capacity(), 2u);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
