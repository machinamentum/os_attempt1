#include <stdio.h>
#include <stdlib.h>

#define HSF_IMPLEMENTATION
#define HSF_ALLOC malloc
#define HSF_FREE  free
#include "../include/iso9660.h"

#include <string.h>

char *concatenate(const char *str0, const char *str1) {
	u32 l0 = strlen(str0);
	u32 l1 = strlen(str1);

	char *out = (char *)malloc(l0+l1+1);
	memcpy(out, str0, l0);
	memcpy(out+l0, str1, l1);
	out[l0+l1] = 0;
	return out;
}

void visitor_callback(hsf *HSF, const char *DirName, hsf_directory_entry *entry, void *UserPayload) {
	printf("%s: FileName: %d:'%.*s'\n", DirName, entry->FileNameLength, entry->FileNameLength, &entry->FileName[0]);

	if (entry->FileName[0] == 0) return; // "."
	if (entry->FileName[0] == '\1') return; // ".."

	if (entry->FileFlags & HSF_FILE_FLAG_IS_DIR) {
		char *FileName = (char *)malloc(entry->FileNameLength+1);
		memcpy(FileName, &entry->FileName[0], entry->FileNameLength);
		FileName[entry->FileNameLength] = 0;
		char *Dir = concatenate(DirName, FileName);
		printf("Recursing: '%s'\n", Dir);
		HsfVisitDirectory(HSF, Dir, visitor_callback, 0);

		free(FileName);
		free(Dir);
	}
}

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("%s [ISO FILE] <FILE PATH IN ISO>\n", argv[0]);
		return 0;
	}

	hsf handle;
	HsfOpen(&handle, argv[1]);

	HsfVisitDirectory(&handle, "/", visitor_callback, 0);

	if (argc >= 3) {
		hsf_file *File = HsfFileOpen(&handle, argv[2]);

		if (!File) {
			printf("Couldnt open: %s\n", argv[2]);
			return -1;
		} else {
			printf("Found file: %s\n\n", argv[2]);
		}

		HsfFileSeek(File, 0, HSF_SEEK_END);
		u32 length = HsfFileTell(File);
		HsfFileSeek(File, 0, HSF_SEEK_SET);

		char *buffer = (char *)malloc(length+1);

		HsfFileRead(buffer, 1, length, File);
		buffer[length] = 0;

		printf("CONTENTS:\n%s\n", buffer);
	}

	HsfClose(&handle);
	return 0;
}