#include <stddef.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "bencode.h"
#include "sha1.h"



void print_int(d_res_t* decoded_str, int iter) {
    printf("%ld", decoded_str->data.v_list->data[iter]->data.v_long);  
}

void print_str(d_res_t* decoded_str, int iter) {
    printf("\"%s\"", decoded_str->data.v_list->data[iter]->data.v_str);
}

void print_dict(d_res_t*);

void print_list(d_res_t* decoded_str) {
    printf("[");
    for (int i = 0; i < decoded_str->data.v_list->len; i++) { 
        switch (decoded_str->data.v_list->data[i]->type) {
            case LONG_TYPE:
                print_int(decoded_str, i);
                break;
            case LIST_TYPE:
                print_list(decoded_str->data.v_list->data[i]);
                break;
            case STRING_TYPE:
                print_str(decoded_str, i);
                break;
            case DICT_TYPE:
                print_dict(decoded_str->data.v_list->data[i]); 
        }
        
        if (i < decoded_str->data.v_list->len - 1) {
            printf(",");
        }
    }
    printf("]");
}

void print_dict(d_res_t* decoded_str) {
    array_list_t* keys = decoded_str->data.v_dict->keys;
    array_list_t* values = decoded_str->data.v_dict->values;

    bool is_key = 1;
    printf("{");
    for (int i = 0; i < keys->len;) { 
        if (is_key) {
            switch (keys->data[i]->type) {
                case LONG_TYPE:
                    printf("%ld", keys->data[i]->data.v_long);
                    break;
                case LIST_TYPE:
                    print_list(keys->data[i]);
                    break;
                case STRING_TYPE:
                    printf("\"%s\"", keys->data[i]->data.v_str);
                    break;
                case DICT_TYPE:
                    print_dict(keys->data[i]);
                    break;
            } 
            printf(":");
            is_key = false;
        } else {
            switch (values->data[i]->type) {
                case LONG_TYPE:
                    printf("%ld", values->data[i]->data.v_long);
                    break;
                case LIST_TYPE:
                    print_list(values->data[i]);
                    break;
                case STRING_TYPE:
                    printf("\"%s\"", values->data[i]->data.v_str);
                    break;
                case DICT_TYPE:
                    print_dict(values->data[i]);
                    break;
            }
            if (i < keys->len - 1) {
                printf(",");
            }
            i++;
            is_key = true;
        }
    }

    printf("}");
}

void print_decoded_string(d_res_t* decoded_str) {
    switch (decoded_str->type) {
        case STRING_TYPE:
            printf("\"%s\"\n", decoded_str->data.v_str);
            break;
        case LONG_TYPE:
            printf("%ld\n", decoded_str->data.v_long);
            break;
        case LIST_TYPE:
            print_list(decoded_str);           
            printf("\n");
            break;
        case DICT_TYPE:
            print_dict(decoded_str);
            printf("\n");
            break;
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

    /* ---- DECODE ----*/
    if (strcmp(command, "decode") == 0) {
        const char* encoded_str = argv[2];

        d_res_t* decoded_str = decode(encoded_str);
        
        print_decoded_string(decoded_str);
        d_res_free(decoded_str);

    /* ---- INFO ----*/
    } else if (strcmp(command, "info") == 0) {
        const char* filename = argv[2];
        FILE* fp = fopen(filename, "rb");
        if (fp == NULL) {
            fprintf(stderr, "No such file: %s\n", filename);
            exit(1);
        }

        
        fseek(fp, 0L, SEEK_END);
        int file_size = ftell(fp);
        rewind(fp);

        char buf[file_size];

        for (int i = 0; i < file_size; i++) {
            buf[i] = getc(fp);
        }

        int* current_index = malloc(sizeof(int));
        *current_index = 1;
        d_res_t* decoded_string = decode_dict((char*)buf, current_index, file_size);

        free(current_index);

        array_list_t* keys = decoded_string->data.v_dict->keys;
        array_list_t* values = decoded_string->data.v_dict->values;

        d_res_t* info_dict;

        for (int i = 0; i < keys->len; i++) {
            if (strcmp(keys->data[i]->data.v_str, "announce") == 0) {
                printf("Tracker URL: %s\n", values->data[i]->data.v_str);
            }

            if (strcmp(keys->data[i]->data.v_str, "info") == 0) {
                info_dict = values->data[i];
                for (int j = 0; j < values->data[i]->data.v_dict->keys->len; j++) {
                    if (strcmp(values->data[i]->data.v_dict->keys->data[j]->data.v_str, "length") == 0) {
                        printf("Length: %ld\n", values->data[i]->data.v_dict->values->data[j]->data.v_long);
                    }
                }
            }
        }


        long infodict_offset = strstr(buf, "info") - buf + 4;
        char encoded_info_dict[file_size - infodict_offset - 1]; 
        for (int i = 0; i < file_size - infodict_offset - 1; i++) {
            encoded_info_dict[i] = *(buf + infodict_offset + i);
        }
        printf("Encoded info dict: %s\n", encoded_info_dict);
        
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1((unsigned char*)encoded_info_dict, file_size - infodict_offset - 1, hash);
        printf("Offset char: %c\n", buf[infodict_offset + 1]);
        
        printf("Info Hash: ");
        for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
            printf("%02x", hash[i]);
        }
        printf("\n");

        d_res_free(decoded_string);
        fclose(fp);
    } else if (strcmp(command, "encode") == 0) {

        const char* encoded_str = argv[2];

        d_res_t* decoded_str = decode(encoded_str);
        
        char* result = encode(decoded_str) ;
        printf("%s", result);

        free(result);
        d_res_free(decoded_str);

    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}
