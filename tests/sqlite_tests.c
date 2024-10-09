
#define LIBBLACKSQUID_IMPLEMENTATION
#define LTBS_SQLITE_IMPLEMENTATION
#include "../ltbs_sqlite.h"
#include <stdio.h>

int main()
{
    Arena context = {0};

    const char schema[] =
	"CREATE TABLE Posts "
	"( "
	"    ID        INTEGER PRIMARY KEY NOT NULL, "
	"    Title     TEXT NOT NULL, "
	"    PostDate  DATETIME NOT NULL "
	") ";

    const char *titles[] = {
	"First title!",
	"Second title",
	"this is the Third title",
	"How about a fourth title?",
	"A fifth title, in my test case? More likely than you think!",
	"testing a sixth"
    };

    const char *dates[] = {
	"2024-10-01",
	"2024-10-02",
	"2024-10-03",
	"2024-10-04",
	"2024-10-05",
	"2024-10-06"
    };

    {
	int return_code;
	sqlite3 *db = Ltbs_Sqlite3_vt.connect(":memory:", &return_code);

	if ( return_code != SQLITE_OK )
	{
	    fprintf(stderr, "Error connecting to in-memory db.\n");
	    return 1;
	}

	Ltbs_Sqlite3_vt.with_db_execsql(db, schema, &return_code);

	for ( int index = 0; index < 6; index++ )
	{
	    printf("%s\n", titles[index]);
	    Ltbs_Sqlite3_vt.with_db_execsql(
		db,
		"INSERT INTO Posts (Title, PostDate) VALUES (?, ?) ",
		&return_code,
		String_Vt.cs(titles[index], &context),
		String_Vt.cs(dates[index], &context)
	    );
	}

	ltbs_cell *response = Ltbs_Sqlite3_vt.with_db_query(
	    db,
	    "SELECT * FROM Posts ",
	    &context,
	    &return_code
	);

	if ( response )
	{
	    pair_iterate(response, head, tracker,
	    {
		printf(
		    "----------\n"
		    "ID      : %d\n"
		    "Title   : %s\n"
		    "PostDate: %s\n",
		    Hash_Vt.lookup(&head, "ID")->data.integer,
		    Hash_Vt.lookup(&head, "Title")->data.string.strdata,
		    Hash_Vt.lookup(&head, "PostDate")->data.string.strdata
		    );
	    });
	}
	
	else
	{
	    fprintf(stderr, "SQLITE ERROR CODE: %d\n", return_code);
	}
	
	response = Ltbs_Sqlite3_vt.with_db_query(
	    db,
	    "SELECT * FROM Posts WHERE ID = ?",
	    &context,
	    &return_code,
	    List_Vt.from_int(3, &context)
	);

	if ( response )
	{
	    pair_iterate(response, head, tracker,
	    {
		printf(
		    "\n\nQuery with param\n"
		    "----------\n"
		    "ID      : %d\n"
		    "Title   : %s\n"
		    "PostDate: %s\n",
		    Hash_Vt.lookup(&head, "ID")->data.integer,
		    Hash_Vt.lookup(&head, "Title")->data.string.strdata,
		    Hash_Vt.lookup(&head, "PostDate")->data.string.strdata
		    );
	    })
	}

	Ltbs_Sqlite3_vt.close(db, &return_code);
    }

    
    
    arena_free(&context);
    return 0;
}
