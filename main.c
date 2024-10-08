#include <stdlib.h>
#include <stdio.h>
#include <errno.h> 
#include <stdbool.h>
#include <string.h>

#define FOOTER_SIZE 26
#define SIGNATURE_LEN 16
#define SIGNATURE "TRUEVISION-XFILE"

#pragma pack(push, 1) // prevents compiler padding in struct
struct TGAFooter {
    unsigned int extensionAreaOffset;    // Bytes 0-3
    unsigned int developerDirOffset;     // Bytes 4-7
    char signature[SIGNATURE_LEN];       // Bytes 8-23
    char dot;                            // Byte 24 
    char nullTerminator;                 // Byte 25
};
#pragma pack(pop)

int check_tga_footer(const char *file_path); 

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
    
    int sneed = check_tga_footer(fName);

    return EXIT_SUCCESS;
}

int check_tga_footer(const char *file_path) {
    FILE *f = fopen(file_path, "rb");
    if (f == NULL) {
        perror("Error opening f");
        return 0;
    }

    // seek to last 26 bytes
    if (fseek(f, -FOOTER_SIZE, SEEK_END) != 0) {
        perror("Error seeking f");
        fclose(f);
        return 0; 
    }

    struct TGAFooter footer;
    if (fread(&footer, 1, FOOTER_SIZE, f) != FOOTER_SIZE) {
        perror("Error reading footer");
        fclose(f);
        return 0;
    }

    if (fclose(f) != 0) { //Errors: EBADF or errno for others
        perror("Error closing file");
        printf("Value of errno: %d\n", errno);
        return 0;
    }

    // signature = "TRUEVISION-XFILE" for new TGA format
    if (memcmp(footer.signature, SIGNATURE, SIGNATURE_LEN) == 0) {
        printf("New TGA format \n");
        printf("Extension Area Offset: %u\n", footer.extensionAreaOffset);
        printf("Developer Directory Offset: %u\n", footer.developerDirOffset);
        return 2; 
    } else {
        printf("Original TGA or files fucked\n");
        return 1;
    }
}

char *read_file(const char* fName){
    FILE *f = fopen(fName, "r"); //
    printf("Value of errno: %d\n", errno);
    if (f == NULL) {
        puts("Error, unable to open the file");
        fclose(f); //necessary?
        exit(EXIT_FAILURE);
    }

    if (fclose(f) != 0) { //Errors: EBADF or errno for others
        puts("Error closing file");
        printf("Value of errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    return "sneeds";
}
