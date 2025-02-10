#pragma once

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
	int v_int;
	char* v_str;
	array_list_t* v_list;
	dict_t* v_dict;
} d_union_t;


typedef enum {
	INT_TYPE,
	STRING_TYPE,
	LIST_TYPE,
	DICT_TYPE,
} union_type;



struct DecodingResult {
	d_union_t data;
	union_type type;
};

d_res_t* decode_str(const char* bencoded_value);
d_res_t* decode_int(const char* bencoded_value);
d_res_t* decode_list(const char* bencoded_value);
d_res_t* decode_dict(const char* bencoded_value);
bool is_digit(char c);

void d_res_free(d_res_t*);
void dict_free(dict_t*);
void array_list_free(array_list_t*);
