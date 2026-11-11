#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"

/* Opens file and triggers tag reading */
Status View(char *Filename) {
    FILE *fp = fopen(Filename, "r");
    if (!fp) {
        printf("ERROR: Unable to open file %s\n", Filename);
        return e_failure;
    }
    
    Status res = ReadTags(fp);
    fclose(fp);
    return res;
}

/* Parses and displays ID3v2 frames */
Status ReadTags(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    long int FileSize = ftell(fp);
    fseek(fp, 10, SEEK_SET); /* Skip 10-byte ID3 header */

    while (ftell(fp) < FileSize) {
        char buffer[5] = {0};
        
        if (fread(buffer, 1, 4, fp) != 4) break;

        /* Check for standard supported tags */
        if (strcmp(buffer, "TIT2") == 0) printf("\nTitle   : ");
        else if (strcmp(buffer, "TPE1") == 0) printf("\nArtist  : ");
        else if (strcmp(buffer, "TALB") == 0) printf("\nAlbum   : ");
        else if (strcmp(buffer, "TYER") == 0) printf("\nYear    : ");
        else if (strcmp(buffer, "TCON") == 0) printf("\nMusic   : ");
        else if (strcmp(buffer, "COMM") == 0) printf("\nComment : ");
        else if (buffer[0] == '\0') break; /* Padding reached */

        /* Read 4-byte size of frame */
        unsigned int size;
        if (fread(&size, 4, 1, fp) != 1) break;
        size = SWAP_ENDIAN(size); 
        
        /* Check if it's a tag we want to print */
        if (strcmp(buffer, "TIT2") == 0 || strcmp(buffer, "TPE1") == 0 || 
            strcmp(buffer, "TALB") == 0 || strcmp(buffer, "TYER") == 0 || 
            strcmp(buffer, "TCON") == 0 || strcmp(buffer, "COMM") == 0) {
            
            fseek(fp, 3, SEEK_CUR); /* Skip flags + encoding byte */
            
            int data_size = size - 1;
            char *tag_data = malloc(data_size + 1);
            if (!tag_data) return e_failure;

            fread(tag_data, 1, data_size, fp);
            tag_data[data_size] = '\0';
            printf("%s", tag_data);
            
            free(tag_data); /* Prevent memory leak */
        } else {
            /* Skip unhandled tags */
            fseek(fp, 2, SEEK_CUR); /* Skip flags */
            fseek(fp, size, SEEK_CUR); /* Skip content */
        }
    }
    printf("\n");
    return e_success;
}
