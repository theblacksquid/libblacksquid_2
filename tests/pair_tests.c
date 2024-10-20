
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define LIBBLACKSQUID_IMPLEMENTATION
#include "../libblacksquid.h"

ltbs_cell *ltbs_new_integer(int value, Arena *context)
{
    ltbs_cell *result = arena_alloc(context, sizeof(ltbs_cell));
    result->type = LTBS_INT;
    result->data.integer = value;

    return result;
}

int compare_int(ltbs_cell *val1, ltbs_cell *val2)
{
    return val1->data.integer > val2->data.integer;
}

int is_even(ltbs_cell *val)
{
    return (val->data.integer % 2) == 0;
}

ltbs_cell *multiply_by_two(ltbs_cell *cell, Arena *context)
{
    return List_Vt.from_int(cell->data.integer * 2, context);
}

void print_int(ltbs_cell *cell)
{
    printf("%d\n", cell->data.integer);
}

int main()
{
    Arena context = {0};

    {
	ltbs_cell* list = arena_alloc(&context, sizeof(ltbs_cell));
	list->type = LTBS_PAIR;
	list->data.pair.head = 0;
	list->data.pair.rest = 0;
	
	for (int index = 0; index < 10; index++)
	    list = List_Vt.cons(ltbs_new_integer(index, &context), list, &context);

	{
	    pair_iterate(list, head, tracker, { printf("%d, ", head->data.integer); });

	    printf("\n\n");
	}

	{
	    pair_iterate(List_Vt.reverse(list, &context), head, tracker,
            {
		printf("%d, ", head->data.integer);
	    });

	    printf("\n\n");

	    printf("Smallest list1 value: %d\n", List_Vt.min(list, compare_int)->data.integer);
	}

	list = List_Vt.cons(ltbs_new_integer(10, &context), list, &context);
	list = List_Vt.cons(ltbs_new_integer(-1, &context), list, &context);
	list = List_Vt.cons(ltbs_new_integer(20, &context), list, &context);
	
	{
	    printf("List1 current status: \n");

	    pair_iterate(pair_reverse(list, &context), head, tracker,
	    {
		printf("%d, ", head->data.integer);
	    });

	    printf("\n\n");
	}

	ltbs_cell* sorted = pair_sort(list, compare_int, &context);

	{
	    printf("List1 sorted: \n");
	    pair_iterate(sorted, head, tracker, { printf("%d, ", head->data.integer); });

	    printf("\n\n");
	}


	printf("Appending lists...\n");
	ltbs_cell *appended = List_Vt.append(list, sorted, &context);
	pair_iterate(appended, head, tracker, { printf("%d, ", head->data.integer); });
	printf("\n\n");
    }

    {
	srand(time(NULL));
	ltbs_cell* list = arena_alloc(&context, sizeof(ltbs_cell));
	list->type = LTBS_PAIR;
	list->data.pair.head = 0;
	list->data.pair.rest = 0;

	{
	    printf("creating randomized list... \n");
	    for (int index = 0; index < 20; index++)
		list = List_Vt.cons(ltbs_new_integer(rand() % 100, &context), list, &context);
	}

	{
	    pair_iterate(list, head, tracker, { printf("%d, ", head->data.integer); });

	    printf("\n\n");
	}

	ltbs_cell *sorted = List_Vt.sort(list, compare_int, &context);

	{
	    printf("Sorting randomized list...\n");

	    pair_iterate(sorted, head, tracker, { printf("%d, ", head->data.integer); });

	    printf("\n\n");
	}

	ltbs_cell *even_ints = List_Vt.filter(sorted, is_even, &context);

	{
	    printf("Filtering for even numbers...\n");

	    pair_iterate(even_ints, head, tracker, { printf("%d, ", head->data.integer); });

	    printf("\n\n");	    
	}

	ltbs_cell *times_two = List_Vt.map(even_ints, multiply_by_two, &context);

	List_Vt.for_each(times_two, print_int, NULL);
    }

    arena_free(&context);
    return 0;
}
