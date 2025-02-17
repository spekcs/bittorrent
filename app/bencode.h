#pragma once

#ifndef BENCODE_H
#define BENCODE_H

struct DecodingResult;
typedef struct DecodingResult d_res_t;

typedef struct {
	d_res_t** data;
	int len;
} array_list_t;

typedef struct {
	array_list_t* keys;
	array_list_t* values;
} dict_t;

typedef union {
	long v_long;
	char* v_str;
	array_list_t* v_list;
	dict_t* v_dict;
} d_union_t;


typedef enum {
	LONG_TYPE,
	STRING_TYPE,
	LIST_TYPE,
	DICT_TYPE,
} union_type;

struct DecodingResult {
	d_union_t data;
	union_type type;
};

char* encode(d_res_t* bdecoded_object);
d_res_t* decode(const char* bencoded_value);
d_res_t* decode_dict(const char* bencoded_value, int* current_index, long length);
bool is_digit(char c);

void print_decoded_string(d_res_t* decoded_str);
d_res_t* decode_from_file(const char* filename);

void d_res_free(d_res_t*);
void dict_free(dict_t*);
void array_list_free(array_list_t*);

#endif
