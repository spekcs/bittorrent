#include "bencode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* decode_str(const char* bencoded_value) {
        int length = atoi(bencoded_value); 
        const char* colon_index = strchr(bencoded_value, ':');
        if (colon_index != NULL) {
            const char* start = colon_index + 1;
            char* decoded_str = (char*)malloc(length + 1);
            strncpy(decoded_str, start, length);
            decoded_str[length] = '\0';
            return decoded_str;
        } else {
            fprintf(stderr, "Invalid encoded value: %s\n", bencoded_value);
            exit(1);
        }
 }

char* decode_int(const char* bencoded_value) {
        const char* end_index = strchr(bencoded_value, 'e');
        if (end_index != NULL) {
            int len = strlen(bencoded_value);
            
            if (len > 3 && bencoded_value[1] == '0') {
            fprintf(stderr, "Encoded integer can't have leading zeroes.");
            exit(1);
        }

            char* decoded_str = (char*)malloc(len);
            strncpy(decoded_str, &bencoded_value[1], len-2);
            return decoded_str;
        } else {
            fprintf(stderr, "Invalid encoded value: %s\n", bencoded_value);
            exit(1);
        }
}
