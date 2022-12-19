#include <Windows.h>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <vector>
#include <iomanip>

#include "FixedSizeAllocator.h"

constexpr int16_t last_block_index = -1;

void FSAPage::init(uint16_t block_size, uint16_t block_count)
{
    header_.free_list_root = 0;
    header_.next_page = nullptr;
    header_.block_size = block_size;
    char* buffer = (char*)this + sizeof(FSAPageHeader);
    for (int16_t i = 0; i < block_count; ++i) {
        *((int16_t*)buffer) = i + 1;
        if (i == block_count - 1) {
            *((int16_t*)buffer) = last_block_index;
        }
        buffer += block_size;
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
        if (header_.next_page == nullptr) { // initialize next page
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
    uint32_t block_count = (sysInfo.dwPageSize - sizeof(FSAPage)) / header_.block_size;
    if (p > (char*)this && p < (char*)this + sizeof(FSAPageHeader) + header_.block_size * block_count) {
        // find block index in page
        int16_t index = (int16_t)(((char*)p - ((char*)this + sizeof(FSAPageHeader))) / header_.block_size);

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
#ifndef NDEBUG
    ++alloc_count_;
#endif
    return page_->alloc();
}

bool FixedSizeAllocator::free(void* p)
{
#ifndef NDEBUG
    ++free_count_;
#endif
    return page_->free(p);
}

#ifndef NDEBUG
void FixedSizeAllocator::dumpStat() const
{
    // busy and free blocks
    uint32_t busy_blocks = 0;
    uint32_t free_blocks = 0;
    uint32_t page_count = 0;

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    uint32_t block_count_per_page = (sysInfo.dwPageSize - sizeof(FSAPage)) / block_size_;

    FSAPage* page = page_;
    while (page != nullptr)
    {
        ++page_count;
        FSAPageHeader header = *((FSAPageHeader*)page);
        int16_t fh = header.free_list_root;
        while (fh != last_block_index) {
            fh = *(int16_t*)((char*)page + sizeof(FSAPageHeader) + fh * block_size_);
            ++free_blocks;
        }
        busy_blocks += block_count_per_page - free_blocks;

        page = header.next_page;
    }

    std::cout << "\tUsed blocks: " << busy_blocks << std::endl;
    std::cout << "\tFree blocks: " << free_blocks << std::endl;


    // OS allocated blocks
    std::cout << "\tOS allocated pages: " << page_count << " (" << page_count * sysInfo.dwPageSize << " bytes)" << std::endl;
    
    // memory returned to user
    std::cout << "\tAlloc count: " << alloc_count_ << std::endl;
    std::cout << "\tFree count: " << free_count_ << std::endl;
}

void FixedSizeAllocator::dumpBlocks() const
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    uint32_t block_count_per_page = (sysInfo.dwPageSize - sizeof(FSAPage)) / block_size_;
    FSAPage* page = page_;
    while (page != nullptr)
    {
        FSAPageHeader* header = ((FSAPageHeader*)page);
        std::vector<bool> free_blocks(block_count_per_page);
        int16_t fh = header->free_list_root;
        while (fh != last_block_index) {
            free_blocks[fh] = 1;
            fh = *(int16_t*)((char*)page + sizeof(FSAPageHeader) + fh * block_size_);
        }

        for (uint32_t i = 0; i < block_count_per_page; ++i) {
            if (!free_blocks[i]) {
                std::cout << "Allocated block:" << std::endl;
                std::cout << "\taddress: " << (void*)((char*)page + sizeof(FSAPageHeader) + i * block_size_) << std::endl;
                std::cout << "\tsize: " << block_size_ << std::endl;
                std::cout << std::endl;
            }
        }

        page = header->next_page;
    }
}
#endif
