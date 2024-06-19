
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define ARENA_IMPLEMENTATION
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

int main()
{
    Arena context = {0};

    {
	ltbs_cell* list = arena_alloc(&context, sizeof(ltbs_cell));
	list->type = LTBS_PAIR;
	list->data.pair.head = 0;
	list->data.pair.rest = 0;
	
	for (int index = 0; index < 10; index++)
	    list = pair_cons(ltbs_new_integer(index, &context), list, &context);

	{
	    for (ltbs_cell *tracker = list;
		 tracker->data.pair.head != 0;
		 tracker = pair_rest(tracker))
		printf("%d, ", pair_head(tracker)->data.integer);

	    printf("\n\n");
	}

	{
	    for (ltbs_cell *tracker = pair_reverse(list, &context);
		 tracker->data.pair.head != 0;
		 tracker = pair_rest(tracker))
		printf("%d, ", pair_head(tracker)->data.integer);

	    printf("\n\n");

	    printf("Smallest list1 value: %d\n", pair_min(list, compare_int)->data.integer);
	}

	list = pair_cons(ltbs_new_integer(10, &context), list, &context);
	list = pair_cons(ltbs_new_integer(-1, &context), list, &context);
	list = pair_cons(ltbs_new_integer(20, &context), list, &context);

	/* printf("Smallest list1 value: %d\n", pair_min(list, compare_int)->data.integer); */
	/* printf("Length before min_and_remove: %d\n", pair_length(list)); */
	/* ltbs_cell *removed = pair_min_and_remove(list, compare_int); */
	/* printf("Length after: %d\n", pair_length(list)); */
	/* printf("removed value: %d\n", removed->data.integer); */
	/* printf("Smallest list1 value: %d\n", pair_min(list, compare_int)->data.integer); */

	{
	    printf("List1 current status: \n");
	    for (ltbs_cell *tracker = pair_reverse(list, &context);
		 tracker->data.pair.head != 0;
		 tracker = pair_rest(tracker))
		printf("%d, ", pair_head(tracker)->data.integer);

	    printf("\n\n");
	}

	ltbs_cell* sorted = pair_sort(list, compare_int, &context);

	{
	    printf("List1 sorted: \n");
	    for (ltbs_cell *tracker = sorted;
		 tracker->data.pair.head != 0;
		 tracker = pair_rest(tracker))
		printf("%d, ", pair_head(tracker)->data.integer);

	    printf("\n\n");
	}
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
		list = pair_cons(ltbs_new_integer(rand() % 100, &context), list, &context);
	}

	{
	    for (ltbs_cell *tracker = list;
		 tracker->data.pair.head != 0;
		 tracker = pair_rest(tracker))
		printf("%d, ", pair_head(tracker)->data.integer);

	    printf("\n\n");
	}

	ltbs_cell *sorted = pair_sort(list, compare_int, &context);

	{
	    printf("Sorting randomized list...\n");
	    for (ltbs_cell *tracker = sorted;
		 tracker->data.pair.head != 0;
		 tracker = pair_rest(tracker))
		printf("%d, ", pair_head(tracker)->data.integer);

	    printf("\n\n");
	}

	ltbs_cell *even_ints = pair_filter(sorted, is_even, &context);

	{
	    printf("Filtering for even numbers...\n");

	    for (ltbs_cell *tracker = even_ints;
		 tracker->data.pair.head != 0;
		 tracker = pair_rest(tracker))
		printf("%d, ", pair_head(tracker)->data.integer);

	    printf("\n\n");	    
	}
    }

    arena_free(&context);
    return 0;
}
