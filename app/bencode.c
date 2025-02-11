#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bencode.h"


bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

d_res_t* decode_str(const char* bencoded_value) {
    int length = atoi(bencoded_value); 
    const char* colon_index = strchr(bencoded_value, ':');
    if (colon_index != NULL) {
        const char* start = colon_index + 1;
        char* decoded_str = (char*)malloc(length + 1);
        strncpy(decoded_str, start, length);
        decoded_str[length] = '\0';
        d_res_t* result = malloc(sizeof(d_res_t));
        result->type = STRING_TYPE;
        result->data.v_str = decoded_str;

        return result;
    } else {
        fprintf(stderr, "Invalid encoded value: %s\n", bencoded_value);
        exit(1);
    }
 }

d_res_t* decode_int(const char* bencoded_value) {
    const char* end_index = strchr(bencoded_value, 'e');
    if (end_index != NULL) {
        int len = strlen(bencoded_value) - 2;
        
        if (len > 1 && bencoded_value[1] == '0') {
            fprintf(stderr, "Encoded integer can't have leading zeroes.");
            exit(1);
        }

        char* decoded_str = (char*)malloc(len + 1);
        strncpy(decoded_str, &bencoded_value[1], len);
        fprintf(stderr,"Decoded str: %s", decoded_str);
        long decoded_int = atol(decoded_str);
        fprintf(stderr,"Decoded int: %ld\n", decoded_int);
        free(decoded_str);
        d_res_t* result = malloc(sizeof(d_res_t));
        result->type = LONG_TYPE;
        result->data.v_long = decoded_int;

        return result;
    } else {
        fprintf(stderr, "Invalid encoded value: %s\n", bencoded_value);
        exit(1);
    }
}

d_res_t* decode_list(const char* bencoded_value, int* current_index) {
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
            int substr_len = strchr((bencoded_value + *current_index), 'e') - (bencoded_value + *current_index) + 1;
            char* encoded_str = malloc(substr_len + 1);
            encoded_str[substr_len] = '\0';
            strncpy(encoded_str, (bencoded_value + *current_index), substr_len);
            d_res_t* result = decode_int(encoded_str);
            free(encoded_str);

            res_list->data[res_list->len] = result;
            res_list->len++;

            *current_index += substr_len - 1;
        } else if (is_digit(bencoded_value[*current_index])){
            const char* colon_index = strchr((bencoded_value + *current_index), ':');
            int colon_relative_index = colon_index - (bencoded_value + *current_index);
            fprintf(stderr, "Colon: %d\n", colon_relative_index);
            char* substr_len = malloc(length);
            strncpy(substr_len, (bencoded_value + *current_index), colon_relative_index);
            int substring_length_bytes = atoi(substr_len);
            fprintf(stderr, "Substr len in bytes: %d\n", substring_length_bytes);

            free(substr_len);

            char* substr_encoded = malloc(substring_length_bytes);
            strncpy(substr_encoded, (bencoded_value + *current_index), substring_length_bytes + colon_relative_index + 1);
            d_res_t* result = decode_str(substr_encoded);

            res_list->data[res_list->len] = result;
            res_list->len++;

            free(substr_encoded);
            *current_index += substring_length_bytes;
            *current_index += colon_relative_index;

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

d_res_t* decode_dict(const char* bencoded_value, int* current_index) {
    int length = strlen(bencoded_value);

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
            (*current_index)++;
            d_res_t* result = decode_dict(bencoded_value, current_index);
            if (is_key) {
                res_dict->keys->data[res_dict->keys->len] = result;
                res_dict->keys->len++;
            } else {
                res_dict->values->data[res_dict->values->len] = result;
                res_dict->values->len++;
            }
            is_key = !is_key;
            
        } else if (bencoded_value[*current_index] == 'i') {
            int substr_len = strchr((bencoded_value + *current_index), 'e') - (bencoded_value + *current_index) + 1;
            char* encoded_str = malloc(substr_len + 1);
            encoded_str[substr_len] = '\0';
            strncpy(encoded_str, (bencoded_value + *current_index), substr_len);
            d_res_t* result = decode_int(encoded_str);
            free(encoded_str);

            if (is_key) {
                is_key = false;
                keys->data[keys->len] = result;
                keys->len++;
            } else {
                is_key = true;
                values->data[values->len] = result;
                values->len++;
            }

            *current_index += substr_len - 1;
        } else if (is_digit(bencoded_value[*current_index])){
            const char* colon_index = strchr((bencoded_value + *current_index), ':');
            int colon_relative_index = colon_index - (bencoded_value + *current_index);
            fprintf(stderr, "Colon: %d\n", colon_relative_index);
            char* substr_len = malloc(length);
            strncpy(substr_len, (bencoded_value + *current_index), colon_relative_index);
            int substring_length_bytes = atoi(substr_len);
            fprintf(stderr, "Substr len in bytes: %d\n", substring_length_bytes);

            free(substr_len);

            char* substr_encoded = malloc(substring_length_bytes);
            strncpy(substr_encoded, (bencoded_value + *current_index), substring_length_bytes + colon_relative_index + 1);
            d_res_t* result = decode_str(substr_encoded);


            if (is_key) {
                is_key = false;
                keys->data[keys->len] = result;
                keys->len++;
            } else {
                is_key = true;
                values->data[values->len] = result;
                values->len++;
            }

            free(substr_encoded);
            *current_index += substring_length_bytes;
            *current_index += colon_relative_index;
        } else if (bencoded_value[*current_index] == 'l') {
            (*current_index)++;
            d_res_t* result = decode_list(bencoded_value, current_index);

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

