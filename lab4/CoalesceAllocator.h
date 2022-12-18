#pragma once
#include <cstdint>

constexpr uint32_t coalesce_page_size = 10000000;

struct CoalesceBlockHeader
{
    CoalesceBlockHeader* next_free_block;
    CoalesceBlockHeader* prev_free_block;
    uint32_t size;
};

struct CoalescePageHeader
{
    class CoalescePage* next_page;
};

class CoalescePage
{
public:
    void init();
    void destroy();
    void* alloc(size_t size);
    bool free(void* p);
private:
    CoalescePageHeader header_;
    CoalesceBlockHeader* free_block_;
};

class CoalesceAllocator
{
public:
    CoalesceAllocator();
    ~CoalesceAllocator();

    void init();
    void destroy();

    void* alloc(size_t size);
    bool free(void* p);

#ifndef NDEBUG
    void dumpStat() const;
    void dumpBlocks() const;
#endif
private:
    CoalescePage* page_;
};
