all:
	gcc main.c -o bin -Wall

san:
	gcc -fsanitize=address main.c -o bin -Wall

checkstyle:
	./checkpatch.pl --no-tree -f main.c

cppcheck:
	cppcheck --enable=all --inconclusive --std=posix main.c
