#include "DynamicArray.h"
#include <cassert>

int main()
{
    Array<std::string> arr;
    // insert some data
    arr.insert("11111111111111");
    arr.insert("22");
    // insert new data in the middle
    arr.insert(1, "00000");

    assert("11111111111111" == arr[0]);
    assert("00000" == arr[1]);
    assert("22" == arr[2]);
}
