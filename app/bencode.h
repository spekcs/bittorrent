#pragma once

char* decode_str(const char* bencoded_value);
char* decode_int(const char* bencoded_value);
char* decode_list(const char* bencoded_value);
char* decode_dict(const char* bencoded_value);
bool is_digit(char c);
