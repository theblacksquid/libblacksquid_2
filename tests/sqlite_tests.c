
#define LIBBLACKSQUID_IMPLEMENTATION
#define LTBS_SQLITE_IMPLEMENTATION
#include "../ltbs_sqlite.h"
#include <stdio.h>

void test_with_db(sqlite3 *db, int *return_code);

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

    {
	int return_code;
	Ltbs_Sqlite3_vt.with_db(":memory:", test_with_db, &return_code);
    }
    
    arena_free(&context);
    return 0;
}

void test_with_db(sqlite3 *db, int *return_code)
{
    static Arena context = {0};
    
    const char schema[] =
	"CREATE TABLE Transactions "
	"( "
	"    ID            INTEGER PRIMARY KEY NOT NULL, "
	"    TxnDate       DATETIME NOT NULL, "
	"    AccountFrom   TEXT NOT NULL, "
	"    AccountTo     TEXT NOT NULL, "
	"    TxnValue      DOUBLE NOT NULL, "
	"    Comment       TEXT NULL "
	") ";

    Ltbs_Sqlite3_vt.with_db_execsql(db, schema, return_code);

    if ( *return_code != SQLITE_OK )
    {
	fprintf(
	    stderr,
	    "SQLITE ERROR : %d\n"
	    "Line         : %d\n"
	    "Error Message: %s\n",
	    *return_code,
	    __LINE__,
	    sqlite3_errmsg(db)
	    );
	return;
    }

    Ltbs_Sqlite3_vt.with_db_execsql(
	db,
	"INSERT INTO Transactions "
	"(TxnDate, AccountFrom, AccountTo, TxnValue, Comment) "
	"VALUES (?, ?, ?, ?, ?)",
	return_code,
	String_Vt.cs("2024-10-01", &context),
	String_Vt.cs("CurrentAccount", &context),
	String_Vt.cs("Generic Supermarket", &context),
	List_Vt.from_float(100, &context),
	String_Vt.cs("buying groceries", &context)
    );

    if ( *return_code != SQLITE_OK )
    {
	fprintf(
	    stderr,
	    "SQLITE ERROR : %d\n"
	    "Line         : %d\n"
	    "Error Message: %s\n",
	    *return_code,
	    __LINE__,
	    sqlite3_errmsg(db)
	    );
	return;
    }
    
    Ltbs_Sqlite3_vt.with_db_execsql(
	db,
	"INSERT INTO Transactions "
	"(TxnDate, AccountFrom, AccountTo, TxnValue, Comment) "
	"VALUES (?, ?, ?, ?, ?)",
	return_code,
	String_Vt.cs("2024-10-02", &context),
	String_Vt.cs("Evil Megacorp", &context),
	String_Vt.cs("CurrentAccount", &context),
	List_Vt.from_float(10000, &context),
	String_Vt.cs("payroll", &context)
    );

    if ( *return_code != SQLITE_OK )
    {
	fprintf(
	    stderr,
	    "SQLITE ERROR : %d\n"
	    "Line         : %d\n"
	    "Error Message: %s\n",
	    *return_code,
	    __LINE__,
	    sqlite3_errmsg(db)
	    );
	return;
    }

    Ltbs_Sqlite3_vt.with_db_execsql(
	db,
	"INSERT INTO Transactions "
	"(TxnDate, AccountFrom, AccountTo, TxnValue, Comment) "
	"VALUES (?, ?, ?, ?, ?)",
	return_code,
	String_Vt.cs("2024-10-03", &context),
	String_Vt.cs("CurrentAccount", &context),
	String_Vt.cs("DomainNamesForCheap.io", &context),
	List_Vt.from_float(50, &context),
	String_Vt.cs("Renew domain", &context)
    );

    if ( *return_code != SQLITE_OK )
    {
	fprintf(
	    stderr,
	    "SQLITE ERROR : %d\n"
	    "Line         : %d\n"
	    "Error Message: %s\n",
	    *return_code,
	    __LINE__,
	    sqlite3_errmsg(db)
	    );
	return;
    }

    Ltbs_Sqlite3_vt.with_db_execsql(
	db,
	"INSERT INTO Transactions "
	"(TxnDate, AccountFrom, AccountTo, TxnValue, Comment) "
	"VALUES (?, ?, ?, ?, ?)",
	return_code,
	String_Vt.cs("2024-10-04", &context),
	String_Vt.cs("CurrentAccount", &context),
	String_Vt.cs("Cash", &context),
	List_Vt.from_float(500, &context),
	String_Vt.cs("Withdraw cash", &context)
    );

    Ltbs_Sqlite3_vt.with_db_execsql(
	db,
	"INSERT INTO Transactions "
	"(TxnDate, AccountFrom, AccountTo, TxnValue, Comment) "
	"VALUES (?, ?, ?, ?, ?)",
	return_code,
	String_Vt.cs("2024-10-04", &context),
	String_Vt.cs("CurrentAccount", &context),
	String_Vt.cs("Withdrawal Fee", &context),
	List_Vt.from_float(5, &context),
	String_Vt.cs("Withdraw cash", &context)
    );

    ltbs_cell *response = Ltbs_Sqlite3_vt.with_db_query(
	db,
	"SELECT * FROM Transactions",
	&context,
	return_code
    );

    if ( *return_code != SQLITE_OK )
    {
	fprintf(
	    stderr,
	    "SQLITE ERROR : %d\n"
	    "Line         : %d\n"
	    "Error Message: %s\n",
	    *return_code,
	    __LINE__,
	    sqlite3_errmsg(db)
	    );
	return;
    }

    printf("\n\n");
    
    pair_iterate(response, row, tracker,
    {
	printf(
	    "\n---------------\n"
	    "ID         : %d\n"
	    "TxnDate    : %s\n"
	    "AccountFrom: %s\n"
	    "AccountTo  : %s\n"
	    "TxnValue   : %f\n"
	    "Comment    : %s\n\n",
	    Hash_Vt.lookup(&row, "ID")->data.integer,
	    Hash_Vt.lookup(&row, "TxnDate")->data.string.strdata,
	    Hash_Vt.lookup(&row, "AccountFrom")->data.string.strdata,
	    Hash_Vt.lookup(&row, "AccountTo")->data.string.strdata,
	    Hash_Vt.lookup(&row, "TxnValue")->data.floatval,
	    Hash_Vt.lookup(&row, "Comment")->data.string.strdata
        );
    });

    ltbs_cell *response2 = Ltbs_Sqlite3_vt.with_db_query(
	db,
	"WITH "
	"GrossValue AS "
	"( "
	"    SELECT AccountTo AS AccountName, "
	"           SUM(TxnValue) AS Balance "
	"    FROM Transactions "
	"    GROUP BY AccountName "
	"), "
	" "
	"Expenses AS "
	"( "
	"    SELECT AccountFrom AS AccountName, "
	"    	   SUM(TxnValue) AS TotalExpense "
	"    FROM Transactions "
	"    GROUP BY AccountName "
	") "
	" "
	"SELECT SUM(Balance) - SUM(IFNULL(TotalExpense, 0)) AS TotalBalance, "
	"       GrossValue.AccountName "
	"FROM GrossValue "
	"LEFT JOIN Expenses ON Expenses.AccountName = GrossValue.AccountName "
	"GROUP BY GrossValue.AccountName ",
	&context,
	return_code
    );

    
    if ( *return_code != SQLITE_OK )
    {
	fprintf(
	    stderr,
	    "SQLITE ERROR : %d\n"
	    "Line         : %d\n"
	    "Error Message: %s\n",
	    *return_code,
	    __LINE__,
	    sqlite3_errmsg(db)
	    );
	return;
    }
    
    pair_iterate(response2, row, tracker,
    {
	printf(
	    "\n---------------\n"
	    "AccountName: %s\n"
	    "Balance    : %f\n",
	    Hash_Vt.lookup(&row, "AccountName")->data.string.strdata,
	    Hash_Vt.lookup(&row, "TotalBalance")->data.floatval
        );
    });

    arena_free(&context);
}
