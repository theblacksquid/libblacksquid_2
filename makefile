
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

all: pair hashmap string

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

clean:
	rm ./pair;
	rm ./hashmap;
	rm ./string;
