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
int main() {
	FILE *file1 = fopen("test1.txt", "r");
	FILE *file2 = fopen("test2.txt", "r");
	
	if (compare_two_files(file1, file2)) {
		printf("files are the same");
	} else {
		printf("files are different");
	}
	return 0;
}

