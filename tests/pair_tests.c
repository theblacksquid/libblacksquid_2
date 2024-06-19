
#include <stdio.h>
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
		printf("%d\n", pair_head(tracker)->data.integer);   
	}

	{
	    for (ltbs_cell *tracker = pair_reverse(list, &context);
		 tracker->data.pair.head != 0;
		 tracker = pair_rest(tracker))
		printf("%d\n", pair_head(tracker)->data.integer);
	}
    }

    arena_free(&context);
    return 0;
}
