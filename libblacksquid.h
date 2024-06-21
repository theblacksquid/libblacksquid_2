// libblacksquid.h - v1.0 - MIT License, theblacksquid 2024
//
// Provides operations around data structures based on an Arena
// Allocator, wrapped around a tagged union
// 
// This file provides both the interface and the implementation.
// To instantiate the implementation,
//      #define STB_C_LEXER_IMPLEMENTATION
// in *ONE* source file, before #including this file.

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
#define LIBBLACKSQUID_H

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
	LTBS_BYTE,
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
	    ltbs_cell *head;
	    ltbs_cell *rest;
	} pair;
	
	struct ltbs_string
	{
	    byte *strdata;
	    unsigned int length;
	} string;
	
	struct ltbs_array
	{
	    ltbs_cell **arrdata;
	    unsigned int length;
	} array;

	struct ltbs_hashmap
	{
	    ltbs_cell *children[4];
	    ltbs_cell *key;
	    ltbs_cell *value;
	} hashmap;
    } data;
};

#define HASH_FACTOR 1111111111111111111u

static ltbs_cell *pair_head(ltbs_cell *pair);
static ltbs_cell *pair_rest(ltbs_cell *pair);
static ltbs_cell *pair_cons(ltbs_cell *value, ltbs_cell *list, Arena *context);
static unsigned int pair_is_atom(ltbs_cell *value);
static unsigned int pair_count(ltbs_cell *list);
static ltbs_cell *pair_by_index(ltbs_cell *list, unsigned int index);
static ltbs_cell *pair_reverse(ltbs_cell *list, Arena *context);
static ltbs_cell *pair_append(ltbs_cell *list1, ltbs_cell *list2, Arena *context);
static unsigned int pair_length(ltbs_cell *list);
static ltbs_cell *pair_take(ltbs_cell *list, unsigned int to_take, Arena* context);
static ltbs_cell *pair_min(ltbs_cell *list, int (*compare)(ltbs_cell*, ltbs_cell*));
static ltbs_cell *pair_sort(ltbs_cell *list, int (*compare)(ltbs_cell*, ltbs_cell*), Arena *context);
static ltbs_cell *pair_copy(ltbs_cell *list, Arena *destination);
static ltbs_cell *pair_min_and_remove(ltbs_cell *list, int (*compare)(ltbs_cell*, ltbs_cell*));
static ltbs_cell *pair_filter(ltbs_cell *list, int (*pred)(ltbs_cell*), Arena *context);

static ltbs_cell *string_from_cstring(const char *cstring, Arena *context);
static ltbs_cell *string_substring(ltbs_cell *string, unsigned int start, unsigned int end, Arena *context);
static int string_compare(ltbs_cell *string1, ltbs_cell *string2);
static ltbs_cell *string_append(ltbs_cell *string1, ltbs_cell *string2, Arena *context);
static ltbs_cell *string_to_list(ltbs_cell *string, Arena *context);
static ltbs_cell *string_reverse(ltbs_cell *string, Arena *context);
static ltbs_cell *string_copy(ltbs_cell *string, Arena *destination);
static void string_print(ltbs_cell *string);

static ltbs_cell *array_to_list(ltbs_cell *array, Arena *context);
static ltbs_cell *array_ref(ltbs_cell *array, unsigned int index);
static ltbs_cell *pair_to_array(ltbs_cell *list, Arena *context);
static ltbs_cell *array_reverse(ltbs_cell *array, Arena *context);
static ltbs_cell *array_slice(ltbs_cell *array, unsigned int start, unsigned int length, Arena *context);
static ltbs_cell *array_append(ltbs_cell *array1, ltbs_cell *array2, Arena *context);
static ltbs_cell *array_copy(ltbs_cell *array, Arena *destination);

static ltbs_cell *hash_make(Arena *context);
static ltbs_cell *hash_upsert(ltbs_cell **map, ltbs_cell *key, ltbs_cell *value, Arena *context);
static int hash_compute(ltbs_string *key);

#ifdef LIBBLACKSQUID_IMPLEMENTATION
#define ARENA_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>

static ltbs_cell *ltbs_alloc(Arena *context)
{
    return arena_alloc(context, sizeof(ltbs_cell));
}

static ltbs_cell *pair_head(ltbs_cell *list)
{
    return list->data.pair.head;
}

static ltbs_cell *pair_rest(ltbs_cell *list)
{
    return list->data.pair.rest;
}

static ltbs_cell *pair_cons(ltbs_cell *value, ltbs_cell *rest, Arena *context)
{
    ltbs_cell *new_cell = arena_alloc(context, sizeof(ltbs_cell));
    new_cell->data.pair.head = value;
    new_cell->data.pair.rest = rest;
    new_cell->type = LTBS_PAIR;

    return new_cell;
}

static unsigned int pair_is_atom(ltbs_cell *value)
{
    if ( (value->type != LTBS_PAIR) ||
	 (value->type != LTBS_ARRAY) ||
	 (value->type != LTBS_HASHMAP) )
	return 0;
    else
	return 1;
}

static unsigned int pair_count(ltbs_cell *list)
{
    unsigned int result = 0;

    for ( ltbs_cell *tracker = list; tracker != 0; tracker = pair_rest(tracker) )
    {
	result++;
    }

    return result;
}

static ltbs_cell *pair_by_index(ltbs_cell *list, unsigned int index)
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

static ltbs_cell *pair_append(ltbs_cell* list1, ltbs_cell* list2, Arena* context)
{
    Arena workspace        = {0};
    ltbs_cell* result      = arena_alloc(context, sizeof(ltbs_cell));
    result->type           = LTBS_PAIR;
    result->data.pair.head = 0;
    result->data.pair.rest = 0;

    {
	for (ltbs_cell* tracker = pair_reverse(list1, &workspace);
	     tracker->data.pair.head != 0;
	     tracker = pair_rest(tracker))
	{
	    result = pair_cons(pair_head(tracker), result, context);
	}
    }

    {
	for (ltbs_cell* tracker = pair_reverse(list2, &workspace);
	     tracker->data.pair.head != 0;
	     tracker = pair_rest(tracker))
	{
	    result = pair_cons(pair_head(tracker), result, context);
	}
    }

    arena_free(&workspace);
    return result;
}   

static unsigned int pair_length(ltbs_cell* list)
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

static ltbs_cell* pair_reverse(ltbs_cell* list, Arena* context)
{
    ltbs_cell* result = arena_alloc(context, sizeof(ltbs_cell));
    result->type = LTBS_PAIR;
    result->data.pair.head = 0;
    result->data.pair.rest = 0;

    for (ltbs_cell* tracker = list;
	 tracker->data.pair.head != 0;
	 tracker = pair_rest(tracker))
    {
	result = pair_cons(pair_head(tracker), result, context);
    }

    return result;
}

static ltbs_cell *pair_min(ltbs_cell *list, int (*compare)(ltbs_cell*, ltbs_cell*))
{
    int length = pair_length(list);

    if ( length == 1 )
	return pair_head(list);

    else
    {
	ltbs_cell *current_min = pair_head(list);

	for (ltbs_cell *tracker = pair_rest(list);
	     tracker->data.pair.head != 0;
	     tracker = pair_rest(tracker))
	{
	    if ( compare(current_min, pair_head(tracker)) > 0 )
	    {
		current_min = pair_head(tracker);
	    }
	}

	return current_min;
    }
}

static ltbs_cell *pair_min_and_remove(ltbs_cell *list, int (*compare)(ltbs_cell*, ltbs_cell*))
{
    int length = pair_length(list);

    if ( length == 1 )
	return pair_head(list);

    else
    {
	ltbs_cell *current_min = pair_head(list);
	ltbs_cell *before_min = list;
	ltbs_cell *follow = list;

	for (ltbs_cell *tracker = pair_rest(list);
	     tracker->data.pair.head != 0;
	     tracker = pair_rest(tracker))
	{   
	    if ( compare(current_min, pair_head(tracker)) > 0 )
	    {
		before_min = follow;
		current_min = pair_head(tracker);
	    }

	    follow = tracker;
	}
	
	if ( current_min != pair_head(list) )
	{
	    before_min->data.pair.rest = pair_rest(pair_rest(before_min));
	}

	return current_min;
    }
}

static ltbs_cell *pair_copy(ltbs_cell *list, Arena *destination)
{
    Arena workspace = {0};

    ltbs_cell *current_list = arena_alloc(&workspace, sizeof(ltbs_cell));
    current_list->type = LTBS_PAIR;
    current_list->data.pair.head = 0;
    current_list->data.pair.rest = 0;
    
    for (ltbs_cell *tracker = list;
	 tracker->data.pair.head != 0;
	 tracker = pair_rest(tracker))
    {
	current_list = pair_cons(pair_head(tracker), current_list, &workspace);
    }
    
    arena_free(&workspace);

    return pair_reverse(current_list, destination);
}

static ltbs_cell *pair_sort(ltbs_cell *list, int (*compare)(ltbs_cell*, ltbs_cell*), Arena *context)
{
    Arena workspace = {0};

    ltbs_cell *result = arena_alloc(&workspace, sizeof(ltbs_cell));
    ltbs_cell *list_copy = pair_copy(list, &workspace);
    int length = pair_length(list_copy);

    result->type = LTBS_PAIR;
    result->data.pair.head = 0;
    result->data.pair.rest = 0;

    while ( length > 1 )
    {	
	ltbs_cell *min = pair_min_and_remove(list_copy, compare);
	if ( min == pair_head(list_copy) )
	    list_copy = pair_rest(list_copy);
	
        result = pair_cons(min, result, &workspace);
	length = pair_length(list_copy);
    }

    if ( pair_head(list_copy) != 0 )
	result = pair_cons(pair_head(list_copy), result, &workspace);
    
    arena_free(&workspace);

    return pair_reverse(result, context);
}

static ltbs_cell *pair_filter(ltbs_cell *list, int (*pred)(ltbs_cell*), Arena *context)
{
    ltbs_cell *result = arena_alloc(context, sizeof(ltbs_cell));
    result->type = LTBS_PAIR;
    result->data.pair.head = 0;
    result->data.pair.rest = 0;

    for (ltbs_cell *tracker = list;
	 tracker->data.pair.head != 0;
	 tracker = pair_rest(tracker))
    {
	if ( pred(pair_head(tracker)) )
	    result = pair_cons(pair_head(tracker), result, context);
    }

    return result;
}

static ltbs_cell *string_from_cstring(const char *cstring, Arena *context)
{
    ltbs_cell *result = arena_alloc(context, sizeof(ltbs_cell));
    int length = 0;
    result->type = LTBS_STRING;
    result->data.string.strdata = (unsigned char *)cstring;

    for ( byte ch = cstring[0]; ch != 0; ch = cstring[++length] ){}

    result->data.string.length = length;
    return result;
}

static ltbs_cell *string_substring(ltbs_cell *string, unsigned int start, unsigned int end, Arena *context)
{
    if ( (string->data.string.length < start) ||
	 (string->data.string.length < end) )
    {
	exit(1);
	return 0;	
    }

    else
    {
	ltbs_cell *result = arena_alloc(context, sizeof(ltbs_cell));
	result->data.string.strdata = &string->data.string.strdata[start];
	result->data.string.length = end - start;

	return result;
    }
}

static int string_compare(ltbs_cell *string1, ltbs_cell *string2)
{
    if ( (string1 != 0) &&
	 (string2 != 0) &&
	 (string1->data.string.length != string2->data.string.length) )
	return 0;

    else
    {
	int length = string1->data.string.length;
	byte *buffer1 = string1->data.string.strdata;
	byte *buffer2 = string2->data.string.strdata;
	for (int index = 0; index < length; index++)
	    if ( buffer1[index] != buffer2[index] ) return 0;

	return 1;
    }
}

static ltbs_cell *string_append(ltbs_cell *string1, ltbs_cell *string2, Arena *context)
{
    ltbs_cell *result = arena_alloc(context, sizeof(ltbs_cell));
    int length = string1->data.string.length + string2->data.string.length;
    byte *bytearray = arena_alloc(context, length + 1);
    
    result->type = LTBS_STRING;
    result->data.string.length = length;
    result->data.string.strdata = bytearray;
    bytearray[length + 1] = '\0';

    for (int index = 0; index < length; index++)
    {
	int length1 = string1->data.string.length;
	if ( index < length1 )
	    bytearray[index] = string1->data.string.strdata[index];
	else
	    bytearray[index] = string2->data.string.strdata[index - length1];
    }

    return result;
}

static void string_print(ltbs_cell *string)
{
    for (unsigned int index = 0; index < string->data.string.length; index++)
	putc(string->data.string.strdata[index], stdout);
}

static ltbs_cell *string_to_list(ltbs_cell *string, Arena *context)
{
    ltbs_cell *result = arena_alloc(context, sizeof(ltbs_cell));
    unsigned int length = string->data.string.length;

    result->type = LTBS_PAIR;
    result->data.pair.head = 0;
    result->data.pair.rest = 0;

    for (unsigned int index = 0; index < length; index++)
    {
	ltbs_cell *to_add = arena_alloc(context, 1);
	to_add->type = LTBS_BYTE;
	to_add->data.byteval = string->data.string.strdata[index];
	result = pair_cons(to_add, result, context);
    }

    return result;
}

static ltbs_cell *string_reverse(ltbs_cell *string, Arena *context)
{
    ltbs_cell *result = arena_alloc(context, sizeof(ltbs_cell));
    int length = string->data.string.length;
    byte *buffer = arena_alloc(context, length + 1);
    int inner_index = 0;

    result->type = LTBS_STRING;
    result->data.string.length = length;
    result->data.string.strdata = buffer;
    buffer[length + 1] = '\0';

    for (int index = length - 1; index > -1; index--)
    {
	buffer[inner_index] = string->data.string.strdata[index];
	inner_index++;
    }

    return result;
}

static ltbs_cell *string_copy(ltbs_cell *string, Arena *destination)
{
    ltbs_cell *result = arena_alloc(destination, sizeof(ltbs_cell));
    unsigned int length = string->data.string.length;
    byte *buffer = arena_alloc(destination, length + 1);

    result->type = LTBS_STRING;
    result->data.string.strdata = buffer;
    result->data.string.length = length;
    buffer[length + 1] = '\0';

    for (unsigned int index = 0; index < length; index++)
	buffer[index] = string->data.string.strdata[index];

    return result;
}


static ltbs_cell *array_to_list(ltbs_cell *array, Arena *context)
{
    ltbs_cell *result = ltbs_alloc(context);
    int length = array->data.array.length;
    result->type = LTBS_PAIR;
    result->data.pair.head = 0;
    result->data.pair.rest = 0;

    for (int index = 0; index < length; index++)
    {
	ltbs_cell *to_add = array->data.array.arrdata[index];
	result = pair_cons(to_add, result, context);
    }

    return result;
}

static ltbs_cell *array_ref(ltbs_cell *array, unsigned int index)
{
    return array->data.array.arrdata[index];
}

static ltbs_cell *pair_to_array(ltbs_cell *list, Arena *context)
{
    ltbs_cell *result = ltbs_alloc(context);
    int length = pair_length(list);
    ltbs_cell **buffer = arena_alloc(context, sizeof(ltbs_cell*) * length);
    ltbs_cell *tracker = list;

    result->type = LTBS_ARRAY;
    result->data.array.length = length;
    result->data.array.arrdata = buffer;

    for (int index = 0; index < length; index++)
    {
	buffer[index] = pair_head(tracker);
	tracker = pair_rest(tracker);
    }

    return result;
}

static ltbs_cell *array_reverse(ltbs_cell *array, Arena *context)
{
    ltbs_cell *result = ltbs_alloc(context);
    int length = array->data.array.length;
    ltbs_cell **buffer = arena_alloc(context, sizeof(ltbs_cell*) * length);
    int inner_index = length;

    result->type = LTBS_ARRAY;
    result->data.array.arrdata = buffer;
    result->data.array.length = length;
    
    for (int index = 0; index < length; index++)
	buffer[index] = array->data.array.arrdata[inner_index--];

    return result;
}

static ltbs_cell *array_slice(ltbs_cell *array, unsigned int start, unsigned int length, Arena *context)
{
    ltbs_cell *result = ltbs_alloc(context);
    result->type = LTBS_ARRAY;
    result->data.array.length = length;
    result->data.array.arrdata = &array->data.array.arrdata[start];

    return result;
}

static ltbs_cell *array_append(ltbs_cell *array1, ltbs_cell *array2, Arena *context)
{
    ltbs_cell *result = ltbs_alloc(context);
    int length1 = array1->data.array.length;
    int length2 = array2->data.array.length;
    int length_total = length1 + length2;
    ltbs_cell **buffer = arena_alloc(context, sizeof(ltbs_cell*) * length_total);
    
    result->type = LTBS_ARRAY;
    result->data.array.length = length_total;
    result->data.array.arrdata = buffer;

    for (int index = 0; index < length_total; index++)
    {
	if ( index < length1 )
	    buffer[index] = array1->data.array.arrdata[index];
	else
	    buffer[index + length1] = array2->data.array.arrdata[index - length1];
    }

    return result;
}

static ltbs_cell *array_copy(ltbs_cell *array, Arena *destination)
{
    int length = array->data.array.length;
    ltbs_cell *result = ltbs_alloc(destination);
    ltbs_cell **buffer = arena_alloc(destination, sizeof(ltbs_cell*) * length);

    result->type = LTBS_ARRAY;
    result->data.array.arrdata = buffer;
    result->data.array.length = length;

    for ( int index = 0; index < length; index++ )
	buffer[index] = array->data.array.arrdata[index];

    return result;
}

static ltbs_cell *hash_make(Arena *context)
{
    ltbs_cell *result = ltbs_alloc(context);
    result->type = LTBS_HASHMAP;
    result->data.hashmap.children[0] = 0;
    result->data.hashmap.children[1] = 0;
    result->data.hashmap.children[2] = 0;
    result->data.hashmap.children[3] = 0;
    result->data.hashmap.value = 0;

    return result;
}

static int hash_compute(ltbs_string *key)
{
    int result = 0x100;

    for (unsigned int index = 0; index < key->length; index++)
    {
	result ^= key->strdata[index];
	result *= HASH_FACTOR;
    }

    return result;
}

static ltbs_cell *hash_upsert(ltbs_cell **map, ltbs_cell *key, ltbs_cell *value, Arena *context)
{
    for (unsigned int hash = hash_compute(&key->data.string); *map; hash <<= 2)
    {	
	if ( ((*map)->data.hashmap.key != 0) &&
	     string_compare(key, (*map)->data.hashmap.key) )
	{
	    return (*map)->data.hashmap.value;
	}

	map = &(*map)->data.hashmap.children[hash >> 30];
    }

    if ( (context != 0) && (value != 0) )
    {
	*map = ltbs_alloc(context);
	(*map)->type = LTBS_HASHMAP;
	(*map)->data.hashmap.key = key;
	(*map)->data.hashmap.value = value;

	return value;	
    }
    
    else
	return 0;
}

#endif // LIBBLACKSQUID_IMPLEMENTATION
#endif // LIBBLACKSQUID_H
