
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define LIBBLACKSQUID_IMPLEMENTATION
#include "../libblacksquid.h"

int main ()
{
    Arena context = {0};

    {
	ltbs_cell *hashmap = Hash_Vt.new(&context);
	ltbs_cell *hello = String_Vt.cs("hello", &context);
	ltbs_cell *cruel = String_Vt.cs("cruel", &context);
	ltbs_cell *world = String_Vt.cs("world", &context);

	printf("Inserting 'hello': 42\n");
	Hash_Vt.upsert(
	    &hashmap,
	    hello,
	    &((ltbs_cell) { .data = { .integer = 42 } }),
	    &context
	);

	printf("Inserting 'cruel': 21\n");
	Hash_Vt.upsert(
	    &hashmap,
	    cruel,
	    &((ltbs_cell) { .data = { .integer = 21 } }),
	    &context
	);

	printf("Inserting 'world': 64\n");
	Hash_Vt.upsert(
	    &hashmap,
	    world,
	    &((ltbs_cell) { .data = { .integer = 64 } }),
	    &context
	);

	printf("'hello': %d\n", Hash_Vt.upsert(&hashmap, hello, 0, 0)->data.integer);
	printf("'cruel': %d\n", Hash_Vt.upsert(&hashmap, cruel, 0, 0)->data.integer);
	printf("'world': %d\n", Hash_Vt.upsert(&hashmap, world, 0, 0)->data.integer);
	printf("\n\nWith Hash_Vt.lookup\n\n");
	printf("'hello': %d\n", Hash_Vt.lookup(&hashmap, "hello")->data.integer);
	printf("'cruel': %d\n", Hash_Vt.lookup(&hashmap, "cruel")->data.integer);
	printf("'world': %d\n", Hash_Vt.lookup(&hashmap, "world")->data.integer);
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
	printf("'hello': %d\n", Hash_Vt.lookup(&hashmap, "hello")->data.integer);
	printf("'cruel': %d\n", Hash_Vt.lookup(&hashmap, "cruel")->data.integer);
	printf("'world': %d\n", Hash_Vt.lookup(&hashmap, "world")->data.integer);

	ltbs_cell *keys = Hash_Vt.keys(&hashmap, &context);

	printf("keys: ( ");
	pair_iterate(keys, head, tracker,
        {
	    String_Vt.print(head); printf(" ");
	});
	printf(")\n");
    }

    arena_free(&context);
    
    return 0;
}
