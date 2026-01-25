#include "utils.h"
#include <stdbool.h>
#include <stdio.h>


bool compare_two_files(char *file1, char *file2) {
	FILE *file1P = fopen(file1, "rb");
	if (!file1P) {
		printf("error opening file 1 from compare\n");
		return false;
	}
	FILE *file2P = fopen(file2, "rb");
	if (!file2P) {
		printf("error opening file 2 from compate\n");
		return false;
	}
	int c1;
	int c2;
	while ((c1 != EOF) && (c2 != EOF)) {
		c1 = fgetc(file1P);
		c2 = fgetc(file2P);
		if (c1 != c2) {
			fclose(file1P);
			fclose(file2P);
			return false;
		}
	}
	fclose(file1P);
	fclose(file2P);
	return true;
	
}
void copy_two_files(char *file1, char *file2) {
	FILE *file1P = fopen(file1, "rb");
	if (!file1P) {
		printf("error: playlist file may not exist");
		return;
	}
	FILE *file2P = fopen(file2, "wb");
	char buffer[1024];
	size_t bytes;
	while ((bytes = fread(buffer, 1, sizeof(buffer), file1P)) > 0) {
		fwrite(buffer, 1, bytes, file2P);
	}
	fclose(file1P);
	fclose(file2P);
}

