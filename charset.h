#ifndef CHARSET__H_
#define CHARSET__H_

#include <stdlib.h>

// need caller to free pointer.
unsigned char *utf16le_to_utf8(const unsigned short *utf16);
unsigned short *utf8_to_utf16le(const unsigned char *utf8);
char *binary_to_base16(char *data, size_t len);

unsigned char *utf16le_to_utf8(const unsigned short *utf16)
{
    int len8 = 0, i = 0;
    while(1) {
        if(utf16[i] == 0) {
            break;
        }
        if (utf16[i] < 0x80) {
            ++len8;
        } else if (utf16[i] < 0x800) {
            len8 += 2;
        } else {
            len8 += 3;
        }
        ++i;
    }

    unsigned char *utf8 = (unsigned char *)malloc(len8 + 1);
    if (utf8 == 0) {
        return 0;
    }

    i = 0;
    int j = 0;
    while(1) {
        unsigned short c = utf16[i++];
        if(c == 0) {
            utf8[j] = '\0';
            return utf8;
        }
        if (c < 0x80) {                             //  7 bits effective
            utf8[j++] = c;
        } else if (c < 0x800) {                     //  11 bits effective
            utf8[j++] = 0xC0 | (c >> 6);            //  high 5 bits
            utf8[j++] = 0x80 | (c & 0x3F);          //  low 6 bits
        } else {                                    //  16 bits effective
            utf8[j++] = 0xE0 | (c >> 12);           //  high 4 bits
            utf8[j++] = 0x80 | ((c >> 6) & 0x3F);   //  mid 6 bits
            utf8[j++] = 0x80 | (c & 0x3F);          //  low 6 bits
        }
    }
}

unsigned short *utf8_to_utf16le(const unsigned char *utf8)
{
    int len16 = 0, i = 0;
    while(1) {
        if(utf8[i] == 0) {
            break;
        }
        if(utf8[i] < 0x80) {
            ++i;
        } else if((utf8[i] & 0x20) == 0){
            if((utf8[i+1] & 0xC0) != 0x80) {
                return 0;
            }
            i += 2;
        } else {
            if((utf8[i+1] & 0xC0) != 0x80 
                || (utf8[i+2] & 0xC0) != 0x80 ) {
                return 0;
            }
            i += 3;
        }
        ++len16;
    }
    
    unsigned short *utf16 = (unsigned short*)malloc(len16 * sizeof (unsigned short));
    if(utf16 == 0) {
        return 0;
    }

    i = 0;
    int j = 0;
    while(1) {
        unsigned char c = utf8[i++];
        if(c == 0) {
            utf16[j] = '\0';
            return utf16;
        }
        if(c < 0x80) {
            utf16[j++] = c;
        } else if((c & 0x20) == 0) {
            utf16[j++] = ((c & 0x1F) << 6)
                        | (utf8[i++] & 0x3F); 
        } else {
            utf16[j++] = ((c & 0x0F) << 12)
                        | ((utf8[i] & 0x3F) << 6)
                        | (utf8[i+1] & 0x3F);
            i += 2;
        }
    }
}

char base16charset[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

char *binary_to_base16(char *data, size_t len)
{
    size_t i = 0;
    char *res = (char *)malloc(len * 2);
    while(i < len) {
        res[i * 2] = base16charset[ data[i] >> 4 ];
        res[i*2 + 1] = base16charset[ data[i] & 0xF ];
        ++i;
    }
    return res;
}

#endif