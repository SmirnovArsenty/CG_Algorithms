#include "MemoryManager.h"

MemoryAllocator::MemoryAllocator()
    : fsa16{ 16u }
    , fsa32{ 32u }
    , fsa64{ 64u }
    , fsa128{ 128u }
    , fsa256{ 256u }
    , fsa512{ 512u }
    , coalesce{}
{
}

MemoryAllocator::~MemoryAllocator()
{
}

void MemoryAllocator::init()
{
    fsa16.init();
    fsa32.init();
    fsa64.init();
    fsa128.init();
    fsa256.init();
    fsa512.init();
    coalesce.init();
}

void MemoryAllocator::destroy()
{
    fsa16.destroy();
    fsa32.destroy();
    fsa64.destroy();
    fsa128.destroy();
    fsa256.destroy();
    fsa512.destroy();
    coalesce.destroy();
}

void* MemoryAllocator::alloc(size_t size)
{
    std::lock_guard<std::mutex> lg(mutex_);

    if (size < 16) { return fsa16.alloc(); }
    if (size < 32) { return fsa32.alloc(); }
    if (size < 64) { return fsa64.alloc(); }
    if (size < 128) { return fsa128.alloc(); }
    if (size < 256) { return fsa256.alloc(); }
    if (size < 512) { return fsa512.alloc(); }
    if (size < coalesce_page_size) { return coalesce.alloc(size); }

    // to OS

    return nullptr;
}

void MemoryAllocator::free(void* p)
{

}

#ifndef NDEBUG
void MemoryAllocator::dumpStat() const
{

}
void MemoryAllocator::dumpBlocks() const
{

}
#endif
