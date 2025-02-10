#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "bencode.h"


void* decode_bencode(const char* bencoded_value) {
    if (is_digit(bencoded_value[0])) { 
        fprintf(stderr, "Enconding string\n");
        return decode_str(bencoded_value);
    } else if (bencoded_value[0] == 'i') {
        fprintf(stderr, "Enconding int\n");
        return decode_int(bencoded_value);
    } else if (bencoded_value[0] == 'l') {
        fprintf(stderr, "Encoding list\n");
        return decode_list(bencoded_value);
    } else {
        fprintf(stderr, "Invalid encoding.");
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
        if (is_digit(decoded_str[0]) || decoded_str[0] == '-') {
            printf("%s\n", decoded_str);
        } else if (decoded_str[0] == '[') {
            printf("%s\n", decoded_str);
        } else {
            printf("\"%s\"\n", decoded_str);
        }
        free(decoded_str);
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}
