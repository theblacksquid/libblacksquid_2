
/** ARENA START  **/

// from https://github.com/tsoding/arena

// Copyright 2022 Alexey Kutepov <reximkut@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:

// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef ARENA_H_
#define ARENA_H_

#include <stddef.h>
#include <stdint.h>

#ifndef ARENA_ASSERT
#include <assert.h>
#define ARENA_ASSERT assert
#endif

#define ARENA_BACKEND_LIBC_MALLOC 0
#define ARENA_BACKEND_LINUX_MMAP 1
#define ARENA_BACKEND_WIN32_VIRTUALALLOC 2
#define ARENA_BACKEND_WASM_HEAPBASE 3

#ifndef ARENA_BACKEND
#define ARENA_BACKEND ARENA_BACKEND_LIBC_MALLOC
#endif // ARENA_BACKEND

typedef struct Region Region;

struct Region {
    Region *next;
    size_t count;
    size_t capacity;
    uintptr_t data[];
};

typedef struct {
    Region *begin, *end;
} Arena;

#define REGION_DEFAULT_CAPACITY (8*1024)

Region *new_region(size_t capacity);
void free_region(Region *r);

// TODO: snapshot/rewind capability for the arena
// - Snapshot should be combination of a->end and a->end->count.
// - Rewinding should be restoring a->end and a->end->count from the snapshot and
// setting count-s of all the Region-s after the remembered a->end to 0.
void *arena_alloc(Arena *a, size_t size_bytes);
void *arena_realloc(Arena *a, void *oldptr, size_t oldsz, size_t newsz);

void arena_reset(Arena *a);
void arena_free(Arena *a);

#endif // ARENA_H_

#ifdef ARENA_IMPLEMENTATION

#if ARENA_BACKEND == ARENA_BACKEND_LIBC_MALLOC
#include <stdlib.h>

// TODO: instead of accepting specific capacity new_region() should accept the size of the object we want to fit into the region
// It should be up to new_region() to decide the actual capacity to allocate
Region *new_region(size_t capacity)
{
    size_t size_bytes = sizeof(Region) + sizeof(uintptr_t)*capacity;
    // TODO: it would be nice if we could guarantee that the regions are allocated by ARENA_BACKEND_LIBC_MALLOC are page aligned
    Region *r = malloc(size_bytes);
    ARENA_ASSERT(r);
    r->next = NULL;
    r->count = 0;
    r->capacity = capacity;
    return r;
}

void free_region(Region *r)
{
    free(r);
}
#elif ARENA_BACKEND == ARENA_BACKEND_LINUX_MMAP
#include <unistd.h>
#include <sys/mman.h>

Region *new_region(size_t capacity)
{
    size_t size_bytes = sizeof(Region) + sizeof(uintptr_t) * capacity;
    Region *r = mmap(NULL, size_bytes, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    ARENA_ASSERT(r != MAP_FAILED);
    r->next = NULL;
    r->count = 0;
    r->capacity = capacity;
    return r;
}

void free_region(Region *r)
{
    size_t size_bytes = sizeof(Region) + sizeof(uintptr_t) * r->capacity;
    int ret = munmap(r, size_bytes);
    ARENA_ASSERT(ret == 0);
}

#elif ARENA_BACKEND == ARENA_BACKEND_WIN32_VIRTUALALLOC

#if !defined(_WIN32)
#  error "Current platform is not Windows"
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define INV_HANDLE(x)       (((x) == NULL) || ((x) == INVALID_HANDLE_VALUE))

Region *new_region(size_t capacity)
{
    SIZE_T size_bytes = sizeof(Region) + sizeof(uintptr_t) * capacity;
    Region *r = VirtualAllocEx(
        GetCurrentProcess(),      /* Allocate in current process address space */
        NULL,                     /* Unknown position */
        size_bytes,               /* Bytes to allocate */
        MEM_COMMIT | MEM_RESERVE, /* Reserve and commit allocated page */
        PAGE_READWRITE            /* Permissions ( Read/Write )*/
    );
    if (INV_HANDLE(r))
        ARENA_ASSERT(0 && "VirtualAllocEx() failed.");

    r->next = NULL;
    r->count = 0;
    r->capacity = capacity;
    return r;
}

void free_region(Region *r)
{
    if (INV_HANDLE(r))
        return;

    BOOL free_result = VirtualFreeEx(
        GetCurrentProcess(),        /* Deallocate from current process address space */
        (LPVOID)r,                  /* Address to deallocate */
        0,                          /* Bytes to deallocate ( Unknown, deallocate entire page ) */
        MEM_RELEASE                 /* Release the page ( And implicitly decommit it ) */
    );

    if (FALSE == free_result)
        ARENA_ASSERT(0 && "VirtualFreeEx() failed.");
}

#elif ARENA_BACKEND == ARENA_BACKEND_WASM_HEAPBASE
#  error "TODO: WASM __heap_base backend is not implemented yet"
#else
#  error "Unknown Arena backend"
#endif

// TODO: add debug statistic collection mode for arena
// Should collect things like:
// - How many times new_region was called
// - How many times existing region was skipped
// - How many times allocation exceeded REGION_DEFAULT_CAPACITY

void *arena_alloc(Arena *a, size_t size_bytes)
{
    size_t size = (size_bytes + sizeof(uintptr_t) - 1)/sizeof(uintptr_t);

    if (a->end == NULL) {
        ARENA_ASSERT(a->begin == NULL);
        size_t capacity = REGION_DEFAULT_CAPACITY;
        if (capacity < size) capacity = size;
        a->end = new_region(capacity);
        a->begin = a->end;
    }

    while (a->end->count + size > a->end->capacity && a->end->next != NULL) {
        a->end = a->end->next;
    }

    if (a->end->count + size > a->end->capacity) {
        ARENA_ASSERT(a->end->next == NULL);
        size_t capacity = REGION_DEFAULT_CAPACITY;
        if (capacity < size) capacity = size;
        a->end->next = new_region(capacity);
        a->end = a->end->next;
    }

    void *result = &a->end->data[a->end->count];
    a->end->count += size;
    return result;
}

void *arena_realloc(Arena *a, void *oldptr, size_t oldsz, size_t newsz)
{
    if (newsz <= oldsz) return oldptr;
    void *newptr = arena_alloc(a, newsz);
    char *newptr_char = newptr;
    char *oldptr_char = oldptr;
    for (size_t i = 0; i < oldsz; ++i) {
        newptr_char[i] = oldptr_char[i];
    }
    return newptr;
}

void arena_reset(Arena *a)
{
    for (Region *r = a->begin; r != NULL; r = r->next) {
        r->count = 0;
    }

    a->end = a->begin;
}

void arena_free(Arena *a)
{
    Region *r = a->begin;
    while (r) {
        Region *r0 = r;
        r = r->next;
        free_region(r0);
    }
    a->begin = NULL;
    a->end = NULL;
}

#endif // ARENA_IMPLEMENTATION

/** ARENA END  **/

#ifndef LIBBLACKSQUID_H

typedef enum ltbs_type ltbs_type;
typedef struct ltbs_cell ltbs_cell;
typedef struct ltbs_string ltbs_string;
typedef struct ltbs_array ltbs_array;
typedef struct ltbs_pair ltbs_pair;
// based on https://nullprogram.com/blog/2023/09/30/
typedef struct ltbs_hashmap ltbs_hashmap;
typedef unsigned char byte;

struct ltbs_cell
{
    enum ltbs_type
    {
	LTBS_INT,
	LTBS_FLOAT,
	LTBS_STRING,
	LTBS_ARRAY,
	LTBS_PAIR,
	LTBS_HASHMAP
    } type;

    union
    {
	int integer;
	float floatval;
	byte byteval;
	
	struct ltbs_pair
	{
	    ltbs_cell *data;
	    ltbs_cell *prev;
	} pair;
	
	struct ltbs_string
	{
	    byte *strdata;
	    unsigned int length;
	} string;
	
	struct ltbs_array
	{
	    ltbs_cell *arrdata;
	    unsigned int length;
	} array;

	struct ltbs_hashmap
	{
	    ltbs_hashmap *children[4];
	    ltbs_string *key;
	    ltbs_cell *value;
	} hashmap;
    } data;
};

ltbs_cell *pair_head(ltbs_cell *pair);
ltbs_cell *pair_rest(ltbs_cell *pair);
ltbs_cell *pair_cons(ltbs_cell *value, ltbs_cell *list, Arena *context);
unsigned int pair_is_atom(ltbs_cell *value);
unsigned int pair_count(ltbs_cell *list);
ltbs_cell *pair_by_index(ltbs_cell *list, unsigned int index);
ltbs_cell *pair_reverse(ltbs_cell *list, Arena *context);
ltbs_cell *pair_append(ltbs_cell *list1, ltbs_cell *list2, Arena *context);
ltbs_cell *pair_sort(ltbs_cell *list, int (*pred)(ltbs_cell *prev, ltbs_cell *next), Arena *context);
unsigned int pair_length(ltbs_cell *list);
ltbs_cell *pair_take(ltbs_cell *list, unsigned int to_take, Arena* context);


#ifdef LIBBLACKSQUID_IMPLEMENTATION
#define ARENA_IMPLEMENTATION

ltbs_cell *pair_head(ltbs_cell *list)
{
    return list->data.pair.head;
}

ltbs_cell *pair_rest(ltbs_cell *list)
{
    return list->data.pair.rest;
}

unsigned int pair_is_atom(ltbs_cell *value)
{
    if ( (value->type != PAIR) || (value->type != ARRA) || (value->type != HASH) )
	return 0;
    else
	return 1;
}

unsigned int pair_count(ltbs_cell *list)
{
    unsigned int result = 0;

    for ( ltbs_cell *tracker = list; tracker != 0; tracker = pair_rest(tracker) )
    {
	result++;
    }

    return result;
}

ltbs_cell *pair_by_index(ltbs_cell *list, unsigned int index)
{
    ltbs_cell *result = 0;
    ltbs_cell *current = list;

    for (int index_tracker = index; index_tracker >= 0; index_tracker--)
    {
	if ( index_tracker == 0 )
	{
	    result = current;
	    break;
	}

	current = pair_rest(current);
    }

    return result;
}

ltbs_cell *pair_cons_in_context(ltbs_cell *value, ltbs_cell *rest, Arena *context)
{
    ltbs_cell *new_cell = arena_alloc(context, sizeof(ltbs_cell));
    new_cell->data.pair.head = value;
    new_cell->data.pair.rest = rest;
    new_cell->type = PAIR;

    return new_cell;
}

ltbs_cell* pair_append(ltbs_cell* list1, ltbs_cell* list2, Arena* context)
{
    Arena workspace        = {0};
    ltbs_cell* result      = arena_alloc(context, sizeof(ltbs_cell));
    result->type           = PAIR;
    result->data.pair.head = 0;
    result->data.pair.rest = 0;

    {
	for (ltbs_cell* tracker = pair_reverse(list1, &workspace);
	     tracker->data.pair.head != 0;
	     tracker = pair_rest(tracker))
	{
	    result = pair_cons_in_context(pair_head(tracker), result, context);
	}
    }

    {
	for (ltbs_cell* tracker = pair_reverse(list2, &workspace);
	     tracker->data.pair.head != 0;
	     tracker = pair_rest(tracker))
	{
	    result = pair_cons_in_context(pair_head(tracker), result, context);
	}
    }

    arena_free(&workspace);
    return result;
}   

unsigned int pair_length(ltbs_cell* list)
{
    unsigned int result = 0;

    for (ltbs_cell* tracker = list;
	 tracker->data.pair.head != 0;
	 tracker = pair_rest(tracker))
    {
	result++;
    }

    return result;
}

#endif // LIBBLACKSQUID_IMPLEMENTATION
#endif // LIBBLACKSQUID_H
