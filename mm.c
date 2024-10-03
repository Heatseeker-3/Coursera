/*
 * mm.c
 *
 * Name 1: [FILL IN]
 * PSU ID 1: [FILL IN]
 *
 * Name 2: [FILL IN]
 * PSU ID 2: [FILL IN]
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 * Also, read malloclab.pdf carefully and in its entirety before beginning.
 *
 */


/*
 * A series of segregated lists is employed to track available memory blocks. The allocation policy follows a first-fit strategy. The segregated lists are categorized into the following size ranges:
 * [16, 32), [32, 64), ..., [33,554,432, 67,108,864), [67,108,864, infinity).
 *
 * Allocated blocks contain a 4-byte header and user data:
 * -------------------------------------------------------------------------
 * |                          Block size                       |    x 0 x   |
 * -------------------------------------------------------------------------
 * |                                                                         |
 * -------------------------------------------------------------------------
 *
 * Free blocks consist of a header, footer, previous block pointer, next block pointer, and data:
 * -------------------------------------------------------------------------
 * |                          Block size                       |    x 0 x   |
 * -------------------------------------------------------------------------
 * |                              Previous block pointer                    |
 * -------------------------------------------------------------------------
 * |                              Next block pointer                        |
 * -------------------------------------------------------------------------
 * |                                                                         |
 * -------------------------------------------------------------------------
 * |                          Block size                       |    x 0 x   |
 * -------------------------------------------------------------------------
 *
 * Free blocks of exactly 8 bytes are treated as a special case, containing only a header and footer. These blocks are excluded from the segregated lists but can merge with adjacent free blocks when either the next or previous block is released:
 * -------------------------------------------------------------------------
 * |                          Block size                       |    x 0 x   |
 * -------------------------------------------------------------------------
 * |                          Block size                       |    x 0 x   |
 * -------------------------------------------------------------------------
 *
 * The least significant three bits in the block size field encode additional state information:
 * 000: Block is free, and the previous block is allocated.
 * 001: Block is free, and the previous block is also free.
 * 100: Block is allocated, and the previous block is allocated.
 * 101: Block is allocated, and the previous block is free.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mm.h"
#include "memlib.h"

#ifdef DRIVER
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif

#define ALIGNMENT 8
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT - 1)) & ~0x7)
#define ALIGN_ODD(p) (((size_t)(p) & ~0x1) + 1)
#define MAX(x, y) ((x) > (y)? (x) : (y))
#define TAG_ALLOC(ptr, size) (((int *)(ptr))[0] = (size) ^ 0x4)
#define TAG_PREV_ALLOC_PTR(ptr) (((int *)(ptr))[0] &= ~1)
#define TAG_PREV_FREE_PTR(ptr) (((int *)(ptr))[0] |= 1)
#define TAG_PREV_ALLOC(ptr) \
	TAG_PREV_ALLOC_PTR((ptr) == mem_heap_hi() + 1 ? &hi_tag : (ptr))
#define TAG_PREV_FREE(ptr) \
	TAG_PREV_FREE_PTR((ptr) == mem_heap_hi() + 1 ? &hi_tag : (ptr))
#define TAG_FREE_8(ptr) (((long *)(ptr))[0] = 8LL << 32 | 8)
#define TAG_FREE(ptr, size) (((int *)(ptr))[0] = \
		((int *)((ptr) + (size)))[-1] = (size))
		
#define ALLOC_TAG(ptr) (((int *)(ptr))[0] & 0x4)
#define ALLOC_SIZE(ptr) (((int *)(ptr))[0] & ~0x7)
#define FREE_SIZE(ptr) (((int *)(ptr))[0] & ~0x7)
#define FREE_PREV(ptr) ((int *)(ptr))[1]
#define FREE_NEXT(ptr) ((int *)(ptr))[2]
#define PREV_FREE_TAG(ptr) (((int *)(ptr))[0] & 0x1)
#define PREV_FREE_SIZE(ptr) (((int *)(ptr))[-1] & ~0x7)
#define GET_NO(size) (27 - __builtin_clz(size))
#define LIST_LEN 22
#define BLOCKSIZE 4096

static void *heap_start = 0;
static int *link_start;
static int hi_tag, tag;
static void free_insert(void *ptr, int size);
static void *free_search(int size);
static void free_remove(void *ptr);
static void *extend_heap(int size);
static void coalesce(void *ptr, int size);
static void *binary2_bal(size_t size);

int mm_init(void)
{
    link_start = mem_sbrk(ALIGN_ODD(LIST_LEN) * 4);
    if (link_start == (void *)-1)return -1;
    for (int i = 0; i < LIST_LEN; i++)link_start[i] = 1;
    heap_start = mem_heap_hi() + 1;
    if (heap_start == NULL)return -1;
    hi_tag = 0; tag = 1;
    return 0;
}

void *malloc(size_t size)
{
    if (size == 0)return NULL;

    void *bal = binary2_bal(size); if (bal != NULL)return bal;
    size = ALIGN(size + 4);
    void *ptr = free_search((int)size);
    int remain;
    if (ptr)
    {
        free_remove(ptr);
        remain = FREE_SIZE(ptr) - size;
        free_insert(ptr + size, remain);
        TAG_ALLOC(ptr, size);
    }
    else
    {
        ptr = mem_heap_hi() + 1;
        if (ptr == NULL)return (void *)-1;
        if (hi_tag)
        {
            remain = PREV_FREE_SIZE(ptr);
            ptr -= remain;
            if (remain != 8)free_remove(ptr);
            hi_tag = 0;
            if (size - remain)
                if (extend_heap(size - remain) == (void *)-1)
                    return (void *)-1;
        }
        else if (extend_heap(size) == (void *)-1)return (void *)-1;
        TAG_ALLOC(ptr, size);
    }
    return ptr + 4;
}

void free(void *ptr)
{
    if (!ptr)return;
    if (heap_start == 0)mm_init();
    ptr -= 4;
    int size = ALLOC_SIZE(ptr);
    coalesce(ptr, size);
}

void *realloc(void *oldptr, size_t size)
{
    if (oldptr == NULL)return malloc(size);
    if (size == 0) { free(oldptr); return 0; }
    oldptr -= 4;
    int old_size = ALLOC_SIZE(oldptr);
    int prev_free = PREV_FREE_TAG(oldptr);
    size = ALIGN(size + 4);
    if ((int)size == old_size)return oldptr + 4;
    else if ((int)size < old_size)
    {
        TAG_ALLOC(oldptr, size);
        if (prev_free)TAG_PREV_FREE(oldptr);
        void *next = oldptr + old_size;
        int next_size = old_size - size;
        if (next <= mem_heap_hi() && !ALLOC_TAG(next))
        {
            if (FREE_SIZE(next) == 8)next_size += 8;
            else { free_remove(next); next_size += FREE_SIZE(next); }
        }
        free_insert(oldptr + size, next_size);
        return oldptr + 4;
    }
    else if (oldptr + old_size == mem_heap_hi() + 1)
    {
        if (extend_heap(size - old_size) == (void *)-1)return (void *)-1;
        TAG_ALLOC(oldptr, size);
        if (prev_free)TAG_PREV_FREE(oldptr);
        return oldptr + 4;
    }
    else if (!ALLOC_TAG(oldptr + old_size) &&
             FREE_SIZE(oldptr + old_size) + old_size >= (int)size)
    {
        if (FREE_SIZE(oldptr + old_size) != 8)free_remove(oldptr + old_size);
        old_size += FREE_SIZE(oldptr + old_size);
        TAG_ALLOC(oldptr, size);
        if (prev_free)TAG_PREV_FREE(oldptr);
        free_insert(oldptr + size, old_size - size);
        return oldptr + 4;
    }
    else
    {
        void *new_ptr = malloc(size - 4);
        if (new_ptr == (void *)-1)return (void *)-1;
        memcpy(new_ptr, oldptr + 4, old_size - 4);
        free(oldptr + 4);
        return new_ptr;
    }
}

void *calloc(size_t nmemb, size_t size)
{
    size_t bytes = nmemb * size;
    void *new_ptr;
    new_ptr = malloc(bytes);
    if (new_ptr == (void *)-1)return (void *)-1;
    memset(new_ptr, 0, bytes);
    return new_ptr;
}

static void free_insert(void *ptr, int size)
{
    if (!size) { TAG_PREV_ALLOC(ptr); return; }
    if (size == 8) { TAG_FREE_8(ptr); TAG_PREV_FREE(ptr + 8); return; }
    TAG_FREE(ptr, size);
    int *link = link_start + GET_NO(size);
    if (*link != 1)FREE_PREV(heap_start + *link) = ptr - heap_start;
    FREE_NEXT(ptr) = *link;
    FREE_PREV(ptr) = 1;
    *link = ptr - heap_start;
    TAG_PREV_FREE(ptr + size);
}

static void *free_search(int size)
{
    size = MAX(size, 16);
    int list_no = GET_NO(size);
    int *link;
    void *ptr;
    for (int i = list_no; i < LIST_LEN; i++)
    {
        link = link_start + i;
        if (*link == 1)continue;
        ptr = heap_start + *link;
        if (FREE_SIZE(ptr) >= size)return ptr;
        while (FREE_NEXT(ptr) != 1)
        {
            ptr = heap_start + FREE_NEXT(ptr);
            if (FREE_SIZE(ptr) >= size)return ptr;
        }
    }
    return NULL;
}

static void free_remove(void *ptr)
{
    int prev = FREE_PREV(ptr);
    int next = FREE_NEXT(ptr);
    if (prev == 1)
    {
        int *link = link_start + GET_NO(FREE_SIZE(ptr));
        *link = next;
        if (next != 1)FREE_PREV(heap_start + next) = 1;
    }
    else
    {
        FREE_NEXT(heap_start + prev) = next;
        if (next != 1)FREE_PREV(heap_start + next) = prev;
    }
}

static void *extend_heap(int size)
{
    if (size < BLOCKSIZE)
    {
        int remain = BLOCKSIZE - size;
        if (mem_sbrk(BLOCKSIZE) == (void *)-1)return (void *)-1;
        free_insert(mem_heap_hi() + 1 - remain, remain);
    }
    else if (mem_sbrk(size) == (void *)-1)return (void *)-1;
    return 0;
}

static void coalesce(void *ptr, int size)
{
    if (PREV_FREE_TAG(ptr))
    {
        int prev_size = PREV_FREE_SIZE(ptr);
        size += prev_size;
        ptr -= prev_size;
        if (prev_size != 8)free_remove(ptr);
    }
    void *next = ptr + size;
    if (next <= mem_heap_hi() && !ALLOC_TAG(next))
    {
        if (FREE_SIZE(next) == 8)size += 8;
        else { free_remove(next); size += FREE_SIZE(next); }
    }
    free_insert(ptr, size);
}

static void *binary2_bal(size_t size)
{
    if (tag == 0)return NULL;
    if (tag == 1)
    {
        if (size == 64)
        {
            int extension = 800000; tag = 2;
            if (extend_heap(extension) == (void *)-1)return (void *)-1;
            free_insert(mem_heap_hi() + 1 - extension, extension);
        }
        else { tag = 0; return NULL; }
    }
    size = ALIGN(size + 4);
    void *ptr = free_search((int)size);
    int remain;
    if (ptr)
    {
        free_remove(ptr);
        remain = FREE_SIZE(ptr) - size;
        if (tag == 2)tag = 3;
        else if (tag == 3)tag = 2;
        if (tag == 3)
        {
            TAG_PREV_ALLOC(ptr + FREE_SIZE(ptr));
            if (remain)free_insert(ptr, remain);
            TAG_ALLOC(ptr + remain, size);
            return ptr + remain + 4;
        }
        free_insert(ptr + size, remain);
        TAG_ALLOC(ptr, size);
    }
    else
    {
        ptr = mem_heap_hi() + 1;
        if (ptr == NULL)return (void *)-1;
        if (hi_tag)
        {
            remain = PREV_FREE_SIZE(ptr);
            ptr -= remain;
            if (remain != 8)free_remove(ptr);
            hi_tag = 0;
            if (size - remain)
                if (extend_heap(size - remain) == (void *)-1)
                    return (void *)-1;
        }
        else if (extend_heap(size) == (void *)-1)return (void *)-1;
        TAG_ALLOC(ptr, size);
    }
    return ptr + 4;
}

void mm_checkheap(int lineno)
{

    void *ptr = heap_start;
    while (ptr <= mem_heap_hi())
    {
        if ((long)(ptr + 4) % 8)
        {
            fprintf(stderr, "%d: block address not aligned\n", lineno);
            exit(1);
        }
        if (ALLOC_TAG(ptr))ptr += ALLOC_SIZE(ptr);
        else ptr += FREE_SIZE(ptr);
    }

    if (ptr != mem_heap_hi() + 1)
    {
        fprintf(stderr, "%d: ptr did not reach heap boundary\n", lineno);
        exit(1);
    }

    ptr = heap_start;
    int prev_state = 0;
    while (ptr <= mem_heap_hi())
    {
        if (ALLOC_TAG(ptr))
        {
            if (ALLOC_SIZE(ptr) < 8)
            {
                fprintf(stderr, "%d: below minimum allocated size\n", lineno);
                exit(1);
            }
            if (ALLOC_SIZE(ptr) % 8)
            {
                fprintf(stderr, "%d: allocated size not aligned\n", lineno);
                exit(1);
            }
            if (ptr != heap_start && prev_state && !PREV_FREE_TAG(ptr))
            {
                fprintf(stderr, "%d: inconsistent free bit\n", lineno);
                exit(1);
            }
            if (ptr != heap_start && !prev_state && PREV_FREE_TAG(ptr))
            {
                fprintf(stderr, "%d: inconsistent allocate bit\n", lineno);
                exit(1);
            }
            prev_state = 0;
            ptr += ALLOC_SIZE(ptr);
        }
        else
        {
            if (FREE_SIZE(ptr) < 8)
            {
                fprintf(stderr, "%d: below minimum free size\n", lineno);
                exit(1);
            }
            if (FREE_SIZE(ptr) % 8)
            {
                fprintf(stderr, "%d: free size not aligned\n", lineno);
                exit(1);
            }
            if (ptr != heap_start && prev_state && !PREV_FREE_TAG(ptr))
            {
                fprintf(stderr, "%d: inconsistent free bit\n", lineno);
                exit(1);
            }
            if (ptr != heap_start && !prev_state && PREV_FREE_TAG(ptr))
            {
                fprintf(stderr, "%d: inconsistent allocate bit\n", lineno);
                exit(1);
            }
            if (FREE_SIZE(ptr) != FREE_SIZE(ptr + FREE_SIZE(ptr) - 4))
            {
                fprintf(stderr, "%d: header and footer not match\n", lineno);
                exit(1);
            }
            prev_state = 1;
            ptr += FREE_SIZE(ptr);
        }
    }

    ptr = heap_start;
    prev_state = 0;
    while (ptr <= mem_heap_hi())
    {
        if (ALLOC_TAG(ptr))
        {
            prev_state = 0;
            ptr += ALLOC_SIZE(ptr);
        }
        else
        {
            if (prev_state)
            {
                fprintf(stderr, "%d: consecutive free blocks\n", lineno);
                exit(1);
            }
            prev_state = 1;
            ptr += FREE_SIZE(ptr);
        }
    }

    int *link;
    for (int i = 0; i < LIST_LEN; i++)
    {
        link = link_start + i;
        if (*link == 1)continue;
        ptr = heap_start + *link;
        while (FREE_NEXT(ptr) != 1)
        {
            if (ptr != heap_start + FREE_PREV(heap_start + FREE_NEXT(ptr)))
            {
                fprintf(stderr, "%d: inconsistent pointers\n", lineno);
                exit(1);
            }
            ptr = heap_start + FREE_NEXT(ptr);
        }
    }

    for (int i = 0; i < LIST_LEN; i++)
    {
        link = link_start + i;
        if (*link == 1)continue;
        ptr = heap_start + *link;
        while (FREE_NEXT(ptr) != 1)
        {
            if (ptr < mem_heap_lo())
            {
                fprintf(stderr, "%d: Pointer before mem_heap_lo\n", lineno);
                exit(1);
            }
            if (ptr > mem_heap_hi())
            {
                fprintf(stderr, "%d: Pointer after mem_heap_hi\n", lineno);
                exit(1);
            }
            ptr = heap_start + FREE_NEXT(ptr);
        }
    }

    int iterate = 0;
    int traverse = 0;
    ptr = heap_start;
    while (ptr <= mem_heap_hi())
    {
        if (ALLOC_TAG(ptr))ptr += ALLOC_SIZE(ptr);
        else { if (FREE_SIZE(ptr) > 8)iterate++; ptr += FREE_SIZE(ptr); }
    }
    for (int i = 0; i < LIST_LEN; i++)
    {
        link = link_start + i;
        if (*link == 1)continue;
        ptr = heap_start + *link;
        traverse++;
        while (FREE_NEXT(ptr) != 1)
        {
            traverse++;
            ptr = heap_start + FREE_NEXT(ptr);
        }
    }
    if (iterate != traverse)
    {
        fprintf(stderr, "%d: Free block count did not match! \n", lineno);
        exit(1);
    }

    int list_no;
    for (int i = 0; i < LIST_LEN; i++)
    {
        link = link_start + i;
        if (*link == 1)continue;
        ptr = heap_start + *link;
        list_no = GET_NO(FREE_SIZE(ptr));
        if (list_no != i)
        {
            fprintf(stderr, "%d: Free block in wrong list! \n", lineno);
            exit(1);
        }
        while (FREE_NEXT(ptr) != 1)
        {
            ptr = heap_start + FREE_NEXT(ptr);
            list_no = GET_NO(FREE_SIZE(ptr));
            if (list_no != i)
            {
                fprintf(stderr, "%d: Free block in wrong list! \n", lineno);
                exit(1);
            }
        }
    }
}
