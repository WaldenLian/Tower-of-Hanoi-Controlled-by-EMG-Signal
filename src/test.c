#include <stdio.h>

typedef struct {
    unsigned bit0:1;
    unsigned bit1:1;
    unsigned bit2:1;
    unsigned bit3:1;
    unsigned bit4:1;
    unsigned bit5:1;
    unsigned bit6:1;
    unsigned bit7:1;
} data8;

data8 charToData8(char data)
{

}

int main()
{
//    char a = 'A';
    data8 b;
    b.bit0 = 1;
    printf("Char: %c\n", b);
}