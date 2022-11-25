#include "QuickSort.h"
#include "../lab2/DynamicArray.h"
#include <gtest/gtest.h>

constexpr int32_t test_elements_count = 100;

template<typename T, typename Compare>
bool is_sorted(T* first, T* last, Compare comp) {
    last--;
    while (first < last) {
        if (comp(*last, *first)) {
            return false;
        }
        first++;
    }
    return true;
}

auto int_comp = [](const int32_t& a, const int32_t& b) -> bool { return a < b; };

TEST(sort_one_element_array, QuickSortTesting)
{
    int32_t arr[1]{};
    QuickSort(arr, arr + sizeof(arr) / sizeof(arr[0]), int_comp);
    EXPECT_TRUE(is_sorted(arr, arr + sizeof(arr) / sizeof(arr[0]), int_comp));
}

TEST(sort_two_elements_array, QuickSortTesting)
{
    int32_t arr[2] = {1, 0};
    QuickSort(arr, arr + sizeof(arr) / sizeof(arr[0]), int_comp);
    EXPECT_TRUE(is_sorted(arr, arr + sizeof(arr) / sizeof(arr[0]), int_comp));
}

TEST(sort_sorted_array, QuickSortTesting)
{
    int32_t arr[10];
    for (int32_t i = 0; i < 10; ++i) {
        arr[i] = i;
    }
    QuickSort(arr, arr + sizeof(arr) / sizeof(arr[0]), int_comp);
    EXPECT_TRUE(is_sorted(arr, arr + sizeof(arr) / sizeof(arr[0]), int_comp));
}

TEST(sort_reverse_sorted_array, QuickSortTesting)
{
    int32_t arr[10];
    for (int32_t i = 0; i < 10; ++i) {
        arr[i] = 10 - i;
    }
    QuickSort(arr, arr + sizeof(arr) / sizeof(arr[0]), int_comp);
    EXPECT_TRUE(is_sorted(arr, arr + sizeof(arr) / sizeof(arr[0]), int_comp));
}

TEST(sort_random_filled_array, QuickSortTesting)
{
    int32_t arr[test_elements_count];
    for (int32_t i = 0; i < test_elements_count; ++i) {
        arr[i] = rand() % test_elements_count;
    }
    QuickSort(arr, arr + sizeof(arr) / sizeof(arr[0]), int_comp);
    EXPECT_TRUE(is_sorted(arr, arr + sizeof(arr) / sizeof(arr[0]), int_comp));
}

TEST(sort_equal_array, QuickSortTesting)
{
    int32_t arr[test_elements_count];
    for (int32_t i = 0; i < test_elements_count; ++i) {
        arr[i] = 1;
    }
    QuickSort(arr, arr + sizeof(arr) / sizeof(arr[0]), int_comp);
    EXPECT_TRUE(is_sorted(arr, arr + sizeof(arr) / sizeof(arr[0]), [](int32_t a, int32_t b) -> bool { return a < b; }));
}

TEST(sort_lot_of_median_array, QuickSortTesting)
{
    int32_t arr[10];
    for (int32_t i = 0; i < 3; ++i) {
        arr[i] = i;
    }
    for (int32_t i = 3; i < 7; ++i) {
        arr[i] = 5;
    }
    for (int32_t i = 7; i < 10; ++i) {
        arr[i] = i;
    }
    QuickSort(arr, arr + sizeof(arr) / sizeof(arr[0]), int_comp);
    EXPECT_TRUE(is_sorted(arr, arr + sizeof(arr) / sizeof(arr[0]), [](int32_t a, int32_t b) -> bool { return a < b; }));
}

TEST(sort_empty_dynamic_array, QuickSortTesting)
{
    Array<int32_t> arr;
    QuickSort((int32_t*)arr.begin(), (int32_t*)arr.end(), int_comp);
    EXPECT_TRUE(is_sorted((int32_t*)arr.begin(), (int32_t*)arr.end(), int_comp));
}

TEST(sort_random_filled_dynamic_array, QuickSortTesting)
{
    Array<int32_t> arr(test_elements_count);
    for (int32_t i = 0; i < test_elements_count; ++i) {
        arr.insert(rand() % test_elements_count);
    }
    auto comp = int_comp;
    QuickSort((int32_t*)arr.begin(), (int32_t*)arr.end(), comp);
    EXPECT_TRUE(is_sorted((int32_t*)arr.begin(), (int32_t*)arr.end(), comp));
}

TEST(sort_dynamic_array_with_strings, QuickSortTesting)
{
    Array<std::string> arr(test_elements_count);
    for (int32_t i = 0; i < test_elements_count; ++i) {
        arr.insert(std::to_string(rand() % test_elements_count));
    }
    auto comp = [](const std::string& a, const std::string& b) -> bool { return strcmp(a.c_str(), b.c_str()) < 0; };
    QuickSort<std::string>(arr.begin(), arr.end(), comp);
    EXPECT_TRUE(is_sorted<std::string>(arr.begin(), arr.end(), comp));
}
