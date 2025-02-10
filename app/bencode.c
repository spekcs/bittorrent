#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

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
        int len = strlen(bencoded_value) - 2;
        
        if (len > 1 && bencoded_value[1] == '0') {
            fprintf(stderr, "Encoded integer can't have leading zeroes.");
            exit(1);
        }

        char* decoded_str = (char*)malloc(len + 1);
        strncpy(decoded_str, &bencoded_value[1], len);
        decoded_str[len] = '\0';
        return decoded_str;
    } else {
        fprintf(stderr, "Invalid encoded value: %s\n", bencoded_value);
        exit(1);
    }
}

char* decode_list(const char* bencoded_value) {
    int len = strlen(bencoded_value);
    char* result = (char*)malloc(len);
    int curr = 1;

    if (bencoded_value[1] == 'e') {
        strcat(result, "[]\n\0");
        return result;
    }

    strcat(result, "[");

    while (curr < len - 1) {
        if (is_digit(*(bencoded_value + curr))) {
            strcat(result, "\"");
            const char* colon_index = strchr((bencoded_value + curr), ':');
            int colon_relative_index = colon_index - (bencoded_value + curr);
            fprintf(stderr, "Colon: %d\n", colon_relative_index);
            char* substr_len = malloc(len);
            strncpy(substr_len, (bencoded_value + curr), colon_relative_index);
            int substring_length_bytes = atoi(substr_len);
            fprintf(stderr, "Substr len in bytes: %d\n", substring_length_bytes);

            free(substr_len);

            char* substr_encoded = malloc(substring_length_bytes);
            strncpy(substr_encoded, (bencoded_value + curr), substring_length_bytes + colon_relative_index + 1);
            char* substr = decode_str(substr_encoded);
            strcat(result, substr);
            free(substr_encoded);
            free(substr);
            strcat(result, "\"");
            curr += substring_length_bytes;
            curr += colon_relative_index;
        } else if (*(bencoded_value + curr) == 'i') {
            int substr_len = strchr((bencoded_value + curr), 'e') - (bencoded_value + curr) + 1;
            char* encoded_str = malloc(substr_len + 1);
            encoded_str[substr_len] = '\0';
            strncpy(encoded_str, (bencoded_value + curr), substr_len);
            char* substr = decode_int(encoded_str);
            strcat(result, substr);
            curr += substr_len - 1;
            free(encoded_str);
            free(substr);
        }
        curr++;
        if (curr < len - 1) {
            strcat(result, ",");
        } else {
            strcat(result, "]\0");
        }
    }
    return result;
}
