#include <Windows.h>
#include <cstdlib>
#include <cassert>

#include "FixedSizeAllocator.h"

// SYSTEM_INFO sysInfo;
// GetSystemInfo(&sysInfo);
// sysInfo.dwPageSize;

constexpr int16_t last_block_index = -1;

void FSAPage::init(uint16_t block_size, uint16_t block_count)
{
    header_.free_list_root = 0;
    header_.next_page = nullptr;
    char* buffer = (char*)this;
    for (int16_t i = 0; i < block_count; ++i) {
        buffer += block_size;
        *((int16_t*)buffer) = i + 1;
        if (i == block_count - 1) {
            *((int16_t*)buffer) = last_block_index;
        }
    }
}

void FSAPage::destroy()
{
    if (header_.next_page) {
        header_.next_page->destroy();
        header_.next_page = nullptr;
    }
    VirtualFree(this, 0, MEM_RELEASE);
}

void* FSAPage::alloc()
{
    if (header_.free_list_root != last_block_index) {
        void* ret = (char*)this + sizeof(FSAPageHeader) + header_.free_list_root * header_.block_size;

        // move free-list root forward
        header_.free_list_root = *((int16_t*)ret); // set next block to free-list root

        return ret;
    } else {
        if (!header_.next_page) { // initialize next page
            SYSTEM_INFO sysInfo;
            GetSystemInfo(&sysInfo);
            uint32_t block_count = (sysInfo.dwPageSize - sizeof(FSAPage)) / header_.block_size;
            header_.next_page = (FSAPage*)VirtualAlloc(NULL, sizeof(FSAPageHeader) + header_.block_size * block_count, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            header_.next_page->init(header_.block_size, block_count);
        }
        return header_.next_page->alloc();
    }
}

bool FSAPage::free(void* p)
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    if (p < (char*)this || p >= (char*)this + sysInfo.dwPageSize) {
        // find block index in page
        int16_t index = ((char*)p - (char*)this - sizeof(FSAPageHeader)) / header_.block_size;

        // update free-list root
        *((uint16_t*)p) = header_.free_list_root;
        header_.free_list_root = index;
        return true;
    } else if (header_.next_page) {
        return header_.next_page->free(p);
    } else {
        return false;
    }
}

FixedSizeAllocator::FixedSizeAllocator(uint16_t block_size)
    : block_size_{ block_size }
    , page_{ nullptr }
{
    static_assert(sizeof(FSAPageHeader) <= 16, "reduce FSAPageHeader size"); // check FSAPage size; 16 is the smallest block size used
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    assert(sysInfo.dwPageSize > 512); // page size must be greater than max block_size
}

FixedSizeAllocator::~FixedSizeAllocator()
{
}

void FixedSizeAllocator::init()
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    uint32_t block_count = (sysInfo.dwPageSize - sizeof(FSAPage)) / block_size_;
    page_ = (FSAPage*)VirtualAlloc(NULL, sizeof(FSAPageHeader) + block_size_ * block_count, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    page_->init(block_size_, block_count);
}

void FixedSizeAllocator::destroy()
{
    page_->destroy();
}

void* FixedSizeAllocator::alloc()
{
    return page_->alloc();
}

bool FixedSizeAllocator::free(void* p)
{
    return page_->free(p);
}

#ifndef NDEBUG
void FixedSizeAllocator::dumpStat() const
{

}

void FixedSizeAllocator::dumpBlocks() const
{

}
#endif
