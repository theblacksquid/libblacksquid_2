#define ARENA_IMPLEMENTATION
#define LIBBLACKSQUID_IMPLEMENTATION
#include "../libblacksquid.h"
#include <stdio.h>

int main()
{
    Arena context = {0};

    ltbs_cell byte_test = (ltbs_cell) { .type = LTBS_BYTE, .data = { .byteval = '!' } };
    ltbs_cell int_test = (ltbs_cell) { .type = LTBS_INT, .data = { .integer = 12345 }};
    ltbs_cell float_test = (ltbs_cell) { .type = LTBS_FLOAT, .data = { .floatval = (float) -123.456 }};
    ltbs_cell *string_test = string_from_cstring("Hello cruel world", &context);

    byte buffer[10] = {0};
    ltbs_cell custom_test = (ltbs_cell)
    {
	.type = LTBS_CUSTOM,
	.data = { .custom = { .data = buffer, .size = 10 } }
    };

    int array_length = 15;
    ltbs_cell *array_test = ltbs_alloc(&context);
    array_test->type = LTBS_ARRAY;
    array_test->data.array.length = array_length;
    array_test->data.array.arrdata = arena_alloc(&context, sizeof(ltbs_cell) * array_length);

    {
	for ( int index = -7; index < array_length; index++ )
	    array_test->data.array.arrdata[index + 7] = int_from_int(index, &context);	
    }

    ltbs_cell *list_test = ltbs_alloc(&context);
    list_test->type = LTBS_PAIR;
    list_test->data.pair = (ltbs_pair) {0};
    
    {
	for ( int index = 0; index < 15; index++ )
	    list_test = pair_cons(int_from_int(index, &context), list_test, &context);
    }

    ltbs_cell *hashmap_test = hash_make(&context);
    hash_upsert(&hashmap_test, string_from_cstring("byte_test", &context), &byte_test, &context);
    hash_upsert(&hashmap_test, string_from_cstring("int_test", &context), &int_test, &context);
    hash_upsert(&hashmap_test, string_from_cstring("float_test", &context), &float_test, &context);
    hash_upsert(&hashmap_test, string_from_cstring("string_test", &context), string_test, &context);
    hash_upsert(&hashmap_test, string_from_cstring("array_test", &context), array_test, &context);
    hash_upsert(&hashmap_test, string_from_cstring("list_test", &context), list_test, &context);
    hash_upsert(&hashmap_test, string_from_cstring("custom_test", &context), &custom_test, &context);

    ltbs_cell *actual_test = format_string("{{string_test}}, there should be | {{string_test}}, some other text | {{string_test}}, {{string_test}}...", hashmap_test, &context);
    string_print(actual_test);
    printf("\n\n");

    actual_test = format_string("{{list_test}}", hashmap_test, &context);
    string_print(actual_test);
    printf("\n\n");

    actual_test = format_string("{{array_test}}", hashmap_test, &context);
    string_print(actual_test);
    printf("\n\n");

    actual_test = format_string("{{custom_test}}", hashmap_test, &context);
    string_print(actual_test);
    printf("\n\n");

    ltbs_cell *container = hash_make(&context);
    
    hash_upsert(&container, string_from_cstring("hashmap_test", &context), hashmap_test, &context);
    actual_test = format_string("{{hashmap_test}}", container, &context);
    string_print(actual_test);
    printf("\n\n");
    
    arena_free(&context);
}
