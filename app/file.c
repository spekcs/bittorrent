#include <stdio.h>
#include <stdlib.h>

char* get_file_contents(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "No such file: %s\n", filename);
        exit(1);
    }

    fseek(fp, 0L, SEEK_END);
    int file_size = ftell(fp);
    rewind(fp);

    char* buf = malloc(sizeof(char) * file_size + 1);
    buf[file_size] = '\0';

    for (int i = 0; i < file_size; i++) {
        buf[i] = getc(fp);
    }

    fclose(fp);
    return buf;
}

long get_file_length(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "No such file: %s\n", filename);
        exit(1);
    }

    fseek(fp, 0L, SEEK_END);
    long file_size = ftell(fp);
    fclose(fp);
    return file_size;
}
