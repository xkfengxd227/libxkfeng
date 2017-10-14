#include "bitvector.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>

#define		N 100

int main(void) {
	BitVector vec(N);

	FILE *in = fopen("in.txt", "r");
	FILE *out = fopen("out.txt", "w");
	if (in == NULL || out == NULL) {
		exit(-1);
	}
	int i = 0;
	int m;
	for (i = 0; i < N; i++) {
		vec.clear(i);
	}
	while (!feof(in)) {
		fscanf(in, "%d", &m);
		printf("%d\n", m);
		vec.set(m);
	}
	printf("after\n");
	for (i = 0; i < N; i++) {
		if (vec.get(i)) {
			printf("%d\n", i);
			fprintf(out, "%d\n", i);
		}
	}
	fclose(in);
	fclose(out);

	return EXIT_SUCCESS;
}
