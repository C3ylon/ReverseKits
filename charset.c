#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// char* utf16le_to_utf8(const uint16_t* utf16)
// {
//     int len16 = 0;
//     while (utf16[len16] != 0) {
//         len16++;
//     }
//     int len8 = 0;
//     for (int i = 0; i < len16; i++) {
//         if (utf16[i] < 0x80) {
//             len8++;
//         }
//         else if (utf16[i] < 0x800) {
//             len8 += 2;
//         }
//         else {
//             len8 += 3;
//         }
//     }

//     char* utf8 = (char*)malloc(len8 + 1);
//     if (utf8 == NULL) {
//         return NULL;
//     }

//     int j = 0;
//     for (int i = 0; i < len16; i++) {
//         uint16_t c = utf16[i];
//         if (c < 0x80) {
//             utf8[j++] = (char)c;
//         }
//         else if (c < 0x800) {
//             utf8[j++] = (char)(0xC0 | (c >> 6));
//             utf8[j++] = (char)(0x80 | (c & 0x3F));
//         }
//         else {
//             utf8[j++] = (char)(0xE0 | (c >> 12));
//             utf8[j++] = (char)(0x80 | ((c >> 6) & 0x3F));
//             utf8[j++] = (char)(0x80 | (c & 0x3F));
//         }
//     }

//     utf8[j] = '\0';

//     return utf8;
// }


void utf16le_to_utf8(const unsigned char *utf16le_str, unsigned char *utf8_str) {
    int i = 0, j = 0;
    while (utf16le_str[i] != '\0') {
        unsigned short ucs2_char = (utf16le_str[i+1] << 8) | utf16le_str[i];
        i += 2;
        if (ucs2_char <= 0x7f) {
            utf8_str[j++] = (char)ucs2_char;
        } else if (ucs2_char <= 0x7ff) {
            utf8_str[j++] = (char)(0xc0 | (ucs2_char >> 6));
            utf8_str[j++] = (char)(0x80 | (ucs2_char & 0x3f));
        } else if (ucs2_char <= 0xffff) {
            utf8_str[j++] = (char)(0xe0 | (ucs2_char >> 12));
            utf8_str[j++] = (char)(0x80 | ((ucs2_char >> 6) & 0x3f));
            utf8_str[j++] = (char)(0x80 | (ucs2_char & 0x3f));
        } else {
            utf8_str[j++] = '?';
        }
    }
    utf8_str[j] = '\0';
}

void utf8_to_utf16le(const char *utf8_str, char *utf16le_str) {
    int i = 0, j = 0;
    while (utf8_str[i] != '\0') {
        unsigned char utf8_char = (unsigned char)utf8_str[i];
        if ((utf8_char & 0x80) == 0) {
            utf16le_str[j++] = utf8_char;
        } else if ((utf8_char & 0xe0) == 0xc0) {
            utf16le_str[j++] = (char)(((utf8_char & 0x1f) << 6) | (utf8_str[i+1] & 0x3f));
            i++;
        } else if ((utf8_char & 0xf0) == 0xe0) {
            utf16le_str[j++] = (char)(((utf8_char & 0x0f) << 12) | ((utf8_str[i+1] & 0x3f) << 6) | (utf8_str[i+2] & 0x3f));
            i += 2;
        } else {
            utf16le_str[j++] = '?';
        }
        i++;
    }
    utf16le_str[j++] = '\0';
    utf16le_str[j++] = '\0';
}

