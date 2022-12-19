#pragma once

#include "CoalesceAllocator.h"
#include "FixedSizeAllocator.h"

#include <mutex>
#include <vector>

class MemoryAllocator
{
public:
    MemoryAllocator();
    virtual ~MemoryAllocator();

    virtual void init();
    virtual void destroy();

    virtual void* alloc(size_t size);
    virtual void free(void* p);

#ifdef NDEBUG
    virtual void dumpStat() const {};
    virtual void dumpBlocks() const {};
#else
    virtual void dumpStat() const;
    virtual void dumpBlocks() const;
#endif
private:
    FixedSizeAllocator fsa16;
    FixedSizeAllocator fsa32;
    FixedSizeAllocator fsa64;
    FixedSizeAllocator fsa128;
    FixedSizeAllocator fsa256;
    FixedSizeAllocator fsa512;

    CoalesceAllocator coalesce;

    struct NativeBlock
    {
        size_t size;
        void* buffer;
    };
    std::vector<NativeBlock> native_blocks_;

    std::mutex mutex_;
};
