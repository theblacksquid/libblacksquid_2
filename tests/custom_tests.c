#define ARENA_IMPLEMENTATION
#define LIBBLACKSQUID_IMPLEMENTATION
#include "../libblacksquid.h"
#include <stdio.h>

typedef struct point point;
struct point
{
    int x;
    int y;
};

ltbs_cell *make_point(int x, int y, Arena *context)
{
    ltbs_cell *result = ltbs_alloc(context);
    point *data = arena_alloc(context, sizeof(point));
    *data = (point) { .x = x, .y = y };

    *result = (ltbs_cell)
    {
	.type = LTBS_CUSTOM,
	.data = { .custom = { .data = data, .size = sizeof(point) } }
    };

    return result;
}

int main()
{
    Arena context = {0};

    ltbs_cell *list = ltbs_alloc(&context);
    ltbs_cell *wrapper = hash_make(&context);
    *list = PAIR_NIL;

    for ( int index = 0; index < 20; index++ )
	list = pair_cons(make_point(index, index * 2, &context), list, &context);


    hash_upsert(&wrapper, string_from_cstring("list", &context), list, &context);

    ltbs_cell *formatted = format_string("{{list}}", wrapper, &context);
    string_print(formatted);
    printf("\n");

    pair_iterate(list, head, tracker,
    {
        point *value = head->data.custom.data;
	printf("point[%d, %d], ", value->x, value->y);
    });

    printf("\n");
    
    return 0;
}
