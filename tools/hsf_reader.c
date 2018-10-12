#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define HSF_IMPLEMENTATION
#define HSF_ALLOC malloc
#define HSF_FREE  free
#define HSF_INCLUDE_STDIO
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

void visitor_callback(Hsf_Context *HSF, const char *DirName, Hsf_Directory_Entry *entry, void *UserPayload) {
	if (entry->filename[0] == 0) return; // "."
	if (entry->filename[0] == '\1') return; // ".."
    if (entry->file_flags & HSF_FILE_FLAG_IS_DIR) {
        printf("%s%.*s: <dir>\n", DirName, __hsf_get_filename_length(entry), &entry->filename[0]);
    } else {
        printf("%s%.*s: %u bytes\n", DirName, __hsf_get_filename_length(entry), &entry->filename[0], entry->data_length_le);
    }
    
    /*
 if (entry->file_flags & HSF_FILE_FLAG_IS_DIR) {
  char *FileName = (char *)malloc(entry->filename_length+1);
  memcpy(FileName, &entry->filename[0], entry->filename_length);
  FileName[entry->filename_length] = 0;
  char *Dir = concatenate(DirName, FileName);
  printf("Recursing: '%s'\n", Dir);
  hsf_visit_directory(HSF, Dir, visitor_callback, 0);
  
  free(FileName);
  free(Dir);
 }
 */
}

int main(int argc, char **argv) {
	if (argc < 3) {
		printf("%s [command] [ISO FILE] <FILE PATH IN ISO>\n", argv[0]);
		return 0;
	}
    
    Hsf_Context ctx;
    
    if (strcmp(argv[1], "create") == 0) {
        hsf_create_from_fopen(&ctx, argv[2]);
        ctx.io_mode = HSF_IO_READ_WRITE;
        hsf_format_image(&ctx, "Test Vol", 5120 /*10MB*/);
        hsf_destruct_with_fclose(&ctx);
    } else if (strcmp(argv[1], "ls") == 0) {
        char *path;
        if (argc < 4) {
            path = "/";
        } else {
            path = argv[3];
        }
        hsf_create_from_fopen(&ctx, argv[2]);
        if (!ctx.pvd) {
            printf("Couldnt not open file: '%s'\n", argv[2]);
            return -1;
        }
        hsf_visit_directory(&ctx, path, visitor_callback, 0);
        hsf_destruct_with_fclose(&ctx);
    } else if (strcmp(argv[1], "cat") == 0) {
        if (argc < 4) return -1;
        
        hsf_create_from_fopen(&ctx, argv[2]);
        if (!ctx.pvd) {
            printf("Couldnt not open file: '%s'\n", argv[2]);
            return -1;
        }
        
        Hsf_File *file = hsf_file_open(&ctx, argv[3]);
        
        if (!file) {
            printf("Couldnt open: %s\n", argv[2]);
            return -1;
        }
        
        hsf_file_seek(file, 0, HSF_SEEK_END);
        u32 length = hsf_file_tell(file);
        hsf_file_seek(file, 0, HSF_SEEK_SET);
        
        char *buffer = (char *)malloc(length+1);
        
        hsf_file_read(buffer, length, file);
        buffer[length] = 0;
        
        printf("%s", buffer);
        hsf_file_close(file);
        hsf_destruct_with_fclose(&ctx);
    }
    
    /*
    
    HsfVisitDirectory(&handle, "/", visitor_callback, 0);
    
    if (argc >= 3) {
    
    }
    */
    
    // HsfClose(&handle);
    return 0;
}
