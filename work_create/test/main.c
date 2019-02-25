#include <stdio.h>

typedef struct a {
	int a;
	char b[0];
}a_t;

typedef struct b {
	int a;
	char b[];
}b_t;

int main(int argc, char* argv[]) {
	char a[0];
	char b[];	

	printf ("sizeof(a): %lu, sizeof(b): %lu\n", sizeof(a), sizeof(b));
	printf ("sizeof(a_t): %lu, sizeof(b_t): %lu\n", sizeof(a_t), sizeof(b_t));

	return 0;
}

