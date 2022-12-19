#include <Windows.h>
#include <iostream>
#include <cassert>

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

    if (size <= 16) { return fsa16.alloc(); }
    if (size <= 32) { return fsa32.alloc(); }
    if (size <= 64) { return fsa64.alloc(); }
    if (size <= 128) { return fsa128.alloc(); }
    if (size <= 256) { return fsa256.alloc(); }
    if (size <= 512) { return fsa512.alloc(); }
    if (size <= coalesce_page_size) { return coalesce.alloc(size); }

    native_blocks_.push_back({size, VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)});

    return native_blocks_.back().buffer;
}

void MemoryAllocator::free(void* p)
{
    std::lock_guard<std::mutex> lg(mutex_);

    if (fsa16.free(p)) { return; }
    if (fsa32.free(p)) { return; }
    if (fsa64.free(p)) { return; }
    if (fsa128.free(p)) { return; }
    if (fsa256.free(p)) { return; }
    if (fsa512.free(p)) { return; }
    if (coalesce.free(p)) { return; }

    for (size_t i = 0; i < native_blocks_.size(); ++i) {
        if (native_blocks_[i].buffer == p) {
            VirtualFree(p, 0, MEM_RELEASE);
            native_blocks_.erase(native_blocks_.begin() + i);
            return;
        }
    }
    assert(!"can not find allocator for this pointer");
}

#ifndef NDEBUG
void MemoryAllocator::dumpStat() const
{
    std::cout << "Dump Stat:" << std::endl;

    std::cout << "------" << std::endl;
    std::cout << "fsa16" << std::endl;
    std::cout << "------" << std::endl;
    fsa16.dumpStat();
    std::cout << "------" << std::endl;
    std::cout << std::endl;

    std::cout << "------" << std::endl;
    std::cout << "fsa32" << std::endl;
    std::cout << "------" << std::endl;
    fsa32.dumpStat();
    std::cout << "------" << std::endl;
    std::cout << std::endl;

    std::cout << "------" << std::endl;
    std::cout << "fsa64" << std::endl;
    std::cout << "------" << std::endl;
    fsa64.dumpStat();
    std::cout << "------" << std::endl;
    std::cout << std::endl;

    std::cout << "------" << std::endl;
    std::cout << "fsa128" << std::endl;
    std::cout << "------" << std::endl;
    fsa128.dumpStat();
    std::cout << "------" << std::endl;
    std::cout << std::endl;

    std::cout << "------" << std::endl;
    std::cout << "fsa256" << std::endl;
    std::cout << "------" << std::endl;
    fsa256.dumpStat();
    std::cout << "------" << std::endl;
    std::cout << std::endl;

    std::cout << "------" << std::endl;
    std::cout << "fsa512" << std::endl;
    std::cout << "------" << std::endl;
    fsa512.dumpStat();
    std::cout << "------" << std::endl;
    std::cout << std::endl;

    std::cout << "------" << std::endl;
    std::cout << "coalesce" << std::endl;
    std::cout << "------" << std::endl;
    coalesce.dumpStat();
    std::cout << "------" << std::endl;
    std::cout << std::endl;

    std::cout << "------" << std::endl;
    std::cout << "native" << std::endl;
    std::cout << "------" << std::endl;
    std::cout << "OS block count: " << native_blocks_.size() << std::endl;
    std::cout << "------" << std::endl;
}
void MemoryAllocator::dumpBlocks() const
{
    std::cout << "Dump Blocks:" << std::endl;
    fsa16.dumpBlocks();
    fsa32.dumpBlocks();
    fsa64.dumpBlocks();
    fsa128.dumpBlocks();
    fsa256.dumpBlocks();
    fsa512.dumpBlocks();
    coalesce.dumpBlocks();

    for (auto& nb : native_blocks_) {
        std::cout << "Allocated block:" << std::endl;
        std::cout << "\taddress: " << (void*)nb.buffer << std::endl;
        std::cout << "\tsize: " << nb.size << std::endl;
        std::cout << std::endl;
    }
}
#endif
