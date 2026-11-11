#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>

/* Functional macro to convert Big Endian to Little Endian (and vice versa) */
#define SWAP_ENDIAN(x) (((x >> 24) & 0x000000FF) | \
                        ((x >> 8)  & 0x0000FF00) | \
                        ((x << 8)  & 0x00FF0000) | \
                        ((x << 24) & 0xFF000000))

typedef enum Successreturn {
    e_success,
    e_failure
} Status;

typedef struct MP3Reader {
    char *Tagname;
    char *Artistname;
    char *Album;
    char *Year;
    char *Music;
    char *Comment;
} MP3TagReader;

/* Validation Prototypes */
Status read_and_validate_args(int argc, char *argv[]);
Status read_and_validate_edit_arguments(int argc, char *argv[], char **RecordTag);

/* View Prototypes */
Status View(char *Filename);
Status ReadTags(FILE *fp);

/* Edit Prototypes */
Status Proceed_edit(char *RecordTag, char *OldFileName, char *NewTagData);
Status CheckTagAvailable(char *RecordTag, FILE *fptr1);
Status copy_header(FILE *fptr1, FILE *fptr2);
Status starteditTags(FILE *fptr1, FILE *fptr2, char *RecordTag, char *NewTagData);
Status copy_remaining_data(FILE *fptr1, FILE *fptr2);
Status copy_n_bytes(FILE *src, FILE *dest, int count);

#endif
