
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define ARENA_IMPLEMENTATION
#define LIBBLACKSQUID_IMPLEMENTATION
#include "../libblacksquid.h"

int main ()
{
    Arena context = {0};

    {
	ltbs_cell *hashmap = hash_make(&context);
	ltbs_cell *hello = string_from_cstring("hello", &context);
	ltbs_cell *cruel = string_from_cstring("cruel", &context);
	ltbs_cell *world = string_from_cstring("world", &context);

	printf("Inserting 'hello': 42\n");
	hash_upsert(
	    &hashmap,
	    hello,
	    &((ltbs_cell) { .data = { .integer = 42 } }),
	    &context
	);

	printf("Inserting 'cruel': 21\n");
	hash_upsert(
	    &hashmap,
	    cruel,
	    &((ltbs_cell) { .data = { .integer = 21 } }),
	    &context
	);

	printf("Inserting 'world': 64\n");
	hash_upsert(
	    &hashmap,
	    world,
	    &((ltbs_cell) { .data = { .integer = 64 } }),
	    &context
	);

	printf("'hello': %d\n", hash_upsert(&hashmap, hello, 0, 0)->data.integer);
	printf("'cruel': %d\n", hash_upsert(&hashmap, cruel, 0, 0)->data.integer);
	printf("'world': %d\n", hash_upsert(&hashmap, world, 0, 0)->data.integer);
    }
    
    return 0;
}
