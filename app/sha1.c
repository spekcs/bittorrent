#include <stdio.h>
#include <string.h>

unsigned char* calculate_sha1(unsigned char* str) {
    long long h0 = 0x67452301;
    long long h1 = 0xEFCDAB89;
    long long h2 = 0x98BADCFE;
    long long h3 = 0x10325476;
    long long h4 = 0xC3D2E1F0;

    long long ml = strlen((char*)str) * sizeof(unsigned char) * 8;


    for (int i = 0; i < ml; i++) {
        printf("%d|", ((char*)str)[i]);
    }

    str += 0x08;

    for (int i = 0; i < ml; i++) {
        printf("%d|", ((char*)str)[i]);
    }

    return NULL;
}
