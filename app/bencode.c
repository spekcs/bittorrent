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
//  
// static list_item decode_list_helper(const char* bencoded_value, array_list_t* reference_list, int current_index, int length) {
//     while (current_index < length) {
//         if (bencoded_value[current_index] == 'l') {
//             array_list_t* temp_list = malloc(sizeof(array_list_t));
//             list_item* list_items = malloc(sizeof(list_item) * length);
//             temp_list->data = list_items;
//             temp_list->len = 1;
//             temp_list->data[0] = decode_list_helper(bencoded_value, temp_list, current_index + 1, length);
//             reference_list->data[reference_list->len].data.v_list = temp_list;
//         } 
//     }
//     list_item* res = malloc(sizeof(list_item));
//     res->data.v_list = reference_list;
//     return *res;
// }

d_res_t* decode_list(const char* bencoded_value) {
    int length = strlen(bencoded_value);

    array_list_t* stack[length];
    array_list_t** sp = stack;


    array_list_t* res_list = malloc(sizeof(array_list_t));
    d_res_t** list_items = malloc(sizeof(d_res_t*) * length);
    res_list->data = list_items;
    res_list->len = 0;



    int current_index = 1;
    array_list_t* ref_list = res_list;
    array_list_t* prev_list;

    *sp = ref_list;
    sp++;
    
    while (current_index < length) {
        if (bencoded_value[current_index] == 'l') {
            array_list_t* new_list = malloc(sizeof(array_list_t));
            d_res_t** new_list_items = malloc(sizeof(d_res_t*) * length);
            new_list->data = new_list_items;
            new_list->len = 0;
            
            d_res_t* list_item = malloc(sizeof(d_res_t));
            list_item->data.v_list = new_list;
            list_item->type = LIST_TYPE;
            ref_list->data[ref_list->len] = list_item;
            ref_list->len++;

            //push reference list onto the stack
            *sp = ref_list;
            sp++;

            ref_list = new_list;
        } 
        else if (bencoded_value[current_index] == 'i') {
            int substr_len = strchr((bencoded_value + current_index), 'e') - (bencoded_value + current_index) + 1;
            char* encoded_str = malloc(substr_len + 1);
            encoded_str[substr_len] = '\0';
            strncpy(encoded_str, (bencoded_value + current_index), substr_len);
            d_res_t* result = decode_int(encoded_str);
            free(encoded_str);

            ref_list->data[ref_list->len] = result;
            ref_list->len++;

            current_index += substr_len - 1;
        } else if (is_digit(bencoded_value[current_index])){
            const char* colon_index = strchr((bencoded_value + current_index), ':');
            int colon_relative_index = colon_index - (bencoded_value + current_index);
            fprintf(stderr, "Colon: %d\n", colon_relative_index);
            char* substr_len = malloc(length);
            strncpy(substr_len, (bencoded_value + current_index), colon_relative_index);
            int substring_length_bytes = atoi(substr_len);
            fprintf(stderr, "Substr len in bytes: %d\n", substring_length_bytes);

            free(substr_len);

            char* substr_encoded = malloc(substring_length_bytes);
            strncpy(substr_encoded, (bencoded_value + current_index), substring_length_bytes + colon_relative_index + 1);
            d_res_t* result = decode_str(substr_encoded);

            ref_list->data[ref_list->len] = result;
            ref_list->len++;

            free(substr_encoded);
            current_index += substring_length_bytes;
            current_index += colon_relative_index;

        } else if (bencoded_value[current_index] == 'e' && ref_list != res_list) {
            fprintf(stderr, "Popped frame\n");
            sp--;
            ref_list = *sp;
        }
        current_index++;
    }


    d_res_t* result = malloc(sizeof(d_res_t));
    result->type = LIST_TYPE;
    result->data.v_list = res_list;
    return result;
}


//d_res_t* decode_list(const char* bencoded_value) {
//     int len = strlen(bencoded_value);
//     char* result = (char*)calloc(len, sizeof(char));
//     int curr = 0;
//     int int_started_count = 0;
// 
//     if (bencoded_value[1] == 'e') {
//         strcat(result, "[]\0");
//         return result;
//     }
// 
//     while (curr < len - 1) {
//         if (*(bencoded_value + curr) == 'l') {
//             strcat(result, "[");
//             curr++;
//             continue;
//         } else if (is_digit(*(bencoded_value + curr))) {
//             strcat(result, "\"");
//             const char* colon_index = strchr((bencoded_value + curr), ':');
//             int colon_relative_index = colon_index - (bencoded_value + curr);
//             fprintf(stderr, "Colon: %d\n", colon_relative_index);
//             char* substr_len = malloc(len);
//             strncpy(substr_len, (bencoded_value + curr), colon_relative_index);
//             int substring_length_bytes = atoi(substr_len);
//             fprintf(stderr, "Substr len in bytes: %d\n", substring_length_bytes);
// 
//             free(substr_len);
// 
//             char* substr_encoded = malloc(substring_length_bytes);
//             strncpy(substr_encoded, (bencoded_value + curr), substring_length_bytes + colon_relative_index + 1);
//             char* substr = decode_str(substr_encoded);
//             strcat(result, substr);
//             free(substr_encoded);
//             free(substr);
//             strcat(result, "\"");
//             curr += substring_length_bytes;
//             curr += colon_relative_index;
//         } else if (*(bencoded_value + curr) == 'i') {
//             int substr_len = strchr((bencoded_value + curr), 'e') - (bencoded_value + curr) + 1;
//             char* encoded_str = malloc(substr_len + 1);
//             encoded_str[substr_len] = '\0';
//             strncpy(encoded_str, (bencoded_value + curr), substr_len);
//             char* substr = decode_int(encoded_str);
//             strcat(result, substr);
//             curr += substr_len - 1;
//             free(encoded_str);
//             free(substr);
//         } else if (*(bencoded_value + curr) == 'e') {
//             strcat(result, "]");
//             curr++;
//             continue;
//         }
//         if (curr < len - 1) {
//             strcat(result, ",");
//         } 
//     }
//     strcat(result, "]\0");
//     return result;
// }

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

