#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "bencode.h"

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

char* decode_bencode(const char* bencoded_value) {
    if (is_digit(bencoded_value[0])) { 
        return decode_str(bencoded_value);
    } else if (bencoded_value[0] == 'i') {
        return decode_int(bencoded_value);
    } else {
        fprintf(stderr, "Only strings are supported at the moment\n");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
	// Disable output buffering
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

    if (argc < 3) {
        fprintf(stderr, "Usage: your_bittorrent.sh <command> <args>\n");
        return 1;
    }

    const char* command = argv[1];

    if (strcmp(command, "decode") == 0) {
    	// You can use print statements as follows for debugging, they'll be visible when running tests.
        fprintf(stderr, "Logs from your program will appear here!\n");
        const char* encoded_str = argv[2];
        char* decoded_str = decode_bencode(encoded_str);
        printf("\"%s\"\n", decoded_str);
        free(decoded_str);
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}
