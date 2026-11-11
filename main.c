#include <stdio.h>
#include <string.h>
#include "types.h"

/* Validates CLI arguments for View mode */
Status read_and_validate_args(int argc, char *argv[]) {
    if (argc != 3) return e_failure;
    if (strstr(argv[2], ".mp3") == NULL) return e_failure;
    return e_success;
}

/* Validates CLI arguments for Edit mode and maps user flag to ID3 Tag */
Status read_and_validate_edit_arguments(int argc, char *argv[], char **RecordTag) {
    if (argc != 5) return e_failure;
    if (strstr(argv[4], ".mp3") == NULL) return e_failure;

    char *TagFlags[6] = {"-t", "-T", "-A", "-Y", "-c", "-C"};
    char *TagNames[6] = {"TIT2", "TPE1", "TALB", "TYER", "TCON", "COMM"};
    
    for (int i = 0; i < 6; i++) {
        if (strcmp(argv[2], TagFlags[i]) == 0) {
            *RecordTag = TagNames[i];
            return e_success;
        }
    }
    return e_failure;
}

/* Main execution routing based on CLI flags */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("ERROR: Missing arguments. Use -h for help.\n");
        return 1;
    }

    if (strcmp(argv[1], "-v") == 0) {
        if (read_and_validate_args(argc, argv) == e_success) {
            View(argv[2]);
        } else {
            printf("ERROR: Invalid view arguments.\n");
        }
    } 
    else if (strcmp(argv[1], "-e") == 0) {
        char *RecordTag = NULL;
        if (read_and_validate_edit_arguments(argc, argv, &RecordTag) == e_success) {
            Proceed_edit(RecordTag, argv[4], argv[3]);
        } else {
            printf("ERROR: Invalid edit arguments.\n");
        }
    } 
    else if (strcmp(argv[1], "-h") == 0) {
        printf("--------------Help menu--------------\n");
        printf("View: ./a.out -v <file.mp3>\n");
        printf("Edit: ./a.out -e <Tag> <newdata> <file.mp3>\n");
        printf("Tags: -t Title, -T Artist, -A Album, -Y Year, -c Content, -C Comment\n");
    } 
    else {
        printf("ERROR: Invalid mode entered.\n");
    }

    return 0;
}
