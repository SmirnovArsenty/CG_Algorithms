#include <Windows.h>
#include <iostream>
#include <cassert>

#include "CoalesceAllocator.h"

void CoalescePage::init()
{
    header_.next_page = nullptr;
    first_free_block_ = (CoalesceBlockHeader*)((char*)this + sizeof(CoalescePageHeader));
    first_free_block_->next_free_block = nullptr;
    first_free_block_->prev_free_block = nullptr;
    first_free_block_->size = coalesce_page_size;
}

void CoalescePage::destroy()
{
    if (header_.next_page != nullptr) {
        header_.next_page->destroy();
        header_.next_page = nullptr;
    }
    VirtualFree(this, 0, MEM_RELEASE);
}

void* CoalescePage::alloc(size_t size)
{
    CoalesceBlockHeader* fit_block = first_free_block_;

    // try to find fit block by size
    while (fit_block->size < size && fit_block != nullptr) {
        fit_block = fit_block->next_free_block;
    }

    if (fit_block == nullptr) { // fit block not found, go to next page
        if (header_.next_page == nullptr) { // alloc new page
            CoalescePage* page = (CoalescePage*)VirtualAlloc(NULL, coalesce_page_size + sizeof(CoalescePageHeader), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            page->init();
            header_.next_page = page;
        }

        return header_.next_page->alloc(size);
    }

    // split block to right and left
    // right bloc is still free to alloc
    // left returned as the result of alloc
    CoalesceBlockHeader* right_block = (CoalesceBlockHeader*)((char*)fit_block + sizeof(CoalesceBlockHeader) + size);
    right_block->size = fit_block->size - (size + sizeof(CoalesceBlockHeader));
    right_block->next_free_block = fit_block->next_free_block;
    right_block->prev_free_block = fit_block->prev_free_block;
    if (right_block->next_free_block != nullptr) {
        right_block->next_free_block->prev_free_block = right_block;
    }
    if (right_block->prev_free_block != nullptr) {
        right_block->prev_free_block->next_free_block = right_block;
    } else {
        first_free_block_ = right_block;
    }

    // assign size
    fit_block->size = size;
    // mark used
    fit_block->next_free_block = nullptr;
    fit_block->prev_free_block = nullptr;

    // return block without header
    return (char*)fit_block + sizeof(CoalesceBlockHeader);
}

bool CoalescePage::free(void* p)
{
    if (p < this || p > (char*)this + coalesce_page_size + sizeof(CoalescePageHeader)) {
        if (header_.next_page != nullptr) {
            return header_.next_page->free(p);
        } else {
            return false;
        }
    }

    CoalesceBlockHeader* block = (CoalesceBlockHeader*)((char*)this + sizeof(CoalescePageHeader));
    CoalesceBlockHeader* prev_free_block = nullptr;
    while (p != (char*)block + sizeof(CoalesceBlockHeader)) {
        if (block->next_free_block != nullptr || block->prev_free_block != nullptr || block == first_free_block_) {
            prev_free_block = block;
        }
        block = (CoalesceBlockHeader*)(char*)block + block->size;
    }

    CoalesceBlockHeader* next_block = (CoalesceBlockHeader*)((char*)block + block->size + sizeof(CoalesceBlockHeader));

    // attach freed block to free-list
    if (prev_free_block == nullptr) // become first_free_block
    {
        block->next_free_block = first_free_block_;
        block->prev_free_block = nullptr;

        assert(first_free_block_->prev_free_block == nullptr);
        first_free_block_->prev_free_block = block;

        first_free_block_ = block;
    } else {
        block->prev_free_block = prev_free_block;
        block->next_free_block = prev_free_block->next_free_block;

        if (prev_free_block->next_free_block != nullptr) {
            prev_free_block->next_free_block->prev_free_block = block;
        }
        prev_free_block->next_free_block = block;
    }

    if (prev_free_block != nullptr && (char*)prev_free_block + prev_free_block->size == (char*)block)
    {
        prev_free_block->size += block->size + sizeof(CoalesceBlockHeader);
        prev_free_block->next_free_block = block->next_free_block;
        if (block->next_free_block != nullptr) {
            block->next_free_block->prev_free_block = prev_free_block;
        }
        block->next_free_block = nullptr;
        block->prev_free_block = nullptr;
        block->size = 0;
        block->self = nullptr;
        block = prev_free_block;
    }
    if ((char*)next_block < (char*)this + coalesce_page_size + sizeof(CoalescePageHeader) &&
        next_block->next_free_block != nullptr && next_block->prev_free_block != nullptr)
    {
        block->size += next_block->size + sizeof(CoalesceBlockHeader);
        if (next_block->next_free_block != nullptr) {
            next_block->next_free_block->prev_free_block = block;
        }
        block->next_free_block = next_block->next_free_block;

        next_block->next_free_block = nullptr;
        next_block->prev_free_block = nullptr;
        next_block->size = 0;
        next_block->self = nullptr;
    }

    return true;
}

CoalesceAllocator::CoalesceAllocator()
    : page_{ nullptr }
{}
CoalesceAllocator::~CoalesceAllocator() {}

void CoalesceAllocator::init()
{
    page_ = (CoalescePage*)VirtualAlloc(NULL, coalesce_page_size + sizeof(CoalescePageHeader), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    page_->init();
}

void CoalesceAllocator::destroy()
{
    page_->destroy();
}

void* CoalesceAllocator::alloc(size_t size)
{
#ifndef NDEBUG
    ++alloc_count_;
#endif
    return page_->alloc(size);
}

bool CoalesceAllocator::free(void* p)
{
#ifndef NDEBUG
    ++free_count_;
#endif
    return page_->free(p);
}

#ifndef NDEBUG
void CoalesceAllocator::dumpStat() const
{
    uint32_t busy_blocks = 0;
    uint32_t free_blocks = 0;
    uint32_t page_count = 0;

    CoalescePage* page = page_;
    while (page != nullptr) {
        ++page_count;
        CoalesceBlockHeader* block = (CoalesceBlockHeader*)((char*)page + sizeof(CoalescePageHeader));
        while ((char*)block < (char*)page + sizeof(CoalescePageHeader) + coalesce_page_size) {
            if (block->next_free_block == nullptr) {
                ++busy_blocks;
            } else {
                ++free_blocks;
            }
            block = (CoalesceBlockHeader*)((char*)block + block->size);
        }

        CoalescePageHeader page_header = *((CoalescePageHeader*)page);
        page = page_header.next_page;
    }

    std::cout << "\tUsed blocks: " << busy_blocks << std::endl;
    std::cout << "\tFree blocks: " << free_blocks << std::endl;


    // OS allocated blocks
    std::cout << "\tOS allocated pages: " << page_count << " (" << page_count * (coalesce_page_size + sizeof(CoalescePageHeader)) << " bytes)" << std::endl;
    
    // memory returned to user
    std::cout << "\tAlloc count: " << alloc_count_ << std::endl;
    std::cout << "\tFree count: " << free_count_ << std::endl;
}
void CoalesceAllocator::dumpBlocks() const
{
    CoalescePage* page = page_;
    while (page != nullptr) {
        CoalesceBlockHeader* block = (CoalesceBlockHeader*)((char*)page + sizeof(CoalescePageHeader));
        while ((char*)block < (char*)page + sizeof(CoalescePageHeader) + coalesce_page_size) {
            if (block->next_free_block == nullptr) {
                std::cout << "Allocated block:" << std::endl;
                std::cout << "\taddress: " << (void*)block << std::endl;
                std::cout << "\tsize: " << block->size << std::endl;
                std::cout << std::endl;
            }
            block = (CoalesceBlockHeader*)((char*)block + block->size);
        }

        CoalescePageHeader page_header = *((CoalescePageHeader*)page);
        page = page_header.next_page;
    }
}
#endif
