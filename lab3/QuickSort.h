#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>

constexpr int32_t insert_sort_elements = (1 << 6); // 64

template<typename T, typename Compare>
void insert_sort(T* first, T* last, Compare comp)
{
    for (auto i = first; i <= last; i++)
    {
        auto j = i - 1;

        T value = *(j + 1);
        while (j >= first && comp(value, *j))
        {
            *(j + 1) = std::move(*j);
            j--;
        }
        *(j + 1) = value;
    }
}

template<typename T, typename Compare>
T* partition(T* first, T* last, Compare comp)
{
    T* mid = first + (last - first) / 2;
    if (comp(*mid, *first)) {
        std::swap(*mid, *first);
    }
    if (comp(*last, *first)) {
        std::swap(*last, *first);
    }
    if (comp(*last, *mid)) {
        std::swap(*last, *mid);
    }

    return mid;
}

template<typename T, typename Compare>
void QuickSort(T* first, T* last, Compare comp)
{
    if (last < first) {
        throw std::runtime_error("invalid input data");
    }

    last--;
    while (first < last) {
        if (last - first < insert_sort_elements) {
            return insert_sort(first, last, comp);
        }

        T* mid = partition(first, last, comp);

        std::swap(*mid, *(last - 1));
        auto pivot = *(last - 1);

        T* i = first;
        T* j = last - 1;

        while (true) {
            while (comp(*(++i), pivot));
            while (comp(pivot, *(--j)));
            if (i >= j) break;

            std::swap(*(i), *(j));
        }
        std::swap(*(last - 1), *(i));
        *(i) = pivot;

        if (i - first < last - i) {
            QuickSort(first, i, comp);
            first = i + 1;
        } else {
            QuickSort(i + 1, last + 1, comp);
            last = i - 1;
        }
    }
}
