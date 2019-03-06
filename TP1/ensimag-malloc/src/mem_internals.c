/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>
#include "mem.h"
#include "mem_internals.h"

unsigned long knuth_mmix_one_round(unsigned long in)
{
    return in * 6364136223846793005UL % 1442695040888963407UL;
}

void *mark_memarea_and_get_user_ptr(void *ptr, unsigned long size, MemKind k)
{
    //Calcul de la valeur magique à laquelle on garde les 62 premiers bits
    unsigned long magicValue = (knuth_mmix_one_round(size) & ~(0b111UL));
    //On ajoute les 2 derniers bits (Memkind k est défini dan mem_internals.h)
    magicValue = magicValue | (k & 0b111UL);
    //On ajoute la taille aux 8 premiers et aux 8 derniers octets
    *(unsigned int *)ptr = size;
    *(unsigned int*)(ptr + size - 8) = size;
    //On ajoute la magicValue entre les octets 8 et 16 et entre les octets -16 et -8
    *(unsigned int *)(ptr + 8) = magicValue;
    *(unsigned int*)(ptr + size - 16) = magicValue;
    //On retourne la première zone utilisable
    return (void *)(ptr + 16);
}

Alloc mark_check_and_get_alloc(void *ptr)
{
    /* ecrire votre code ici */
    Alloc a = {};
    a.ptr = ptr - 16;
    a.size = *(unsigned int *)(ptr - 16);
    a.kind = *(unsigned int *)(ptr - 8) & 0b111UL;
    assert(*(unsigned int *)(ptr - 8) == *(unsigned int *)(ptr + a.size - 32));
    return a;
}


unsigned long mem_realloc_small() {
    assert(arena.chunkpool == 0);
    unsigned long size = (FIRST_ALLOC_SMALL << arena.small_next_exponant);
    arena.chunkpool = mmap(0,
			   size,
			   PROT_READ | PROT_WRITE | PROT_EXEC,
			   MAP_PRIVATE | MAP_ANONYMOUS,
			   -1,
			   0);
    if (arena.chunkpool == MAP_FAILED)
	handle_fatalError("small realloc");
    arena.small_next_exponant++;
    return size;
}

unsigned long mem_realloc_medium() {
    uint32_t indice = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant;
    assert(arena.TZL[indice] == 0);
    unsigned long size = (FIRST_ALLOC_MEDIUM << arena.medium_next_exponant);
    assert( size == (1 << indice));
    arena.TZL[indice] = mmap(0,
			     size*2, // twice the size to allign
			     PROT_READ | PROT_WRITE | PROT_EXEC,
			     MAP_PRIVATE | MAP_ANONYMOUS,
			     -1,
			     0);
    if (arena.TZL[indice] == MAP_FAILED)
	handle_fatalError("medium realloc");
    // align allocation to a multiple of the size
    // for buddy algo
    arena.TZL[indice] += (size - (((intptr_t)arena.TZL[indice]) % size));
    arena.medium_next_exponant++;
    return size; // lie on allocation size, but never free
}


// used for test in buddy algo
unsigned int nb_TZL_entries() {
    int nb = 0;

    for(int i=0; i < TZL_SIZE; i++)
	if ( arena.TZL[i] )
	    nb ++;

    return nb;
}
