
FLAGS_DEFAULT=\
-g3                   \
-Wall                 \
-Wextra               \
-Wconversion          \
-Wdouble-promotion    \
-Wno-unused-parameter \
-Wno-unused-function  \
-Wno-sign-conversion

WITH_VALGRIND=$(FLAGS_DEFAULT) -fsanitize=undefined
WITH_ASAN=$(FLAGS_DEFAULT) -fsanitize=undefined,address

DEPS=\
$(shell pkg-config --cflags --libs libxml-2.0) \
$(shell pkg-config --cflags --libs sqlite3)

all: pair hashmap string xml_vg xml_asan sqlite_tests_vg

pair: tests/pair_tests.c
	gcc $(WITH_ASAN) tests/pair_tests.c -o pair;
	./pair;
	rm pair;
	gcc $(WITH_VALGRIND) tests/pair_tests.c -o pair;
	valgrind ./pair;

string: tests/string_tests.c
	gcc $(WITH_ASAN) tests/string_tests.c -o string;
	./string;
	rm ./string;
	gcc $(WITH_VALGRIND) tests/string_tests.c -o string;
	valgrind ./string;

hashmap: tests/hashmap_tests.c
	gcc $(WITH_ASAN) tests/hashmap_tests.c -o hashmap;
	./hashmap;
	rm ./hashmap;
	gcc $(WITH_VALGRIND) tests/hashmap_tests.c -o hashmap;
	valgrind ./hashmap;

xml_vg: xml_vg.o
	gcc xml_vg.o $(WITH_VALGRIND) $(DEPS) -o xml_vg
	valgrind ./xml_vg

xml_asan: xml_asan.o
	gcc xml_asan.o $(WITH_ASAN) $(DEPS) -o xml_asan
	./xml_asan

xml_vg.o: tests/lxml2_tests.c
	gcc -c $(WITH_VALGRIND) $(DEPS) tests/lxml2_tests.c -o xml_vg.o

xml_asan.o: tests/lxml2_tests.c
	gcc -c $(WITH_ASAN) $(DEPS) tests/lxml2_tests.c -o xml_asan.o

sqlite_tests_vg: sqlite_tests_vg.o
	gcc sqlite_tests_vg.o $(WITH_VALGRIND) $(DEPS) -o sqlite_tests_vg;
	valgrind ./sqlite_tests_vg;

sqlite_tests_vg.o: ltbs_sqlite.h
	gcc -c tests/sqlite_tests.c $(WITH_VALGRIND) $(DEPS) -o sqlite_tests_vg.o;

ltbs_sqlite.h: ltbs_sqlite.h.in libblacksquid.h
	cat libblacksquid.h ltbs_sqlite.h.in > ltbs_sqlite.h

clean:
	-rm *.o
	-rm ./pair;
	-rm ./hashmap;
	-rm ./string;
	-rm ./xml_vg;
	-rm ./xml_asan;
	-rm ./sqlite_tests_vg;
	-rm ./ltbs_sqlite.h;
