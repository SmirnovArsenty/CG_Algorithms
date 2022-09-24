#include <cstdlib>
#include <iostream>

template<typename T>
class Array final {
    uint32_t capacity_{ 16 };
    T* data_{ nullptr };
    constexpr static uint32_t capacity_scale_{ 2 };
public:
    class Iterator final {
        T* cur_{ nullptr };
        T* begin_{ nullptr }; // if begin_ greater than end_, it is reverse iterator
        T* end_{ nullptr };
    public:
        Iterator(T* begin, T* end)
            : begin_{ begin }
            , end_{ end }
        {
            cur_ = begin_;
        }
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
    };

    Array() :
    
    {

    }
    Array(uint32_t capacity);
    ~Array();

    Array(const Array& other);

    int insert(const T& value);
    int insert(uint32_t index, const T& value);

    void remove(uint32_t index);

    const T& operator[](uint32_t index) const;
    T& operator[](uint32_t index);

    uint32_t size() const;

    Iterator iterator();
    ConstIterator iterator() const;

    Iterator reverseIterator();
    ConstIterator reverseIterator() const;

    Iterator begin();
    ConstIterator cbegin() const;

    Iterator end();
    ConstIterator cend() const;
};

int main() {
    Array<int16_t> a;
    // insertion test
    for (uint32_t i = 0; i < 10; ++i) {
        a.insert(i + 1);
    }
    // operator[] test
    for (uint32_t i = 0; i < a.size(); ++i) {
        a[i] *= 2;
    }
    // iterators test
    for (auto it = a.iterator(); it.has_next(); it.next()) {
        std::cout << it.get() << std::endl;
    }

    // range-for test
    for (auto it : a) {
        std::cout << it.get() << std::endl;
    }

    return 0;
}
