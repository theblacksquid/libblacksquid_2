
#define LIBBLACKSQUID_IMPLEMENTATION
#include "../libblacksquid.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int get_random_int(int min, int max)
{
    return (rand() % max) + min;
}

int main()
{
    srand(time(NULL));

    Arena global = {0};
    ltbs_cell *array_copy_test;
    
    {
	Arena context = {0};

	ltbs_cell *as_list = List_Vt.nil();
	ltbs_cell *array;
	
	{
	    for ( int index = 0; index < 100; index++ )
	    {
		int to_add = get_random_int(1, 300);
	    
		as_list = List_Vt.cons(
		    List_Vt.from_int(to_add, &context),
		    as_list,
		    &context
		);
	    }
	}

	array = Array_Vt.from_list(as_list, &context);
	size_t elem = array->data.array.elem_size;
	int length = array->data.array.total_size / elem;

	{
	    printf("\n----------------------\n");
	    printf("Array_Vt.from_list()");
	    printf("\n----------------------\n");
	    
	    for ( int index = 0; index < length; index++ )
	    {
		ltbs_cell *value = Array_Vt.at_index(array, index);
		printf("value: %ld\n", value->data.integer);
	    }
	    
	    printf("\n----------------------\n");
	}

	ltbs_cell *from_array = Array_Vt.to_list(array, &context);

	{
	    printf("\n----------------------\n");
	    printf("Array_Vt.to_list()");
	    printf("\n----------------------\n");

	    for (ltbs_cell *tracker = from_array;
		 List_Vt.head(tracker);
		 tracker = List_Vt.rest(tracker))
	    {
		ltbs_cell *head = List_Vt.head(tracker);
		ltbs_cell *actual = head->data.custom.data;
		printf("value: %ld\n", actual->data.integer);
	    }
	    
	    printf("\n----------------------\n");

	    array_copy_test = Array_Vt.copy(array, &global);
	}

	arena_free(&context);
    }

    printf("\n----------------------\n");
    printf("Array_Vt.copy()");
    printf("\n----------------------\n");

    int length = array_copy_test->data.array.total_size / array_copy_test->data.array.elem_size;
	    
    {
	for ( int index = 0; index < length; index++ )
	{
	    ltbs_cell *value = Array_Vt.at_index(array_copy_test, index);
	    printf("value: %ld\n", value->data.integer);
	}
    }
	    
    printf("\n----------------------\n");

    ltbs_cell *new_array_test = Array_Vt.new_array(sizeof(ltbs_cell), sizeof(ltbs_cell) * 100, &global);

    printf("\n----------------------\n");
    printf("Array_Vt.set_index()");
    printf("\n----------------------\n");

    {
	for ( int index = 0; index < 100; index++ )
	    Array_Vt.set_index(new_array_test, List_Vt.from_int(100 - index, &global), index);
    }

    {
	for ( int index = 0; index < 100; index++ )
	{
	    ltbs_cell *value = Array_Vt.at_index(new_array_test, index);
	    printf("value: %03ld\n", value->data.integer);
	}	
    }

    printf("\n----------------------\n");
    printf("Array.slice()");
    printf("\n----------------------\n");

    ltbs_cell test_array_slice = Array_Vt.slice(new_array_test, 10, 10);

    {
	for ( int index = 0; index < 10; index++ )
	{
	    ltbs_cell *value = Array_Vt.at_index(&test_array_slice, index);
	    printf("value: %03ld\n", value->data.integer);
	}
    }
    
    printf("\n----------------------\n");

    arena_free(&global);
    
    return 0;
}
