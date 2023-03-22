#include <stdlib.h>

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
        } else if(utf8[i] & 0x20){
            i += 2;
        } else {
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
        unsigned char c = utf8[i];
        if(c == 0) {
            
        }
    }

}
