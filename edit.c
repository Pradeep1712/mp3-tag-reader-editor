#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"

const char *tempFile = "temp.mp3";

/* Highly optimized chunk-copy for remaining audio data */
Status copy_remaining_data(FILE *fptr1, FILE *fptr2) {
    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), fptr1)) > 0) {
        fwrite(buffer, 1, bytes, fptr2);
    }
    return e_success;
}

/* Copies exact N bytes safely to avoid VLA/stack overflow */
Status copy_n_bytes(FILE *src, FILE *dest, int count) {
    char buffer[4096];
    while (count > 0) {
        int to_read = (count < sizeof(buffer)) ? count : sizeof(buffer);
        fread(buffer, 1, to_read, src);
        fwrite(buffer, 1, to_read, dest);
        count -= to_read;
    }
    return e_success;
}

/* Verifies if the target tag actually exists in the file before editing */
Status CheckTagAvailable(char *RecordTag, FILE *fptr1) {
    fseek(fptr1, 10, SEEK_SET); /* Skip header */
    while (1) {
        char buffer[5] = {0};
        if (fread(buffer, 1, 4, fptr1) != 4) break;
        
        if (strcmp(buffer, RecordTag) == 0) return e_success;
        if (buffer[0] == '\0') break;

        unsigned int size;
        if (fread(&size, 4, 1, fptr1) != 1) break;
        size = SWAP_ENDIAN(size);
        
        fseek(fptr1, 2 + size, SEEK_CUR); /* Skip flags and frame payload */
    }
    return e_failure;
}

/* Copies the 10-byte ID3 header intact */
Status copy_header(FILE *fptr1, FILE *fptr2) {
    char buffer[10];
    fread(buffer, 1, 10, fptr1);
    fwrite(buffer, 1, 10, fptr2);
    return e_success;
}

/* Rewrites the ID3 frames, swapping data when the target tag is reached */
Status starteditTags(FILE *fptr1, FILE *fptr2, char *RecordTag, char *NewTagData) {
    while (1) {
        char buffer[5] = {0};
        if (fread(buffer, 1, 4, fptr1) != 4) break;
        if (buffer[0] == '\0') {
            /* No more tags, copy padding/data and break */
            fseek(fptr1, -4, SEEK_CUR); 
            break;
        }

        fwrite(buffer, 1, 4, fptr2); /* Write Tag Name */

        unsigned int size_big_endian;
        fread(&size_big_endian, 4, 1, fptr1);
        unsigned int size_little = SWAP_ENDIAN(size_big_endian);

        if (strcmp(buffer, RecordTag) == 0) {
            /* Calculate and write new size */
            unsigned int new_size = strlen(NewTagData) + 1; /* +1 for encoding byte */
            unsigned int new_size_be = SWAP_ENDIAN(new_size);
            fwrite(&new_size_be, 4, 1, fptr2);
            
            /* Copy 3 bytes (2 flags + 1 encoding byte) */
            char flags_and_encoding[3];
            fread(flags_and_encoding, 1, 3, fptr1);
            fwrite(flags_and_encoding, 1, 3, fptr2);
            
            /* Write new user data */
            fwrite(NewTagData, 1, strlen(NewTagData), fptr2);
            
            /* Fast-forward original pointer past old tag data */
            fseek(fptr1, size_little - 1, SEEK_CUR);
            
            /* Copy the rest of the MP3 file */
            copy_remaining_data(fptr1, fptr2);
            return e_success;
        } else {
            /* Copy untouched tag dynamically */
            fwrite(&size_big_endian, 4, 1, fptr2);
            copy_n_bytes(fptr1, fptr2, size_little + 2);
        }
    }
    return e_success;
}

/* Orchestrates the edit process safely */
Status Proceed_edit(char *RecordTag, char *OldFileName, char *NewTagData) {
    FILE *fptr1 = fopen(OldFileName, "r");
    if (!fptr1) return e_failure;

    if (CheckTagAvailable(RecordTag, fptr1) == e_failure) {
        printf("ERROR: Tag %s does not exist in file.\n", RecordTag);
        fclose(fptr1);
        return e_failure;
    }

    FILE *fptr2 = fopen(tempFile, "w");
    if (!fptr2) {
        fclose(fptr1);
        return e_failure;
    }

    rewind(fptr1);
    copy_header(fptr1, fptr2);
    starteditTags(fptr1, fptr2, RecordTag, NewTagData);

    /* Files MUST be closed before rename/delete operations */
    fclose(fptr1);
    fclose(fptr2);

    remove(OldFileName);
    rename(tempFile, OldFileName);

    printf("Tag data edited successfully.\n");
    return e_success;
}
