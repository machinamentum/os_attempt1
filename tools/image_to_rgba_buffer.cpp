#include <stdio.h>
#include <assert.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdint.h>
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage:\n");
        printf("%s <image> <output.c>\n", argv[0]);
        return 0;
    }
    
    int x, y, channels;
    stbi_uc *buffer = stbi_load(argv[1], &x, &y, &channels, 4);
    
    if (!buffer) {
        printf("Could not load file '%s'\n", argv[1]);
        return -1;
    }
    
    // assert(channels == 4);
    
    FILE *file = fopen(argv[2], "wb+");
    
    if (!file) {
        printf("Could not open file '%s'\n", argv[2]);
        return -1;
    }
    
    fprintf(file, "\n#include \"kernel.h\"\n");
    fprintf(file, "\ns32 image_width = %d;\n", x);
    fprintf(file, "\ns32 image_height = %d;\n", y);
    fprintf(file, "\nu8 image_data[%d] = {",  x * y * 4);
    for (s64 i = 0; i < (x * y * 4); ++i) {
        if ((i % 16) == 0) fprintf(file, "\n    ");
        
        fprintf(file, "0x%02X,", buffer[i]);
    }
    
    fprintf(file, "\n};\n");
    fclose(file);
    
    return 0;
}