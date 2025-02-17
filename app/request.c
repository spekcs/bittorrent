#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* url_encode(unsigned char* bytes) {
    char* encoded_str = calloc(sizeof(char), SHA_DIGEST_LENGTH * 3);

    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        char temp[12];
        sprintf(temp, "%%%02x", bytes[i]);
        strcat(encoded_str, temp);
    }

    return encoded_str;
}
