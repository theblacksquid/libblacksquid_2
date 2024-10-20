
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define LIBBLACKSQUID_IMPLEMENTATION
#include "../libblacksquid.h"

int main()
{
    Arena context = {0};

    ltbs_cell *string1 = String_Vt.cs("Hello world!", &context);
    ltbs_cell *string2 = String_Vt.cs("Ohayou Minna!\n", &context);

    ltbs_cell *appended = String_Vt.append(string1, string2, &context);

    String_Vt.print(string1); putc('\n', stdout);
    String_Vt.print(string2); putc('\n', stdout);
    String_Vt.print(appended);

    int comparison = string_compare(string1, string2);

    printf("comparing string1 and string2: %d\n", comparison);

    printf("comparing same string content: %d\n", String_Vt.compare(string1, String_Vt.cs("Hello world!", &context)));

    String_Vt.print(String_Vt.substring(appended, 6, 18, &context));

    ltbs_cell *reversed = String_Vt.reverse(appended, &context);

    String_Vt.print(reversed);

    ltbs_cell *split = String_Vt.split(String_Vt.cs("this is an example", &context), ' ', &context);
    ltbs_cell *split2 = String_Vt.split_multi(
	String_Vt.cs("this \n\t\tis\nan\t example", &context),
	String_Vt.cs("\t\n\r ", &context),
	&context
    );

    printf("\n (");
    pair_iterate(split, head, tracker,
    {
	printf("\"");
	String_Vt.print(head);
	printf("\"");
	printf(", ");
    });
    printf(") \n");

    printf("\n (");
    pair_iterate(split2, head, tracker,
    {
	printf("\"");
	String_Vt.print(head);
	printf("\"");
	printf(", ");
    });
    printf(") \n");

    printf("Printing a formatted string...\n");
    printf("%s\n", String_Vt.format(&context, "this is an %s, %s example", "hello world", "(yet another)")->data.string.strdata);
    
    arena_free(&context);
    return 0;
}
