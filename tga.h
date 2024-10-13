#ifndef __TGA_H__
#define __TGA_H__

#include <stdio.h>
#include <stdint.h>

#define HEADER_SIZE 18
#define FOOTER_SIZE 26
#define SIGNATURE_LEN 16
#define SIGNATURE "TRUEVISION-XFILE"

#pragma pack(push,1)
typedef struct {
    uint8_t idlength;
    uint8_t colormaptype;
    uint8_t datatypecode;
    int16_t colormaporigin;
    int16_t colormaplength;
    uint8_t colormapdepth;
    int16_t x_origin;
    int16_t y_origin;
    int16_t width;
    int16_t height;
    uint8_t bitsperpixel;
    uint8_t imagedescriptor;
} TGA_Header;
#pragma pack(pop)

#pragma pack(push, 1) // prevents compiler padding in struct
struct TGAFooter {
    uint32_t extensionAreaOffset;    // Bytes 0-3
    uint32_t developerDirOffset;     // Bytes 4-7
    char signature[SIGNATURE_LEN];       // Bytes 8-23
    char dot;                            // Byte 24 
    char nullTerminator;                 // Byte 25
};
#pragma pack(pop)

typedef struct {
    union {
        struct {
            uint8_t b, g, r, a;
        };
        uint8_t raw[4];
        uint32_t val;
    };
    int bytespp;
} TGAColor;

#pragma pack(push,1) 
typedef struct {
    uint8_t *data;
    int width;
    int height;
    int bytespp;
} TGAImage;
#pragma pack(pop)

// Utility functions for TGAColor
TGAColor tga_color_init_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
TGAColor tga_color_init_val(uint32_t val, int bpp);
TGAColor tga_color_init_from_raw(const uint8_t *p, int bpp);

// Functions for TGAImage operations
int check_tga_footer(const char *file_path);
TGAColor tga_color_init_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
TGAColor tga_color_init_val(uint32_t val, int bpp);
TGAColor tga_color_init_from_raw(const uint8_t *p, int bpp);
void tga_free(TGAImage *img);
int tga_read_file(TGAImage *img, const char *filename);
int tga_write_file(TGAImage *img, const char *filename, int rle);
TGAColor tga_get(TGAImage *img, int x, int y);
int tga_set(TGAImage *img, int x, int y, TGAColor c);
void tga_print_format(uint8_t datatypecode);

#endif

