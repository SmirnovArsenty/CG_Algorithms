#include <cstdlib>
#include <iostream>
#include <string>

void debug_log(const std::string& msg)
{
#ifndef _NDEBUG
    std::cout << msg.c_str() << std::endl;
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
        T* begin_{ nullptr }; // if begin_ greater than end_, it is reverse iterator
        T* end_{ nullptr };
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
            return begin_ < end_ ? cur_ < end_ : cur_ > end_;
        }

        T& operator*() { return *cur_; }
        const T& operator*() const { return *cur_; }

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
        memset(data_, 0, sizeof(T) * capacity_);
    }
    Array(uint32_t capacity)
    : capacity_{ capacity }
    , size_{ other.size_ }
    {
        debug_log("Array(capacity:" + std::to_string(size_) + ")");
        data_ = static_cast<T*>(malloc(sizeof(T) * capacity_));
        memset(data_, 0, sizeof(T) * capacity_);
    }
    ~Array()
    {
        debug_log("~Array()");
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
        memcpy(data_, other.data_, sizeof(T) * capacity_); // copy all data (by capacity_) to avoid memset(0)
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
        if (size_ > capacity_) { // reallocating data buffer if size more than capacity
            T* new_data = static_cast<T*>(malloc(sizeof(T) * capacity_ * capacity_scale_));
            memset(new_data, 0, sizeof(T) * capacity_ * capacity_scale_);
            memcpy(new_data, data_, sizeof(T) * capacity_);
            free(data_);
            data_ = new_data;
            capacity_ *= capacity_scale_;
        }
        // move tail of array right from the last element
        for (int32_t i = size_ - 1; i > static_cast<int32_t>(index); --i) { // index may be equal to zero, signed iterator needed
            data_[i] = data_[i - 1];
        }

        data_[index] = value;

        return index;
    }

    void remove(uint32_t index)
    {
        debug_log("Array::remove(uint32_t index:" + std::to_string(index) + ")")
        --size_; // decrement array size
        // move tail of array left from removed element's index
        for (uint32_t i = index; i < size_; ++i) { // index may be equal to zero, signed iterator needed
            data_[i] = data_[i + 1];
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

    uint32_t size() const
    {
        debug_log("Array::size()");
        return size_;
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
        return Iterator(&data_[size_ - 1], data_, &data_[size_ - 1]);
    }
    const Iterator reverseIterator() const
    {
        debug_log("Array::reverseIterator() const");
        return Iterator(&data_[size_ - 1], data_, &data_[size_ - 1]);
    }

    Iterator begin()
    {
        debug_log("Array::begin()");
        return Iterator(data_, &data_[size_ - 1], data_);
    }
    const Iterator cbegin() const
    {
        debug_log("Array::cbegin() const");
        return Iterator(data_, &data_[size_ - 1], data_);
    }

    Iterator end()
    {
        debug_log("Array::end()");
        return Iterator(data_, &data_[size_ - 1], &data_[size_ - 1]);
    }
    const Iterator cend() const
    {
        debug_log("Array::cend() const");
        return Iterator(data_, &data_[size_ - 1], &data_[size_ - 1]);
    }
};

int main() {
    Array<int16_t> arr;
    // insertion test
    for (uint32_t i = 0; i < 30; ++i) {
        arr.insert(i + 1);
    }
    // operator[] test
    for (uint32_t i = 0; i < arr.size(); ++i) {
        arr[i] *= 2;
    }
    // iterators test
    for (auto it = arr.iterator(); it.has_next(); it.next()) {
        std::cout << it.get() << std::endl;
    }

    // range-for test
    for (auto it : arr) {
        std::cout << it << std::endl;
    }

    return EXIT_SUCCESS;
}
