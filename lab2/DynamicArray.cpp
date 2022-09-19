#include <cstdlib>
#include <iostream>

template<typename T>
class Array final {
    uint32_t capacity_{ 16 };
    T* data_{ nullptr };
    constexpr static uint32_t capacity_scale_{ 2 };
public:
    class Iterator final {
    public:
        const T& get() const;
        void set(const T& value);
        void next();
        bool has_next() const;

        T& operator*();
        const T& operator*() const;
    };

    using const Iterator ConstIterator;

    Array();
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

// Iterator implementation

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
