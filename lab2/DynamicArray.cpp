#include <cstdlib>
#include <iostream>
#include <string>

#include <cppunit/TestResult.h>
#include <cppunit/TestListener.h>
#include <cppunit/TestFailure.h>
#include <cppunit/extensions/HelperMacros.h>

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
    , size_{ 0 }
    {
        debug_log("Array(capacity:" + std::to_string(size_) + ")");
        data_ = static_cast<T*>(malloc(sizeof(T) * capacity_));
        memset(data_, 0, sizeof(T) * capacity_);
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
        debug_log("Array::remove(uint32_t index:" + std::to_string(index) + ")");
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
    const Iterator begin() const
    {
        debug_log("Array::cbegin() const");
        return Iterator(data_, &data_[size_ - 1], data_);
    }

    Iterator end()
    {
        debug_log("Array::end()");
        return Iterator(data_, &data_[size_ - 1], &data_[size_ - 1]);
    }
    const Iterator end() const
    {
        debug_log("Array::cend() const");
        return Iterator(data_, &data_[size_ - 1], &data_[size_ - 1]);
    }
};

class DynamicArrayTest : public CPPUNIT_NS::TestFixture
{
    using TestArray = Array<uint32_t>;
    using ConstTestArray = const Array<uint32_t>;
public:
    void test_empty_creation()
    {
        TestArray arr;
        CPPUNIT_ASSERT_EQUAL(true, arr.capacity() > 0);
    }
    void test_creation_with_capacity()
    {
        TestArray arr(20);
        CPPUNIT_ASSERT_EQUAL(true, arr.capacity() >= 20);
    }
    void test_access_operator()
    {
        TestArray arr;
        arr.insert(0u);
        CPPUNIT_ASSERT_EQUAL(0u, arr[0]);
    }
    void test_insert_at_the_end()
    {
        TestArray arr;
        arr.insert(0u);
        arr.insert(1u);
        arr.insert(2u);
        CPPUNIT_ASSERT_EQUAL(0u, arr[0]);
        CPPUNIT_ASSERT_EQUAL(1u, arr[1]);
        CPPUNIT_ASSERT_EQUAL(2u, arr[2]);
    }
    void test_insert_in_the_middle()
    {
        TestArray arr;
        // insert some data
        arr.insert(1u);
        arr.insert(2u);
        // insert new data in the middle
        arr.insert(1, 0u);

        CPPUNIT_ASSERT_EQUAL(1u, arr[0]);
        CPPUNIT_ASSERT_EQUAL(0u, arr[1]);
        CPPUNIT_ASSERT_EQUAL(2u, arr[2]);
    }
    void test_remove_from_the_end()
    {
        TestArray arr;
        arr.insert(0u);
        arr.insert(1u);
        arr.insert(2u);
        // remove last element
        arr.remove(2);
        CPPUNIT_ASSERT_EQUAL(true, arr.size() == 2);
        CPPUNIT_ASSERT_EQUAL(0u, arr[0]);
        CPPUNIT_ASSERT_EQUAL(1u, arr[1]);
    }
    void test_remove_from_the_middle()
    {
        TestArray arr;
        arr.insert(0u);
        arr.insert(1u);
        arr.insert(2u);
        // remove middle element
        arr.remove(1);
        CPPUNIT_ASSERT_EQUAL(true, arr.size() == 2);
        CPPUNIT_ASSERT_EQUAL(0u, arr[0]);
        CPPUNIT_ASSERT_EQUAL(2u, arr[1]);
    }
    void test_iterator()
    {
        TestArray arr;
        arr.insert(0u);
        arr.insert(1u);
        arr.insert(2u);
        auto iter = arr.iterator();
        CPPUNIT_ASSERT_EQUAL(0u, iter.get());
        iter.next();
        CPPUNIT_ASSERT_EQUAL(1u, iter.get());
        iter.next();
        CPPUNIT_ASSERT_EQUAL(2u, iter.get());
    }
    void test_reverse_iterator()
    {
        TestArray arr;
        arr.insert(0u);
        arr.insert(1u);
        arr.insert(2u);
        auto iter = arr.reverseIterator();
        CPPUNIT_ASSERT_EQUAL(2u, iter.get());
        iter.next();
        CPPUNIT_ASSERT_EQUAL(1u, iter.get());
        iter.next();
        CPPUNIT_ASSERT_EQUAL(0u, iter.get());
    }
    void test_iterator_loop()
    {
        TestArray arr;
        arr.insert(0u);
        arr.insert(1u);
        arr.insert(2u);
        auto iter = arr.iterator();
        for (; iter.has_next(); iter.next());
        CPPUNIT_ASSERT_EQUAL(2u, iter.get());
    }
    void test_reverse_iterator_loop()
    {
        TestArray arr;
        arr.insert(0u);
        arr.insert(1u);
        arr.insert(2u);
        auto iter = arr.reverseIterator();
        for (; iter.has_next(); iter.next());
        CPPUNIT_ASSERT_EQUAL(0u, iter.get());
    }
    void test_range_loop()
    {
        TestArray arr;
        arr.insert(0u);
        arr.insert(1u);
        arr.insert(2u);
        uint32_t i = 0u;
        for (uint32_t& element : arr)
        {
            CPPUNIT_ASSERT_EQUAL(i, element);
            ++i;
        }
    }
    void test_const_range_loop()
    {
        TestArray arr;
        arr.insert(0u);
        arr.insert(1u);
        arr.insert(2u);
        ConstTestArray const_arr{ arr };
        uint32_t i = 0u;
        for (auto& element : const_arr)
        {
            CPPUNIT_ASSERT_EQUAL(i, element);
            ++i;
        }
    }
    void test_destructor_call()
    {
        bool is_removed1{ false }, is_removed2{ false };
        class TestType {
        private:
            bool* is_removed_;
            int32_t index_{ 0 };
            std::string name_{};
        public:
            TestType(bool* is_removed) : is_removed_{ is_removed } {}
            ~TestType() { *is_removed_ = true; }
        } destructor_call_test_object1{ &is_removed1 }, destructor_call_test_object2{ &is_removed2 };
        Array<TestType>* arr = new Array<TestType>(20);
        arr->insert(destructor_call_test_object1);
        arr->insert(destructor_call_test_object2);
        delete arr;
        CPPUNIT_ASSERT_EQUAL(true, is_removed1);
        CPPUNIT_ASSERT_EQUAL(true, is_removed2);
    }
};

int main()
{
    CppUnit::TestSuite* test_suite = new CppUnit::TestSuite("DynamicArray TestSuite");
    {
        test_suite->addTest(new CppUnit::TestCaller<DynamicArrayTest>("create empty array", &DynamicArrayTest::test_empty_creation));
        test_suite->addTest(new CppUnit::TestCaller<DynamicArrayTest>("create array with capacity", &DynamicArrayTest::test_creation_with_capacity));
        test_suite->addTest(new CppUnit::TestCaller<DynamicArrayTest>("access operator", &DynamicArrayTest::test_access_operator));
        test_suite->addTest(new CppUnit::TestCaller<DynamicArrayTest>("insert at the end", &DynamicArrayTest::test_insert_at_the_end));
        test_suite->addTest(new CppUnit::TestCaller<DynamicArrayTest>("insert in the middle", &DynamicArrayTest::test_insert_in_the_middle));
        test_suite->addTest(new CppUnit::TestCaller<DynamicArrayTest>("remove from the end", &DynamicArrayTest::test_remove_from_the_end));
        test_suite->addTest(new CppUnit::TestCaller<DynamicArrayTest>("remove from the middle", &DynamicArrayTest::test_remove_from_the_middle));
        test_suite->addTest(new CppUnit::TestCaller<DynamicArrayTest>("iterator", &DynamicArrayTest::test_iterator));
        test_suite->addTest(new CppUnit::TestCaller<DynamicArrayTest>("reverse iterator", &DynamicArrayTest::test_reverse_iterator));
        test_suite->addTest(new CppUnit::TestCaller<DynamicArrayTest>("iterator loop", &DynamicArrayTest::test_iterator_loop));
        test_suite->addTest(new CppUnit::TestCaller<DynamicArrayTest>("reverse iterator loop", &DynamicArrayTest::test_reverse_iterator_loop));
        test_suite->addTest(new CppUnit::TestCaller<DynamicArrayTest>("range loop", &DynamicArrayTest::test_range_loop));
        test_suite->addTest(new CppUnit::TestCaller<DynamicArrayTest>("const range loop", &DynamicArrayTest::test_const_range_loop));
        test_suite->addTest(new CppUnit::TestCaller<DynamicArrayTest>("destructor call on remove", &DynamicArrayTest::test_destructor_call));
    }
    bool is_errored{ false };
    class TestListener : public CppUnit::TestListener
    {
        bool* is_errored_ = nullptr;
    public:
        TestListener(bool* is_errored) : is_errored_{ is_errored } {
        }
        void addFailure(const CppUnit::TestFailure& failure) override
        {
            std::cerr << "ERROR\t\tTest failed: " << failure.failedTestName() << std::endl;
            *is_errored_ = true;
        }

        void startTest(CppUnit::Test* test) override
        {
            std::cout << "\tStarting test: " << test->getName() << std::endl;
        }
        void endTest(CppUnit::Test* test) override
        {
            std::cout << "\tTest ended: " << test->getName() << std::endl;
        }

        void startSuite(CppUnit::Test* suite) override
        {
            std::cout << "Starting suite: " << suite->getName() << std::endl;
        }

        void endSuite(CppUnit::Test* suite) override
        {
            std::cout << "Suite ended: " << suite->getName() << std::endl;
        }

    } test_listener(&is_errored);
    CppUnit::TestResult test_result;
    test_result.addListener(&test_listener);
    test_suite->run(&test_result);
    if (is_errored) {
        std::cerr << "\n\nSome fails occured while running tests\n\n" << std::endl;
        return 1;
    } else {
        std::cout << "\n\nAll tests passed successfuly\n\n" << std::endl;
    }

    return EXIT_SUCCESS;
}
