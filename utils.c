#include "utils.h"


bool compare_two_files(FILE *file1, FILE *file2) {
	rewind(file1);
	rewind(file2);
	int c1;
	int c2;
	while ((c1 != EOF) && (c2 != EOF)) {
		c1 = fgetc(file1);
		c2 = fgetc(file2);
		if (c1 != c2) {
			putchar(c1);
			printf(" ");
			putchar(c2);
			printf("\n");
			rewind(file1);
			rewind(file2);
			return false;
		}
		putchar(c1);
		printf(" ");
		putchar(c2);
		printf("\n");
	}
	rewind(file1);
	rewind(file2);
	return true;
	
}

