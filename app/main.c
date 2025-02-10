#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "bencode.h"


d_res_t* decode_bencode(const char* bencoded_value) {
    if (is_digit(bencoded_value[0])) { 
        fprintf(stderr, "Enconding string\n");
        return decode_str(bencoded_value);
    } else if (bencoded_value[0] == 'i') {
        fprintf(stderr, "Enconding int\n");
        return decode_int(bencoded_value);
    } else if (bencoded_value[0] == 'l') {
        fprintf(stderr, "Encoding list\n");
        return decode_list(bencoded_value);
    } 
    else {
        fprintf(stderr, "Invalid encoding.");
        exit(1);
    }
}

void print_list(d_res_t* decoded_str) {
    printf("[");
    for (int i = 0; i < decoded_str->data.v_list->len; i++) { 
        switch (decoded_str->data.v_list->data[i]->type) {
            case INT_TYPE:
                printf("%d", decoded_str->data.v_list->data[i]->data.v_int);  
                break;
            case LIST_TYPE:
                print_list(decoded_str->data.v_list->data[i]);
                break;
        }
    }
    printf("]");
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
        const char* encoded_str = argv[2];
        d_res_t* decoded_str = decode_bencode(encoded_str);
        switch (decoded_str->type) {
            case STRING_TYPE:
                printf("\"%s\"\n", decoded_str->data.v_str);
                break;
            case INT_TYPE:
                printf("%d\n", decoded_str->data.v_int);
                break;
            case LIST_TYPE:
                print_list(decoded_str);           
                printf("\n");
        }
        d_res_free(decoded_str);
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}
