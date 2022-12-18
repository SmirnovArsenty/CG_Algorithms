#pragma once
#include <cstdint>

struct FSAPageHeader {
    class FSAPage* next_page;
    int16_t free_list_root;
    int16_t block_size;
};

class FSAPage {
private:
    FSAPageHeader header_;
public:
    FSAPage() = default;
    void init(uint16_t block_size, uint16_t block_count);
    void destroy();
    void* alloc();
    bool free(void* p);
};

class FixedSizeAllocator
{
public:
    FixedSizeAllocator(uint16_t block_size);
    ~FixedSizeAllocator();

    void init();
    void destroy();

    void* alloc(); // by block size
    bool free(void* p);

#ifndef NDEBUG
    void dumpStat() const;
    void dumpBlocks() const;
#endif
private:
    uint16_t block_size_;
    FSAPage* page_;
};
