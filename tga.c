#include "tga.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#include <errno.h> 
#include <stdbool.h>

typedef struct {
    uint8_t key;
    char *value;
} targaFormat;

targaFormat TargaFormats[] = {
    { 0,  "No image data included." },
    { 1,  "Uncompressed, color-mapped images."},
    { 2,  "Uncompressed, RGB images." },
    { 3,  "Uncompressed, black and white images." },
    { 9,  "Runlength encoded color-mapped images." },
    { 10, "Runlength encoded RGB images." },
    { 11, "Compressed, black and white images." },
    { 32, "Compressed color-mapped data, using Huffman, Delta, and runlength encoding." },
    { 33, "Compressed color-mapped data, using Huffman, Delta, and runlength encoding.  4-pass quadtree-type process." }
};

int main(int argc, char *argv[])
{
    if (argc<=1) {
        puts("argc <= 1. Correct usage: ./program filename");
        return EXIT_FAILURE;
    } else if (argc >= 3) {
        puts("argc >=3 1. Correct usage: ./program filename");
        return EXIT_FAILURE; 
    }
    const char *fName = argv[1];
    printf("File to process: %s\n", fName);
    
    const int tgaV = check_tga_footer(fName);
    if (tgaV == 0) {
        puts("Your input was not a valid TGA file.");
        return EXIT_FAILURE;
    }

    TGAImage* img = (TGAImage*)malloc(sizeof(TGAImage));
    if (img == NULL) {
        fprintf(stderr, "Memory allocation failed for TGAImage.\n");
        return EXIT_FAILURE;
    }
    tga_read_file(img, fName);
    tga_free(img);
    free(img);


    return EXIT_SUCCESS;
}

int check_tga_footer(const char *file_path) {
    // Open the file in binary mode
    FILE *f = fopen(file_path, "rb");
    if (f == NULL) {
        perror("Error opening file");
        return 0;  // Return 0 for file open error
    }

    // Seek to the footer position (last 26 bytes)
    if (fseek(f, -FOOTER_SIZE, SEEK_END) != 0) {
        perror("Error seeking file");
        fclose(f);
        return 0;  // Return 0 for seek error
    }

    // Read the footer
    struct TGAFooter footer;
    if (fread(&footer, 1, FOOTER_SIZE, f) != FOOTER_SIZE) {
        perror("Error reading footer");
        fclose(f);
        return 0;  // Return 0 for read error
    }

    // Close the file
    if (fclose(f) != 0) {
        perror("Error closing file");
        return 0;  // Return 0 for close error
    }

    // Check if the signature matches the new TGA format (Version 2)
    if (memcmp(footer.signature, SIGNATURE, SIGNATURE_LEN) == 0) {
        printf("New TGA format (Version 2)\n");
        printf("Extension Area Offset: %u\n", footer.extensionAreaOffset);
        printf("Developer Directory Offset: %u\n", footer.developerDirOffset);
        return 2;  // Return 2 for new TGA format (Version 2)
    } else {
        printf("Original TGA format (Version 1 or corrupted file)\n");
        return 1;  // Return 1 for original TGA format (Version 1)
    }
}

int tga_read_file(TGAImage *img, const char *filename) {
    FILE *in = fopen(filename, "rb");
    if (!in) {
        fprintf(stderr, "Can't open file %s\n", filename);
        return 0;
    }

    TGA_Header header;
    fread(&header, sizeof(header), 1, in);

    img->width = header.width;
    img->height = header.height;
    img->bytespp = header.bitsperpixel >> 3;

    if (img->width <= 0 || img->height <= 0 || (img->bytespp != 1 && img->bytespp != 3 && img->bytespp != 4)) {
        fclose(in);
        fprintf(stderr, "Invalid width/height or bytes per pixel.\n");
        return 0;
    }

    unsigned long nbytes = img->width * img->height * img->bytespp;
    img->data = (uint8_t *)malloc(nbytes);
    if (!img->data) {
        fclose(in);
        fprintf(stderr, "Memory allocation failed.\n");
        return 0;
    }

    if (fread(img->data, 1, nbytes, in) != nbytes) {
        fclose(in);
        fprintf(stderr, "Error reading image data.\n");
        free(img->data);
        return 0;
    }
    
    tga_print_format(header.datatypecode);
    return 1;
}

//Utility fn to print tga format
void tga_print_format(uint8_t datatypecode) {
    size_t numFormats = sizeof(TargaFormats) / sizeof(TargaFormats[0]);
    for (size_t i=0; i<numFormats; i++) {
        targaFormat tf = TargaFormats[i];
        if (tf.key == datatypecode) {
            printf("TGA format %u used: %s\n", datatypecode, tf.value);
            return;
        }
    }
    printf("TGA format number %u, not found.", datatypecode);
    return;
}

// Utility Functions for TGAColor
TGAColor tga_color_init_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    TGAColor c = { .b = b, .g = g, .r = r, .a = a, .bytespp = 4 };
    return c;
}

TGAColor tga_color_init_val(uint32_t val, int bpp) {
    TGAColor c = { .val = val, .bytespp = bpp };
    return c;
}

TGAColor tga_color_init_from_raw(const uint8_t *p, int bpp) {
    TGAColor c = { .val = 0, .bytespp = bpp };
    memcpy(c.raw, p, bpp);
    return c;
}

// Memory management functions
void tga_free(TGAImage *img) {
    if (img->data) {
        free(img->data);
        img->data = NULL;
    }
}

int tga_write_file(TGAImage *img, const char *filename, int rle) {
    FILE *out = fopen(filename, "wb");
    if (!out) {
        fprintf(stderr, "Can't open file %s\n", filename);
        return 0;
    }

    TGA_Header header = {0};
    header.bitsperpixel = img->bytespp << 3;
    header.width = img->width;
    header.height = img->height;
    header.datatypecode = (img->bytespp == 1) ? 3 : 2;

    fwrite(&header, sizeof(header), 1, out);
    fwrite(img->data, 1, img->width * img->height * img->bytespp, out);

    fclose(out);
    return 1;
}

TGAColor tga_get(TGAImage *img, int x, int y) {
    if (x < 0 || y < 0 || x >= img->width || y >= img->height) {
        return tga_color_init_val(0, img->bytespp);
    }
    return tga_color_init_from_raw(img->data + (x + y * img->width) * img->bytespp, img->bytespp);
}

int tga_set(TGAImage *img, int x, int y, TGAColor c) {
    if (x < 0 || y < 0 || x >= img->width || y >= img->height) {
        return 0;
    }
    memcpy(img->data + (x + y * img->width) * img->bytespp, c.raw, img->bytespp);
    return 1;
}

void print_type(char *dataTypeCode) {
     
}

