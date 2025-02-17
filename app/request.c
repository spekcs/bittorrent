#include <curl/curl.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* string;
    size_t size;
} response_t;


char* url_encode(unsigned char* bytes) {
    char* encoded_str = calloc(sizeof(char), SHA_DIGEST_LENGTH * 3);

    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        char temp[12];
        sprintf(temp, "%%%02x", bytes[i]);
        strcat(encoded_str, temp);
    }

    return encoded_str;
}

size_t write_chunk(void* data, size_t size, size_t nmemb, void* userdata);

char* get_request(char* url) {
    CURL* curl;
    CURLcode result;

    curl = curl_easy_init();

    if (curl == NULL) {
        fprintf(stderr, "HTTP request failed\n");
        exit(1);
    }


    response_t response;
    response.string = malloc(1);
    response.size = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_chunk);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void* ) &response);

    result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        fprintf(stderr, "Error: %s\n", curl_easy_strerror(result));
        exit(-1);
    }

    curl_easy_cleanup(curl);

    return response.string;
}

size_t write_chunk(void* data, size_t size, size_t nmemb, void* userdata) {
    size_t real_size = size * nmemb;

    response_t* response = (response_t * ) userdata;

    char *ptr = realloc(response->string, response->size + real_size + 1);

    if (ptr == NULL) {
        return CURL_WRITEFUNC_ERROR;
    }

    response->string = ptr;
    memcpy(&(response->string[response->size]), data, real_size);
    response->size += real_size;
    response->string[response->size] = '\0';

    return real_size;
}
