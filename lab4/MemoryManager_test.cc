#include "MemoryManager.h"
#include <gtest/gtest.h>

TEST(FSA16, MemoryManager)
{
    MemoryAllocator alloc;
    alloc.init();

    void* p[16];
    for (int i = 0; i < 16; ++i) {
        p[i] = alloc.alloc(i + 1);
        *((bool*)p[i]) = true;
    }
    for (int i = 0; i < 16; ++i) {
        EXPECT_TRUE(*((bool*)p[i]));
    }
    for (int i = 0; i < 16; ++i) {
        alloc.free(p[i]);
    }

    alloc.dumpBlocks();
    alloc.dumpStat();

    alloc.destroy();
}

TEST(FSA16_new_page, MemoryManager)
{
    MemoryAllocator alloc;
    alloc.init();

    void* p[1000];

    for (int i = 0; i < 1000; ++i) {
        p[i] = alloc.alloc(8);
        *(int8_t*)p[i] = 3;
    }
    for (int i = 0; i < 1000; ++i) {
        EXPECT_TRUE(*((int8_t*)p[i]) == 3);
    }
    for (int i = 0; i < 1000; ++i) {
        alloc.free(p[i]);
    }

    alloc.dumpBlocks();
    alloc.dumpStat();

    alloc.destroy();
}

TEST(FSA512, MemoryManager)
{
    MemoryAllocator alloc;
    alloc.init();

    void* p[10];

    char str[400] = "10qwert0yuiop[asdfghjkl;zxcvbnm,vgbhjnndfnvd;lvalbvadfvhjlbaslkfnsmnvlkzdfvksbfcoshifojihfasdfhaklsnfbaajglkhsdlgjersignld;jblislkjslhfbadkj;ghksdbvokanvoibvnaiohbvearnvoaervoajefoakfmakjghaojgkldjbfvfqwertyuiop[asdfghjkl;zxcvbnm,vgbhjnndfnvd;lvalbvadfvhjlbaslkfnsmnvlkzdfvksbfcoshifojihfasdfhaklsnfbaajglkhsdlgjersignld;jblislkjslhfbadkj;ghksdbvokanvoibvnaiohbvearnvoaervoajefoakfmakjghaojgkldjbfvf";
    for (int i = 0; i < 10; ++i) {
        p[i] = alloc.alloc(400);
        memcpy((char*)p[i], str, 400);
    }
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(strcmp((char*)p[i], str) == 0);
    }
    for (int i = 0; i < 10; ++i) {
        alloc.free(p[i]);
    }

    alloc.dumpBlocks();
    alloc.dumpStat();

    alloc.destroy();
}

TEST(coalesce, MemoryManager)
{
    MemoryAllocator alloc;
    alloc.init();

    void* p = alloc.alloc(1000);

    for (int32_t i = 0; i < 1000; ++i) {
        ((char*)p)[i] = i % 0xFF;
    }

    for (int32_t i = 0; i < 1000; ++i) {
        EXPECT_EQ(((unsigned char*)p)[i], i % 0xFF);
    }

    alloc.free(p);

    alloc.dumpBlocks();
    alloc.dumpStat();

    alloc.destroy();
}

TEST(native, MemoryManager)
{
    MemoryAllocator alloc;
    alloc.init();

    const size_t size = size_t(1e8);

    void* p = alloc.alloc(size);

    for (size_t i = 0; i < size; ++i) {
        ((unsigned char*)p)[i] = 0xFF;
    }

    for (size_t i = 0; i < size; ++i) {
        EXPECT_EQ(((unsigned char*)p)[i], 0xFF);
    }

    alloc.free(p);

    alloc.dumpBlocks();
    alloc.dumpStat();

    alloc.destroy();
}
