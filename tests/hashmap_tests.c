
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
	printf("\n\nWith hash_lookup\n\n");
	printf("'hello': %d\n", hash_lookup(&hashmap, "hello")->data.integer);
	printf("'cruel': %d\n", hash_lookup(&hashmap, "cruel")->data.integer);
	printf("'world': %d\n", hash_lookup(&hashmap, "world")->data.integer);
    }

    {
	ltbs_cell *hashmap;
	printf("Doing bulk insert...\n");

	hashmap_from_kvps(
	    hashmap, &context,
	    {"hello", int_from_int(42, &context)},
	    {"cruel", int_from_int(190, &context)},
	    {"world", int_from_int(999, &context)}
        );

	printf("\n\nWith hashmap_from_kvps\n\n");
	printf("'hello': %d\n", hash_lookup(&hashmap, "hello")->data.integer);
	printf("'cruel': %d\n", hash_lookup(&hashmap, "cruel")->data.integer);
	printf("'world': %d\n", hash_lookup(&hashmap, "world")->data.integer);	
    }

    arena_free(&context);
    
    return 0;
}
