
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define ARENA_IMPLEMENTATION
#define LIBBLACKSQUID_IMPLEMENTATION
#include "../libblacksquid.h"

int main()
{
    Arena context = {0};

    ltbs_cell *string1 = string_from_cstring("Hello world!", &context);
    ltbs_cell *string2 = string_from_cstring("Ohayou Minna!\n", &context);

    ltbs_cell *appended = string_append(string1, string2, &context);

    string_print(string1); putc('\n', stdout);
    string_print(string2); putc('\n', stdout);
    string_print(appended);

    int comparison = string_compare(string1, string2);

    printf("comparing string1 and string2: %d\n", comparison);

    printf("comparing same string content: %d\n", string_compare(string1, string_from_cstring("Hello world!", &context)));

    string_print(string_substring(appended, 6, 18, &context));

    ltbs_cell *reversed = string_reverse(appended, &context);

    string_print(reversed);

    ltbs_cell *split = string_split(string_from_cstring("this is an example", &context), ' ', &context);

    printf("\n (");
    pair_iterate(split, head, tracker,
    {
	printf("\"");
	string_print(head);
	printf("\"");
	printf(", ");
    });
    printf(") \n");
    
    arena_free(&context);
    return 0;
}
