#pragma once

#include <cstdlib>
#include <iostream>
#include <string>

void debug_log(const std::string& msg)
{
#ifndef _NDEBUG
    //std::cout << msg.c_str() << std::endl;
#endif
}

template<typename T>
class Array final {
    uint32_t capacity_{ 16 };
    uint32_t size_{ 0 };
    T* data_{ nullptr };
    constexpr static uint32_t capacity_scale_{ 2 };
public:
    class Iterator final {
        T* cur_{ nullptr };
        const T* begin_{ nullptr }; // if begin_ greater than end_, it is reverse iterator
        const T* end_{ nullptr };
    public:
        Iterator(T* begin, T* end, T* cur)
            : begin_{ begin }
            , end_{ end }
            , cur_{ cur }
        {}
        const T& get() const
        {
            return *cur_;
        }
        void set(const T& value)
        {
            *cur_ = value;
        }
        void next()
        {
            begin_ < end_ ? ++cur_ : --cur_;
        }
        bool has_next() const {
            return begin_ < end_ ? cur_ + 1 != end_ : cur_ - 1 != end_;
        }

        T& operator*() { return *cur_; }
        const T& operator*() const { return *cur_; }

        operator T*() { return cur_; }
        operator const T*() const { return cur_; }

        Iterator& operator++() // prefix ++
        {
            next();
            return *this;
        }
        Iterator operator++(int) // postfix ++
        {
            Iterator ret = *this;
            next();
            return ret;
        }

        Iterator& operator--() // prefix --
        {
            begin_ < end_ ? --cur_ : ++cur_;
            return *this;
        }
        Iterator operator--(int) // postfix --
        {
            Iterator ret = *this;
            begin_ < end_ ? --cur_ : ++cur_;
            return ret;
        }

        bool operator!=(const Iterator& other)
        {
            return (begin_ != other.begin_) || (end_ != other.end_) || (cur_ != other.cur_);
        }
    };

    Array()
    {
        debug_log("Array()");
        data_ = static_cast<T*>(malloc(sizeof(T) * capacity_));
        //memset(data_, 0, sizeof(T) * capacity_);
    }
    Array(uint32_t capacity)
    : capacity_{ capacity }
    , size_{ 0 }
    {
        debug_log("Array(capacity:" + std::to_string(capacity_) + ")");
        if (capacity_ == 0) {
            data_ = nullptr;
        } else {
            data_ = static_cast<T*>(malloc(sizeof(T) * capacity_));
            //memset(data_, 0, sizeof(T) * capacity_);
        }
    }
    ~Array()
    {
        debug_log("~Array()");
        for (uint32_t i = 0; i < size_; ++i) {
            static_cast<T*>(&data_[i])->~T();
        }
        free(data_);
        data_ = nullptr;
        capacity_ = 0;
        size_ = 0;
    }

    Array(const Array& other)
    : capacity_{ other.capacity_ }
    , size_{ other.size_ }
    {
        debug_log("Array(const Array& other)");
        data_ = static_cast<T*>(malloc(sizeof(T) * capacity_));
        // memcpy(data_, other.data_, sizeof(T) * capacity_); // copy all data (by capacity_) to avoid memset(0)
        for (uint32_t i = 0; i < size_; ++i) {
            new(data_ + i) T(other.data_[i]);
        }
    }

    uint32_t insert(const T& value) // push_back
    {
        debug_log("Array::insert(const T& value)");
        return insert(size_, value); // insert at the end of array
    }
    uint32_t insert(uint32_t index, const T& value)
    {
        debug_log("Array::insert(uint32_t index:" + std::to_string(index) + ", const T& value)");
        ++size_; // increment array size
        if (size_ > capacity_) { // reallocating data buffer if size greater than capacity
            T* new_data = static_cast<T*>(malloc(sizeof(T) * capacity_ * capacity_scale_));
            for (uint32_t i = 0; i < size_ - 1; ++i) {
                new(new_data + i) T(data_[i]);
                static_cast<T*>(&data_[i])->~T();
            }
            free(data_);
            data_ = new_data;
            capacity_ *= capacity_scale_;
        }
        // move tail of array right from the last element
        for (int32_t i = size_ - 1; i > static_cast<int32_t>(index); --i) { // index may be equal to zero, signed iterator needed
            new(data_ + i) T(data_[i - 1]);
            static_cast<T*>(&data_[i - 1])->~T();
        }

        new(data_ + index) T(value);

        return index;
    }

    void remove(uint32_t index)
    {
        debug_log("Array::remove(uint32_t index:" + std::to_string(index) + ")");
        --size_; // decrement array size
        // call element destructor
        static_cast<T*>(&data_[index])->~T();
        // move tail of array left from removed element's index
        for (uint32_t i = index; i < size_; ++i) {
            new(data_ + i) T(data_[i + 1]);
            static_cast<T*>(&data_[i + 1])->~T();
        }
    }

    const T& operator[](uint32_t index) const
    {
        debug_log("Array::operator[](uint32_t index:" + std::to_string(index) + ") const");
        return data_[index];
    }
    T& operator[](uint32_t index)
    {
        debug_log("Array::operator[](uint32_t index:" + std::to_string(index) + ")");
        return data_[index];
    }

    [[nodiscard]] uint32_t size() const
    {
        debug_log("Array::size()");
        return size_;
    }

    // for tests only
    [[nodiscard]] uint32_t capacity() const
    {
        debug_log("Array::capacity()");
        return capacity_;
    }

    Iterator iterator()
    {
        debug_log("Array::iterator()");
        return begin();
    }
    const Iterator iterator() const
    {
        debug_log("Array::iterator() const");
        return cbegin();
    }

    Iterator reverseIterator()
    {
        debug_log("Array::reverseIterator()");
        return Iterator(data_ + size_ - 1, data_ - 1, data_ + size_ - 1);
    }
    const Iterator reverseIterator() const
    {
        debug_log("Array::reverseIterator() const");
        return Iterator(data_ + size_ - 1, data_ - 1, data_ + size_ - 1);
    }

    Iterator begin()
    {
        debug_log("Array::begin()");
        if (size_ == 0) {
            return Iterator(data_, data_, data_);
        }
        return Iterator(data_, data_ + size_, data_);
    }
    const Iterator begin() const
    {
        debug_log("Array::cbegin() const");
        if (size_ == 0) {
            return Iterator(data_, data_, data_);
        }
        return Iterator(data_, data_ + size_, data_);
    }

    Iterator end()
    {
        debug_log("Array::end()");
        if (size_ == 0) {
            return Iterator(data_, data_, data_);
        }
        return Iterator(data_, data_ + size_, data_ + size_);
    }
    const Iterator end() const
    {
        debug_log("Array::cend() const");
        if (size_ == 0) {
            return Iterator(data_, data_, data_);
        }
        return Iterator(data_, data_ + size_, data_ + size_);
    }
};
