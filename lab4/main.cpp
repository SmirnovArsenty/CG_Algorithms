#include "MemoryManager.h"

int main()
{
    MemoryAllocator allocator;
    allocator.init();

    int *pi = (int *)allocator.alloc(sizeof(int));
    *pi = 5;
    double *pd = (double *)allocator.alloc(sizeof(double));
    *pd = 1.0;

    int *pa = (int *)allocator.alloc(10 * sizeof(int));
    for (int i = 0; i < 10; ++i) {
        pa[i] = 1;
    }
    allocator.dumpStat();
    allocator.dumpBlocks();
    allocator.free(pa);
    allocator.free(pd);
    allocator.free(pi);
    allocator.destroy();
}
