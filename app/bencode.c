#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bencode.h"

#define LONG_MAX_DIGITS 19

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}



static d_res_t* decode_str(const char* bencoded_value, int* current_index) {
    int length = atoi(bencoded_value);
    const char* colon_index = strchr(bencoded_value, ':');
    if (colon_index != NULL) {
        const char* start = colon_index + 1;
        char* decoded_str = malloc(length + 1);
        //strncpy(decoded_str, start, length);
        for (int i = 0; i < length; i++) {
            decoded_str[i] = *(start+i);
        }

        decoded_str[length] = '\0';
        d_res_t* result = malloc(sizeof(d_res_t));
        result->type = STRING_TYPE;
        result->data.v_str = decoded_str;

        *current_index += (colon_index - bencoded_value) + length;

        return result;
    } else {
        fprintf(stderr, "Invalid encoded value: %s\n", bencoded_value);
        exit(1);
    }
 }

static d_res_t* decode_int(const char* bencoded_value, int* current_index) {
    const char* end_index = strchr(bencoded_value, 'e');
    if (end_index != NULL) {
        int len = strlen(bencoded_value) - 2;
        
        if (len > 1 && bencoded_value[1] == '0') {
            fprintf(stderr, "Encoded integer can't have leading zeroes.\n");
            exit(1);
        }

        char* decoded_str = (char*)malloc(len + 1);
        decoded_str[len] = '\0';
        strncpy(decoded_str, &bencoded_value[1], len);
        long decoded_int = atol(decoded_str);
        free(decoded_str);
        d_res_t* result = malloc(sizeof(d_res_t));
        result->type = LONG_TYPE;
        result->data.v_long = decoded_int;

        *current_index += end_index - bencoded_value;

        return result;
    } else {
        fprintf(stderr, "Invalid encoded value: %s\n", bencoded_value);
        exit(1);
    }
}


static d_res_t* decode_list(const char* bencoded_value, int* current_index) {
    int length = strlen(bencoded_value);

    array_list_t* res_list = malloc(sizeof(array_list_t));
    d_res_t** list_items = malloc(sizeof(d_res_t*) * length);
    res_list->data = list_items;
    res_list->len = 0;


    while (*current_index < length) {
        if (bencoded_value[*current_index] == 'l') {
            (*current_index)++;
            d_res_t* result = decode_list(bencoded_value, current_index);
            res_list->data[res_list->len] = result;
            res_list->len++;
        } 
        else if (bencoded_value[*current_index] == 'i') {
            d_res_t* result = decode_int(bencoded_value + *current_index, current_index);

            res_list->data[res_list->len] = result;
            res_list->len++;

        } else if (bencoded_value[*current_index] == 'd') {
            (*current_index)++;
            d_res_t* result = decode_dict(bencoded_value, current_index, length);
            res_list->data[res_list->len] = result;
            res_list->len++;

        } else if (is_digit(bencoded_value[*current_index])){
            d_res_t* result = decode_str(bencoded_value + *current_index, current_index);

            res_list->data[res_list->len] = result;
            res_list->len++;

        } else if (bencoded_value[*current_index == 'e']) {
            d_res_t* result = malloc(sizeof(d_res_t));
            result->type = LIST_TYPE;
            result->data.v_list = res_list;
            return result;

        }
        (*current_index)++;
    }
    fprintf(stderr, "Invalid list encoding, didn't end");
    exit(1);
}

d_res_t* decode_dict(const char* bencoded_value, int* current_index, long length) {
    dict_t* res_dict = malloc(sizeof(dict_t));
    array_list_t* values = malloc(sizeof(array_list_t));
    d_res_t** values_items = malloc(sizeof(d_res_t*) * length);
    values->data = values_items;
    array_list_t* keys = malloc(sizeof(array_list_t));
    d_res_t** keys_items = malloc(sizeof(d_res_t*) * length);
    keys->data = keys_items;
    values->len = 0;
    keys->len = 0;
    res_dict->values = values;
    res_dict->keys = keys;


    bool is_key = true;

    while (*current_index < length) {
        if (bencoded_value[*current_index] == 'd') {
            fprintf(stderr, "Decoding dict\n");
            (*current_index)++;
            d_res_t* result = decode_dict(bencoded_value, current_index, length);
            if (result == NULL) {
                fprintf(stderr, "Failed to decode dict: %s", (bencoded_value + *current_index - 1));
                exit(1);
            }

            if (is_key) {
                res_dict->keys->data[res_dict->keys->len] = result;
                res_dict->keys->len++;
            } else {
                res_dict->values->data[res_dict->values->len] = result;
                res_dict->values->len++;
            }
            is_key = !is_key;
            
        } else if (bencoded_value[*current_index] == 'i') {
            fprintf(stderr, "Decoding int\n");
            d_res_t* result = decode_int(bencoded_value + *current_index, current_index);
            if (result == NULL) {
                fprintf(stderr, "Failed to decode int: %s", (bencoded_value + *current_index));
                exit(1);
            }

            if (is_key) {
                is_key = false;
                keys->data[keys->len] = result;
                keys->len++;
            } else {
                is_key = true;
                values->data[values->len] = result;
                values->len++;
            }

        } else if (is_digit(bencoded_value[*current_index])){
            fprintf(stderr, "Decoding string\n");
            d_res_t* result = decode_str(bencoded_value + *current_index, current_index);

            if (result == NULL) {
                fprintf(stderr, "Failed to decode string: %s", (bencoded_value + *current_index));
                exit(1);
            }

            if (is_key) {
                is_key = false;
                keys->data[keys->len] = result;
                keys->len++;
            } else {
                is_key = true;
                values->data[values->len] = result;
                values->len++;
            }

        } else if (bencoded_value[*current_index] == 'l') {
            fprintf(stderr, "Decodeing list\n");
            (*current_index)++;
            d_res_t* result = decode_list(bencoded_value, current_index);


            if (result == NULL) {
                fprintf(stderr, "Failed to decode list: %s", (bencoded_value + *current_index - 1));
                exit(1);
            }

            if (is_key) {
                is_key = false;
                keys->data[keys->len] = result;
                keys->len++;
            } else {
                is_key = true;
                values->data[values->len] = result;
                values->len++;
            }
        } else if (bencoded_value[*current_index] == 'e') {
            d_res_t* result = malloc(sizeof(d_res_t));
            result->type = DICT_TYPE;
            result->data.v_dict = res_dict;
            return result;
        }
        (*current_index)++;
    }
    fprintf(stderr, "Invalid dict encoding, didn't finish");
    exit(1);
}


d_res_t* decode(const char* bencoded_value) {
    int* current_index = malloc(sizeof(int));
    if (is_digit(bencoded_value[0])) {
        //fprintf(stderr, "Decoding string\n");
        *current_index = 0;
        d_res_t* result = decode_str(bencoded_value, current_index);
        free(current_index);
        return result;
    } else if (bencoded_value[0] == 'i') {
        *current_index = 0;
        //fprintf(stderr, "Decoding int\n");
        d_res_t* result = decode_int(bencoded_value, current_index);
        free(current_index);
        return result;
    } else if (bencoded_value[0] == 'l') {
        *current_index = 1;
        //fprintf(stderr, "Decoding list\n");
        d_res_t* result = decode_list(bencoded_value, current_index);
        free(current_index);
        return result;
    } else if (bencoded_value[0] == 'd') {
        *current_index = 1;
        //fprintf(stderr, "Decoding dict\n");
        d_res_t* result = decode_dict(bencoded_value, current_index, strlen(bencoded_value)); // This is a hack because some mf decided to put null bytes into torrent files as data
        free(current_index);
        return result;
    } else {
        fprintf(stderr, "Invalid encoding.");
        free(current_index);
        exit(1);
    }
}

static char* encode_str(d_res_t* bdecoded_object) {
    long length = strlen(bdecoded_object->data.v_str);
    char* length_str = malloc(sizeof(char) * LONG_MAX_DIGITS);
    sprintf(length_str, "%ld", length);
    char* res = calloc(length, sizeof(char) + LONG_MAX_DIGITS + 1);
    strcat(res, length_str);
    strcat(res, ":");
    strcat(res, bdecoded_object->data.v_str);
    free(length_str);
    return res;
}

static char* encode_long(d_res_t* bdecoded_object) {
    char* res = calloc(LONG_MAX_DIGITS, sizeof(char));
    sprintf(res, "%ld", bdecoded_object->data.v_long);
    return res;
}

static char* encode_list(d_res_t* bdecoded_object) {
    char* fin = calloc(1024, sizeof(char));
    long allocated = 1024;
    char* res;
    for (int i = 0; i < bdecoded_object->data.v_list->len; i++) {
        res = encode(bdecoded_object->data.v_list->data[i]);
        if (strlen(res) + strlen(fin) > allocated) {
            allocated *= 2;
            fin = realloc(fin, allocated);
        }
        strcat(fin, res);
        free(res);
    }
    return fin;
}

char* encode_dict(d_res_t* bdecoded_object) {
    char* fin = calloc(1024, sizeof(char));
    long allocated = 1024;
    char* res_key;
    char* res_val;
    for (int i = 0; i < bdecoded_object->data.v_dict->keys->len; i++) {
        res_key = encode(bdecoded_object->data.v_dict->keys->data[i]);
        res_val = encode(bdecoded_object->data.v_dict->values->data[i]);
        while (strlen(res_key) + strlen(res_val) + strlen(fin) > allocated) {
            allocated *= 2;
            fin = realloc(fin, allocated);
        }
        strcat(fin, res_key);
        strcat(fin, res_val);
        free(res_key);
        free(res_val);
    }
    return fin;
}

char* encode(d_res_t* bdecoded_object) {
    char* fin = calloc(1024, sizeof(char));
    long allocated = 1024;
    char* result;
    switch (bdecoded_object->type) {
        case STRING_TYPE:
            printf("Encoding string");
            result = encode_str(bdecoded_object);
            break;
        case LONG_TYPE:
            printf("Encoding long");
            strcat(fin, "i");
            result = encode_long(bdecoded_object);
            break;
        case LIST_TYPE:
            printf("Encoding list");
            strcat(fin, "l");
            result = encode_list(bdecoded_object);
            break;
        case DICT_TYPE:
            printf("Encoding dict");
            strcat(fin, "d");
            result = encode_dict(bdecoded_object);
            break;
    }
    if (strlen(fin) + strlen(result) > allocated) {
        allocated *= 2;
        fin = realloc(fin, allocated);
    }

    strcat(fin, result);
    if (bdecoded_object->type != STRING_TYPE) {
        strcat(fin, "e");
    }
    free(result);
    return fin;
}

void d_res_free(d_res_t* d_res) {
    switch (d_res->type) {
        case DICT_TYPE:
            dict_free(d_res->data.v_dict);
            break;
        case LIST_TYPE:
            array_list_free(d_res->data.v_list);
            break;
        case STRING_TYPE:
            free(d_res->data.v_str);
            break;
        case LONG_TYPE:
            break;
        default:
            break;
    }
    free(d_res);
}

void dict_free(dict_t* dict) {
    array_list_free(dict->keys);
    array_list_free(dict->values);
    free(dict);
}

void array_list_free(array_list_t* array) {
    for (int i = 0; i < array->len; i++) {
        d_res_free(array->data[i]);
    }
    free(array->data);
    free(array);
} 

