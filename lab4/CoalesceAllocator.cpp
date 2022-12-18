#include <Windows.h>

#include "CoalesceAllocator.h"

void CoalescePage::init()
{
    header_.next_page = nullptr;
    free_block_ = (CoalesceBlockHeader*)((char*)this + sizeof(CoalescePageHeader));
    free_block_->next_free_block = free_block_;
    free_block_->prev_free_block = free_block_;
    free_block_->size = coalesce_page_size;
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
    CoalesceBlockHeader* fit_block = free_block_;

    // try to find fit block by size
    while (fit_block->size < size && fit_block->next_free_block != free_block_) {
        fit_block = fit_block->next_free_block;
    }

    if (fit_block->size < size) { // fit block not found, go to next page
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
    right_block->size = fit_block->size - size;
    right_block->next_free_block = free_block_->next_free_block;
    right_block->prev_free_block = fit_block->prev_free_block;
    right_block->next_free_block->prev_free_block = right_block;
    right_block->prev_free_block->next_free_block = right_block;
    free_block_ = right_block;

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
    while (p != (char*)block + sizeof(CoalesceBlockHeader)) {
        block = (CoalesceBlockHeader*)(char*)block + block->size;
    }

    // attach freed block to free-list
    block->next_free_block = free_block_->next_free_block;
    block->prev_free_block = free_block_;
    free_block_->next_free_block->prev_free_block = block;
    free_block_->next_free_block = block;

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
    return page_->alloc(size);
}

bool CoalesceAllocator::free(void* p)
{
    return page_->free(p);
}

#ifndef NDEBUG
void CoalesceAllocator::dumpStat() const
{

}
void CoalesceAllocator::dumpBlocks() const
{

}
#endif
