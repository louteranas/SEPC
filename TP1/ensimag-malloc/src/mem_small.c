/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

void *emalloc_small(unsigned long size)
{
    void* tete = arena.chunkpool;
    if(tete == NULL) {
        size = mem_realloc_small();
        unsigned long *ptr = (unsigned long *)((unsigned long *)arena.chunkpool);
        for(int i = 0; i < size / CHUNKSIZE; i++) {
            *ptr = (unsigned long)(ptr + CHUNKSIZE);
            ptr = ptr + CHUNKSIZE / sizeof(unsigned long);
        }
        return emalloc_small(size);
    }
    else {
        arena.chunkpool = (void *)(*(unsigned long *)tete);
        return mark_memarea_and_get_user_ptr(tete, CHUNKSIZE, SMALL_KIND);
    }
    return (void *)0;
}

void efree_small(Alloc a) {
    unsigned long ptr = (unsigned long)a.ptr;
    *((unsigned long *)a.ptr) = *((unsigned long *)arena.chunkpool);
    *((unsigned long *)arena.chunkpool) = ptr;
}
