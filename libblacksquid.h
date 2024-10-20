// libblacksquid.h - v1.0 - MIT License, theblacksquid 2024
//
// Provides operations around data structures based on an Arena
// Allocator, wrapped around a tagged union
// 
// This file provides both the interface and the implementation.
// To instantiate the implementation,
//      #define LIBBLACKSQUID_IMPLEMENTATION
// in *ONE* source file, before #including this file.


// Arena Allocator

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



#ifndef LIBBLACKSQUID_H
#define LIBBLACKSQUID_H
#pragma once

typedef enum ltbs_type ltbs_type;
typedef struct ltbs_cell ltbs_cell;
typedef struct ltbs_string ltbs_string;
typedef struct ltbs_array ltbs_array;
typedef struct ltbs_pair ltbs_pair;
// based on https://nullprogram.com/blog/2023/09/30/
typedef struct ltbs_hashmap ltbs_hashmap;
typedef struct ltbs_keyvaluepair ltbs_keyvaluepair;
typedef int (*compare_fn)(ltbs_cell*, ltbs_cell*);
typedef int (*pred_fn)(ltbs_cell*);
typedef char byte;
typedef ltbs_cell *(*transform_fn)(ltbs_cell *cell, Arena *context);
typedef void (*callback_fn)(ltbs_cell *cell, void *param);

#define HASH_FACTOR 1111111111111111111u

#define pair_iterate(to_iter, head, tracker, ...) { for ( ltbs_cell *tracker = to_iter; pair_head(tracker); tracker = pair_rest(tracker) ) { ltbs_cell *head = pair_head(tracker); __VA_ARGS__ } } 

#define hashmap_from_kvps(hashmap, context, ...) {                          \
    hashmap = hash_make(context);	                                    \
    ltbs_keyvaluepair *kvp_list =                                           \
	(ltbs_keyvaluepair[]) { __VA_ARGS__, {0,0} };			    \
    for (int index = 0; kvp_list[index].key != 0; index++ )                 \
    {                                                                       \
        ltbs_cell *key = string_from_cstring(kvp_list[index].key, context); \
        hash_upsert(                                                        \
	    &hashmap,                                                       \
            key,                                                            \
	    kvp_list[index].value,                                          \
	    context                                                         \
       );                                                                   \
    }						                            \
}					                                    \

struct ltbs_cell
{
    enum ltbs_type
    {
	LTBS_BYTE,
	LTBS_INT,
	LTBS_UINT,
	LTBS_FLOAT,
	LTBS_STRING,
	LTBS_ARRAY,
	LTBS_PAIR,
	LTBS_HASHMAP,
	LTBS_CUSTOM
    } type;

    union
    {
	uint64_t uinteger;
	int64_t integer;
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

        struct
	{
	    void *data;
	    size_t size;
	} custom;
    } data;
};

struct ltbs_keyvaluepair
{
    byte *key;
    ltbs_cell *value;
};

struct ltbs_list_vt
{
    ltbs_cell *(*head)(ltbs_cell *pair);
    ltbs_cell *(*rest)(ltbs_cell *pair);
    ltbs_cell *(*cons)(ltbs_cell *value, ltbs_cell *pair, Arena *context);
    unsigned int (*count)(ltbs_cell *pair);
    ltbs_cell *(*by_index)(ltbs_cell *pair, unsigned int index);
    ltbs_cell *(*reverse)(ltbs_cell *pair, Arena *context);
    ltbs_cell *(*append)(ltbs_cell *list1, ltbs_cell *list2, Arena *context);
    ltbs_cell *(*min)(ltbs_cell *list, compare_fn compare);
    ltbs_cell *(*sort)(ltbs_cell *list, compare_fn compare, Arena *context);
    ltbs_cell *(*filter)(ltbs_cell *list, pred_fn pred, Arena *context);
    ltbs_cell *(*nil)();
    ltbs_cell *(*from_int)(int64_t value, Arena *context);
    ltbs_cell *(*from_float)(double value, Arena *context);
    ltbs_cell *(*from_uint)(uint64_t value, Arena *context);
    ltbs_cell *(*map)(ltbs_cell *list, transform_fn transform, Arena *context);
    void (*for_each)(ltbs_cell *list, callback_fn callback, void *param);
};

extern struct ltbs_list_vt List_Vt;

struct ltbs_string_vt
{
    ltbs_cell *(*cs)(const char *cstring, Arena *context);
    ltbs_cell *(*substring)(ltbs_cell *string, unsigned int start, unsigned int end, Arena *context);
    int (*compare)(ltbs_cell *string1, ltbs_cell *string2);
    ltbs_cell *(*append)(ltbs_cell *string1, ltbs_cell *string2, Arena *context);
    ltbs_cell *(*to_list)(ltbs_cell *string, Arena *context);
    ltbs_cell *(*reverse)(ltbs_cell *string, Arena *context);
    ltbs_cell *(*copy)(ltbs_cell *string, Arena *destination);
    void (*print)(ltbs_cell *string);
    ltbs_cell *(*split)(ltbs_cell *string, byte splitter, Arena *context);
    ltbs_cell *(*split_multi)(ltbs_cell *string, ltbs_cell *splitter, Arena *context);
    ltbs_cell *(*format)(Arena *context, const char *fmt, ...);
};

extern struct ltbs_string_vt String_Vt;

struct ltbs_array_vt
{
    
};

struct ltbs_hashmap_vt
{
    ltbs_cell *(*new)(Arena *context);
    ltbs_cell *(*upsert)(ltbs_cell **map, ltbs_cell *key, ltbs_cell *value, Arena *context);
    int (*compute)(ltbs_string *key);
    ltbs_cell *(*lookup)(ltbs_cell **map, byte *cstring);
    ltbs_cell *(*keys)(ltbs_cell **map, Arena *context);
};

extern struct ltbs_hashmap_vt Hash_Vt;

#endif // LIBBLACKSQUID_H

/* #define LIBBLACKSQUID_IMPLEMENTATION */
#ifdef LIBBLACKSQUID_IMPLEMENTATION
#define ARENA_IMPLEMENTATION

ltbs_cell *ltbs_alloc(Arena *context);
ltbs_cell *int_from_int(int64_t num, Arena *context);
ltbs_cell *uint_from_uint(uint64_t num, Arena *context);
ltbs_cell *from_float(double value, Arena *context);
ltbs_cell *pair_head(ltbs_cell *pair);
ltbs_cell *pair_rest(ltbs_cell *pair);
ltbs_cell *pair_cons(ltbs_cell *value, ltbs_cell *list, Arena *context);
unsigned int pair_is_atom(ltbs_cell *value);
unsigned int pair_count(ltbs_cell *list);
ltbs_cell *pair_by_index(ltbs_cell *list, unsigned int index);
ltbs_cell *pair_reverse(ltbs_cell *list, Arena *context);
ltbs_cell *pair_append(ltbs_cell *list1, ltbs_cell *list2, Arena *context);
unsigned int pair_length(ltbs_cell *list);
ltbs_cell *pair_take(ltbs_cell *list, unsigned int to_take, Arena* context);
ltbs_cell *pair_min(ltbs_cell *list, int (*compare)(ltbs_cell*, ltbs_cell*));
ltbs_cell *pair_sort(ltbs_cell *list, int (*compare)(ltbs_cell*, ltbs_cell*), Arena *context);
ltbs_cell *pair_copy(ltbs_cell *list, Arena *destination);
ltbs_cell *pair_min_and_remove(ltbs_cell *list, int (*compare)(ltbs_cell*, ltbs_cell*));
ltbs_cell *pair_filter(ltbs_cell *list, int (*pred)(ltbs_cell*), Arena *context);
ltbs_cell *pair_nil();
ltbs_cell *pair_map(ltbs_cell *list, transform_fn transform, Arena *context);
void pair_foreach(ltbs_cell *list, callback_fn callback, void *param);

struct ltbs_list_vt List_Vt = (struct ltbs_list_vt)
{
    .head = pair_head,
    .rest = pair_rest,
    .cons = pair_cons,
    .by_index = pair_by_index,
    .count = pair_count,
    .append = pair_append,
    .filter = pair_filter,
    .reverse = pair_reverse,
    .min = pair_min,
    .sort = pair_sort,
    .nil = pair_nil,
    .from_int = int_from_int,
    .from_float = from_float,
    .map = pair_map,
    .for_each = pair_foreach,
    .from_uint = uint_from_uint,
};

ltbs_cell *string_from_cstring(const char *cstring, Arena *context);
ltbs_cell *string_substring(ltbs_cell *string, unsigned int start, unsigned int end, Arena *context);
int string_compare(ltbs_cell *string1, ltbs_cell *string2);
ltbs_cell *string_append(ltbs_cell *string1, ltbs_cell *string2, Arena *context);
ltbs_cell *string_to_list(ltbs_cell *string, Arena *context);
ltbs_cell *string_reverse(ltbs_cell *string, Arena *context);
ltbs_cell *string_copy(ltbs_cell *string, Arena *destination);
void string_print(ltbs_cell *string);
ltbs_cell *string_split(ltbs_cell *string, byte splitter, Arena *context);
ltbs_cell *string_split_multi(ltbs_cell *string, ltbs_cell *splitter, Arena *context);
ltbs_cell *string_format(Arena *context, const char *fmt, ...);

struct ltbs_string_vt String_Vt = (struct ltbs_string_vt)
{
    .cs = string_from_cstring,
    .substring = string_substring,
    .compare = string_compare,
    .append = string_append,
    .to_list = string_to_list,
    .reverse = string_reverse,
    .copy = string_copy,
    .print = string_print,
    .split = string_split,
    .split_multi = string_split_multi,
    .format = string_format,
};

ltbs_cell *array_to_list(ltbs_cell *array, Arena *context);
ltbs_cell *array_ref(ltbs_cell *array, unsigned int index);
ltbs_cell *pair_to_array(ltbs_cell *list, Arena *context);
ltbs_cell *array_reverse(ltbs_cell *array, Arena *context);
ltbs_cell *array_slice(ltbs_cell *array, unsigned int start, unsigned int length, Arena *context);
ltbs_cell *array_append(ltbs_cell *array1, ltbs_cell *array2, Arena *context);
ltbs_cell *array_copy(ltbs_cell *array, Arena *destination);

ltbs_cell *hash_make(Arena *context);
ltbs_cell *hash_upsert(ltbs_cell **map, ltbs_cell *key, ltbs_cell *value, Arena *context);
int hash_compute(ltbs_string *key);
ltbs_cell *hash_lookup(ltbs_cell **map, byte *cstring);
ltbs_cell *hash_keys(ltbs_cell **map, Arena *context);

struct ltbs_hashmap_vt Hash_Vt = (struct ltbs_hashmap_vt)
{
    .new = hash_make,
    .upsert = hash_upsert,
    .compute = hash_compute,
    .lookup = hash_lookup,
    .keys = hash_keys,
};

ltbs_cell *format_string(char *format, ltbs_cell *data_list, Arena *context);
ltbs_cell *format_serialize(char *format, ltbs_cell *data_map, Arena *context);

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

static const ltbs_cell PAIR_NIL = (ltbs_cell)
{
    .type = LTBS_PAIR,
    .data = { .pair = { .head = 0, .rest = 0 } }
};

ltbs_cell *pair_nil()
{
    return &PAIR_NIL;
}

ltbs_cell *ltbs_alloc(Arena *context)
{
    ltbs_cell *result = arena_alloc(context, sizeof(ltbs_cell));
    *result = (const ltbs_cell) {0};
    return result;
}

ltbs_cell *int_from_int(int64_t num, Arena *context)
{
    ltbs_cell *result = ltbs_alloc(context);
    result->type = LTBS_INT;
    result->data.integer = num;

    return result;
}

ltbs_cell *from_float(double num, Arena *context)
{
    ltbs_cell *result = ltbs_alloc(context);
    result->type = LTBS_FLOAT;
    result->data.floatval = num;

    return result;
}

ltbs_cell *uint_from_uint(uint64_t num, Arena *context)
{
    ltbs_cell *result = ltbs_alloc(context);
    result->type = LTBS_UINT;
    result->data.uinteger = num;

    return result;    
}

ltbs_cell *pair_head(ltbs_cell *list)
{
    return list->data.pair.head;
}

ltbs_cell *pair_rest(ltbs_cell *list)
{
    return list->data.pair.rest;
}

ltbs_cell *pair_cons(ltbs_cell *value, ltbs_cell *rest, Arena *context)
{
    ltbs_cell *new_cell = arena_alloc(context, sizeof(ltbs_cell));
    new_cell->data.pair.head = value;
    new_cell->data.pair.rest = rest;
    new_cell->type = LTBS_PAIR;

    return new_cell;
}

unsigned int pair_is_atom(ltbs_cell *value)
{
    if ( (value->type != LTBS_PAIR) ||
	 (value->type != LTBS_ARRAY) ||
	 (value->type != LTBS_HASHMAP) )
	return 0;
    else
	return 1;
}

unsigned int pair_count(ltbs_cell *list)
{
    unsigned int result = 0;

    pair_iterate(list, _, __, { result++; });

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

ltbs_cell *pair_append(ltbs_cell* list1, ltbs_cell* list2, Arena* context)
{
    Arena *workspace       = malloc(sizeof(Arena));
    *workspace             = (Arena) {0};
    ltbs_cell* result      = arena_alloc(context, sizeof(ltbs_cell));
    result->type           = LTBS_PAIR;
    result->data.pair.head = 0;
    result->data.pair.rest = 0;

    {
	pair_iterate(pair_reverse(list1, workspace), head, tracker,
	{
	    result = pair_cons(pair_head(tracker), result, context);
	});
    }

    {
	pair_iterate(pair_reverse(list2, workspace), head, tracker,
        {
	    result = pair_cons(head, result, context);
	});
    }

    arena_free(workspace);
    free(workspace);
    return result;
}   

unsigned int pair_length(ltbs_cell* list)
{
    unsigned int result = 0;

    pair_iterate(list, _, __, { result++; });

    return result;
}

ltbs_cell* pair_reverse(ltbs_cell* list, Arena* context)
{
    ltbs_cell* result = arena_alloc(context, sizeof(ltbs_cell));
    result->type = LTBS_PAIR;
    result->data.pair.head = 0;
    result->data.pair.rest = 0;

    pair_iterate(list, head, _, { result = pair_cons(head, result, context); });

    return result;
}

ltbs_cell *pair_min(ltbs_cell *list, int (*compare)(ltbs_cell*, ltbs_cell*))
{
    int length = pair_length(list);

    if ( length == 1 )
	return pair_head(list);

    else
    {
	ltbs_cell *current_min = pair_head(list);

        pair_iterate(list, head, _,
	{
	    if ( compare(current_min, head) > 0 )
	    {
		current_min = head;
	    }
        });

	return current_min;
    }
}

ltbs_cell *pair_min_and_remove(ltbs_cell *list, int (*compare)(ltbs_cell*, ltbs_cell*))
{
    int length = pair_length(list);

    if ( length == 1 )
	return pair_head(list);

    else
    {
	ltbs_cell *current_min = pair_head(list);
	ltbs_cell *before_min = list;
	ltbs_cell *follow = list;

	pair_iterate(pair_rest(list), head, tracker,
	{   
	    if ( compare(current_min, head) > 0 )
	    {
		before_min = follow;
		current_min = head;
	    }

	    follow = tracker;
	});

	if ( current_min != pair_head(list) )
	{
	    before_min->data.pair.rest = pair_rest(pair_rest(before_min));
	}

	return current_min;
    }
}

ltbs_cell *pair_copy(ltbs_cell *list, Arena *destination)
{
    Arena *workspace = malloc(sizeof(Arena));
    *workspace = (Arena) {0};
    ltbs_cell *result;
    ltbs_cell *current_list = arena_alloc(workspace, sizeof(ltbs_cell));
    *current_list = PAIR_NIL;
    
    pair_iterate(list, head, tracker,
    {
	current_list = pair_cons(head, current_list, workspace);
    });

    result = pair_reverse(current_list, destination);
    arena_free(workspace);
    free(workspace);
    return result;
}

ltbs_cell *pair_sort(ltbs_cell *list, int (*compare)(ltbs_cell*, ltbs_cell*), Arena *context)
{
    Arena *workspace = malloc(sizeof(Arena));
    *workspace = (Arena) {0};
    ltbs_cell *result;
    ltbs_cell *output = arena_alloc(workspace, sizeof(ltbs_cell));
    ltbs_cell *list_copy = pair_copy(list, workspace);
    int length = pair_length(list_copy);

    *output = PAIR_NIL;

    while ( length > 1 )
    {	
	ltbs_cell *min = pair_min_and_remove(list_copy, compare);
	if ( min == pair_head(list_copy) )
	    list_copy = pair_rest(list_copy);
	
        output = pair_cons(min, output, workspace);
	length = pair_length(list_copy);
    }

    if ( pair_head(list_copy) != 0 )
	output = pair_cons(pair_head(list_copy), output, workspace);

    result = pair_reverse(output, context);
    arena_free(workspace);
    free(workspace);
    return result;
}

ltbs_cell *pair_filter(ltbs_cell *list, int (*pred)(ltbs_cell*), Arena *context)
{
    ltbs_cell *result = arena_alloc(context, sizeof(ltbs_cell));
    result->type = LTBS_PAIR;
    result->data.pair.head = 0;
    result->data.pair.rest = 0;

    pair_iterate(list, head, tracker,
    {
	if ( pred(head) )
	    result = pair_cons(head, result, context);
    });

    return result;
}

ltbs_cell *pair_map(ltbs_cell *list, transform_fn transform, Arena *context)
{
    ltbs_cell *result = pair_nil();

    pair_iterate(list, head, tracker,
    {
	result = pair_cons(transform(head, context), result, context);
    });

    return result;
}

void pair_foreach(ltbs_cell *list, callback_fn callback, void *param)
{
    pair_iterate(list, head, tracker,
    {
	callback(head, param);
    });
}

ltbs_cell *string_from_cstring(const char *cstring, Arena *context)
{
    ltbs_cell *result = arena_alloc(context, sizeof(ltbs_cell));
    int length = 0;
    result->type = LTBS_STRING;
    
    for ( byte ch = cstring[0]; ch != 0; ch = cstring[++length] ){}

    byte *buffer = arena_alloc(context, length);

    result->data.string.strdata = buffer;
    result->data.string.length = length;

    for ( int index = 0; index < length; index++ )
	buffer[index] = cstring[index];

    buffer[length] = 0;
    
    return result;
}

ltbs_cell *string_substring(ltbs_cell *string, unsigned int start, unsigned int end, Arena *context)
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

int string_compare(ltbs_cell *string1, ltbs_cell *string2)
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

ltbs_cell *string_append(ltbs_cell *string1, ltbs_cell *string2, Arena *context)
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

void string_print(ltbs_cell *string)
{
    for (unsigned int index = 0; index < string->data.string.length; index++)
	putc(string->data.string.strdata[index], stdout);
}

ltbs_cell *string_to_list(ltbs_cell *string, Arena *context)
{
    ltbs_cell *result = arena_alloc(context, sizeof(ltbs_cell));
    unsigned int length = string->data.string.length;

    result->type = LTBS_PAIR;
    result->data.pair.head = 0;
    result->data.pair.rest = 0;

    for (unsigned int index = 0; index < length; index++)
    {
	ltbs_cell *to_add = arena_alloc(context, sizeof(ltbs_cell));
	to_add->type = LTBS_BYTE;
	to_add->data.byteval = string->data.string.strdata[index];
	result = pair_cons(to_add, result, context);
    }

    return result;
}

ltbs_cell *string_reverse(ltbs_cell *string, Arena *context)
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

ltbs_cell *string_copy(ltbs_cell *string, Arena *destination)
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

ltbs_cell *string_split(ltbs_cell *string, byte splitter, Arena *context)
{
    ltbs_cell *copy = string_copy(string, context);
    ltbs_cell *result = ltbs_alloc(context); *result = PAIR_NIL;
    byte *buffer_copy = copy->data.string.strdata;
    
    unsigned int index = 0;
    unsigned int current_length = 0;

    while ( index < copy->data.string.length )
    {
	if ( buffer_copy[index] == splitter )
	{
	    ltbs_cell *to_add = ltbs_alloc(context);
	    to_add->type = LTBS_STRING;
	    to_add->data.string.strdata = &buffer_copy[index - current_length];
	    to_add->data.string.length = current_length;
	    buffer_copy[index] = '\0';
	    result = pair_cons(to_add, result, context);

	    current_length = 0;
	}

	else current_length++;
	
	index++;
    }

    ltbs_cell *to_add = ltbs_alloc(context);
    to_add->type = LTBS_STRING;
    to_add->data.string.strdata = &buffer_copy[index - current_length];
    to_add->data.string.length = current_length;
    buffer_copy[index] = '\0';
    result = pair_cons(to_add, result, context);
    result = pair_reverse(result, context);

    return result;
}

int string_contains(ltbs_cell *string, byte c)
{
    int result = 0;
    byte *buffer = string->data.string.strdata;
    int length = string->data.string.length;

    for ( int index = 0; index < length; index++ )
    {
	if ( buffer[index] == c ) result = 1;
    }

    return result;
}

ltbs_cell *string_split_multi(ltbs_cell *string, ltbs_cell *splitter, Arena *context)
{
    ltbs_cell *copy = string_copy(string, context);
    ltbs_cell *result = ltbs_alloc(context); *result = PAIR_NIL;
    byte *buffer_copy = copy->data.string.strdata;
    byte *buffer = splitter->data.string.strdata;
    int length = splitter->data.string.length;
    
    unsigned int index = 0;
    unsigned int current_length = 0;

    while ( index < copy->data.string.length )
    {
	if ( string_contains(splitter, buffer_copy[index]) && (current_length != 0) )
	{
	    ltbs_cell *to_add = ltbs_alloc(context);
	    to_add->type = LTBS_STRING;
	    to_add->data.string.strdata = &buffer_copy[index - current_length];
	    to_add->data.string.length = current_length;
	    buffer_copy[index] = '\0';
	    result = pair_cons(to_add, result, context);

	    current_length = 0;
	}

	else if (string_contains(splitter, buffer_copy[index]))
	    current_length = 0;

	else current_length++;
	
	index++;
    }

    ltbs_cell *to_add = ltbs_alloc(context);
    to_add->type = LTBS_STRING;
    to_add->data.string.strdata = &buffer_copy[index - current_length];
    to_add->data.string.length = current_length;
    buffer_copy[index] = '\0';
    result = pair_cons(to_add, result, context);
    result = pair_reverse(result, context);

    return result;
}

int count_string_formats(const char *format)
{
    int result = 0;
    int is_prev_percent = 0;

    for ( int index = 0; format[index] != 0; index++ )
    {
	if ( format[index] == '%' && !is_prev_percent )
        {
	    result++;
	    is_prev_percent = 1;
	}

	else 
	{
	    is_prev_percent = 0;
	}
    }

    return result;
}

int ltbs_strlen(char *string)
{
    int result = 0;

    for (int index = 0; string[index] != 0; index++)
	result++;

    return result;
}

ltbs_cell *string_format(Arena *context, const char *fmt, ...)
{
    ltbs_cell *result = ltbs_alloc(context);
    char *buffer;
    size_t total_size = 0;
    va_list arguments;
    FILE *as_file;
    
    va_start(arguments, fmt);

    {
	for (size_t index = 0; fmt[index]; index++) total_size++;
    }

    {
	int total_formats = count_string_formats(fmt);

	for ( int index = 0; index < total_formats; index++ )
	{
	    char *current = va_arg(arguments, char *);
	    total_size += ltbs_strlen(current);
	}

	buffer = arena_alloc(context, total_size + 1);
    }

    {
	for ( size_t index = 0; index < total_size; index++ ) buffer[index] = 0;
    }

    va_end(arguments);

    va_start(arguments, fmt);

    result->type = LTBS_STRING;
    result->data.string.length = total_size;
    result->data.string.strdata = buffer;

    as_file = fmemopen(buffer, total_size, "r+");
    vfprintf(as_file, fmt, arguments);
    fclose(as_file);

    va_end(arguments);
    return result;
}

ltbs_cell *array_to_list(ltbs_cell *array, Arena *context)
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

ltbs_cell *array_ref(ltbs_cell *array, unsigned int index)
{
    return array->data.array.arrdata[index];
}

ltbs_cell *pair_to_array(ltbs_cell *list, Arena *context)
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

ltbs_cell *array_reverse(ltbs_cell *array, Arena *context)
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

ltbs_cell *array_slice(ltbs_cell *array, unsigned int start, unsigned int length, Arena *context)
{
    ltbs_cell *result = ltbs_alloc(context);
    result->type = LTBS_ARRAY;
    result->data.array.length = length;
    result->data.array.arrdata = &array->data.array.arrdata[start];

    return result;
}

ltbs_cell *array_append(ltbs_cell *array1, ltbs_cell *array2, Arena *context)
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

ltbs_cell *array_copy(ltbs_cell *array, Arena *destination)
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

ltbs_cell *hash_make(Arena *context)
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

int hash_compute(ltbs_string *key)
{
    int result = 0x100;

    for (unsigned int index = 0; index < key->length; index++)
    {
	result ^= key->strdata[index];
	result *= HASH_FACTOR;
    }

    return result;
}

ltbs_cell *hash_upsert(ltbs_cell **map, ltbs_cell *key, ltbs_cell *value, Arena *context)
{
    ltbs_cell *result = 0;
    
    for (unsigned int hash = hash_compute(&key->data.string); *map; hash <<= 2)
    {	
	if ( ((*map)->data.hashmap.key != 0) &&
	     string_compare(key, (*map)->data.hashmap.key) )
	{
	    if ( (context != 0) && (value != 0) )
	    {
		ltbs_cell *key_copy = string_copy(key, context);
		*map = ltbs_alloc(context);
		(*map)->type = LTBS_HASHMAP;
		(*map)->data.hashmap.key = key_copy;
		(*map)->data.hashmap.value = value;

		for ( int index = 0; index < 4; index++ )
		    (*map)->data.hashmap.children[index] = 0;

		result = value;
	        break;
	    }

	    result = (*map)->data.hashmap.value;
	    break;
	}

	map = &(*map)->data.hashmap.children[hash >> 30];
    }

    if ( (context != 0) && (value != 0) )
    {
	ltbs_cell *key_copy = string_copy(key, context);
	*map = ltbs_alloc(context);
	(*map)->type = LTBS_HASHMAP;
	(*map)->data.hashmap.key = key_copy;
	(*map)->data.hashmap.value = value;

	for ( int index = 0; index < 4; index++ )
	    (*map)->data.hashmap.children[index] = 0;

	result = value;
    }
    
    return result;
}

ltbs_cell *hash_lookup(ltbs_cell **map, byte *cstring)
{
    Arena *scratch = malloc(sizeof(Arena));
    *scratch = (Arena) {0};
    ltbs_cell *result = 0;
    
    ltbs_cell *key = string_from_cstring(cstring, scratch);
    result = hash_upsert(map, key, 0, 0);
    
    arena_free(scratch);
    free(scratch);

    return result;
}

void __hash_keys_impl(ltbs_cell **map, ltbs_cell **out_list, Arena *context)
{
    ltbs_cell *key = (*map)->data.hashmap.key;
    *out_list = pair_cons(key, *out_list, context);

    for (int index = 0; index < 4; index++)
    {
	ltbs_cell *child = (*map)->data.hashmap.children[index];
	if ( child != 0 ) __hash_keys_impl(&child, out_list, context);
    }
}

ltbs_cell *hash_keys(ltbs_cell **map, Arena *context)
{
    ltbs_cell *result = ltbs_alloc(context); *result = PAIR_NIL;
    __hash_keys_impl(map, &result, context);
    return result;
}

#endif // LIBBLACKSQUID_IMPLEMENTATION


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
