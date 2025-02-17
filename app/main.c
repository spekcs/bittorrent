#include <stddef.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "bencode.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include "file.h"
#include "request.h"

#define UNIQUE_CLIENT_ID "geZCJhqS1MliO2Ju9O9S"
#define LISTENING_PORT 6881

typedef struct {
    char* string;
    size_t size;
} response_t;

size_t write_chunk(void* data, size_t size, size_t nmemb, void* userdata);

static void handle_decode(const char* encoded_str) {
    d_res_t* decoded_str = decode(encoded_str);
    if (decoded_str == NULL) {
        fprintf(stderr, "Unable to decode string, something went wrong");
        exit(1);
    }
    print_decoded_string(decoded_str);
    d_res_free(decoded_str);
}



static d_res_t* get_info_dict(d_res_t* decoded_obj) {
    d_res_t* info_dict;

    array_list_t* keys = decoded_obj->data.v_dict->keys;
    array_list_t* values = decoded_obj->data.v_dict->values;

    for (int i = 0; i < keys->len; i++) {
        if (strcmp(keys->data[i]->data.v_str, "info") == 0) {
            info_dict = values->data[i];
        }
    }

    return info_dict;
}

static char* get_tracker_url(d_res_t* decoded_obj) {
    array_list_t* keys = decoded_obj->data.v_dict->keys;
    array_list_t* values = decoded_obj->data.v_dict->values;

    for (int i = 0; i < keys->len; i++) {
        if (strcmp(keys->data[i]->data.v_str, "announce") == 0) {
            return values->data[i]->data.v_str;
        }
    }

    fprintf(stderr, "Tracker URL not found");
    exit(1);
}

static unsigned char* get_info_dict_hash(const char* filename) {
    long file_size = get_file_length(filename);
    char* buf = get_file_contents(filename);

    long infodict_offset = strstr(buf, "info") - buf + 4;
    char encoded_info_dict[file_size - infodict_offset - 1]; 
    for (int i = 0; i < file_size - infodict_offset - 1; i++) {
        encoded_info_dict[i] = *(buf + infodict_offset + i);
    }
    
    unsigned char* hash = malloc(SHA_DIGEST_LENGTH);
    SHA1((unsigned char*)encoded_info_dict, file_size - infodict_offset - 1, hash);

    return hash;
}

static void handle_info(const char* filename) {
    d_res_t* decoded_string = decode_from_file(filename);

    array_list_t* keys = decoded_string->data.v_dict->keys;
    array_list_t* values = decoded_string->data.v_dict->values;

    d_res_t* info_dict = get_info_dict(decoded_string);
    long piece_length;

    char* tracker_url = get_tracker_url(decoded_string);
    printf("Tracker URL: %s\n", tracker_url);


    for (int i = 0; i < keys->len; i++) {
        if (strcmp(keys->data[i]->data.v_str, "info") == 0) {
            for (int j = 0; j < values->data[i]->data.v_dict->keys->len; j++) {
                if (strcmp(values->data[i]->data.v_dict->keys->data[j]->data.v_str, "length") == 0) {
                    printf("Length: %ld\n", values->data[i]->data.v_dict->values->data[j]->data.v_long);
                }

                if (strcmp(values->data[i]->data.v_dict->keys->data[j]->data.v_str, "piece length") == 0) {
                    piece_length = values->data[i]->data.v_dict->values->data[j]->data.v_long;
                }
            }
        }
    }

    d_res_free(decoded_string);

    /*---- CALCULATING SHA1 OF INFO DICT ----*/
    unsigned char* hash = get_info_dict_hash(filename);
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
    free(hash);

    /*---- PRINTING PIECES ----*/

    long file_size = get_file_length(filename);
    char* buf = get_file_contents(filename);

    printf("Piece Length: %ld\n", piece_length);

    printf("Piece Hashes:\n");
    char* pieces_index = strstr(buf, "pieces");
    char* start_index = strchr(pieces_index, ':');
    long pieces_offset = start_index - buf + 1;
    while (file_size - pieces_offset > SHA_DIGEST_LENGTH) {
        for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
            printf("%02x", (unsigned char)(*(buf + pieces_offset + i)));
        }
        pieces_offset += 20;
        printf("\n");
    }

    free(buf);
}



static void handle_peers(const char* filename) {
    d_res_t* decoded_string = decode_from_file(filename);

    array_list_t* keys = decoded_string->data.v_dict->keys;
    array_list_t* values = decoded_string->data.v_dict->values;

    char* tracker_url = get_tracker_url(decoded_string);


    unsigned char* info_dict_hash = get_info_dict_hash(filename);
    char* encoded_info_dict_hash = url_encode(info_dict_hash);

    char* params = malloc(get_file_length(filename));
    sprintf(params, "?info_hash=%s&peer_id=%s&port=%d&uploaded=0&downloaded=0&left=%ld&compact=1", encoded_info_dict_hash,
            UNIQUE_CLIENT_ID,
            LISTENING_PORT,
            get_file_length(filename));


    char* tracker_url_with_params = strcat(tracker_url, params);

    fprintf(stderr, "URL --> %s\n", tracker_url_with_params);

    free(params);
    free(encoded_info_dict_hash);

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

    curl_easy_setopt(curl, CURLOPT_URL, tracker_url_with_params);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_chunk);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void* ) &response);

    result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        fprintf(stderr, "Error: %s\n", curl_easy_strerror(result));
        exit(-1);
    }

    printf("%s\n", response.string);

    curl_easy_cleanup(curl);
    free(response.string);
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
        handle_decode(argv[2]);
    } else if (strcmp(command, "info") == 0) {
        handle_info(argv[2]);
    } else if (strcmp(command, "encode") == 0) {

        const char* encoded_str = argv[2];

        d_res_t* decoded_str = decode(encoded_str);
        
        char* result = encode(decoded_str) ;
        printf("%s", result);

        free(result);
        d_res_free(decoded_str);

    } else if (strcmp(command, "peers") == 0) {
        handle_peers(argv[2]);
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}
