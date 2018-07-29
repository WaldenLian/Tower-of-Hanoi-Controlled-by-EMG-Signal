#include <p32xxxx.h>
#include <plib.h>

#define rs PORTDbits.RD2    // J11 17
#define sck PORTDbits.RD3   // J11 18
#define reset PORTDbits.RD5 // J10 13
#define cs PORTDbits.RD6    // J11 12
#define sda PORTF   // RF lower-four bits 
                    // {D3, D2, D1, D0} => {RF3, RF2, RF1, RF0} 
                    // J11 43, J11 41
                    // J11 8, J11 7
#define ROM_IN PORTEbits.RE1    // J10 11
#define ROM_OUT PORTEbits.RE0   // J10 12
#define ROM_SCK PORTEbits.RE2   // J10 10
#define ROM_CS PORTEbits.RE3    // J10 9


void transfer_cmd(unsigned char cmd)
{
    int k;
    cs = 0;
    rs = 0;
    for (k = 0; k < 8; k++)
    {
        cmd = cmd << 1;
        sck = 0;
        sda = 0;
        sck = 1;
    }
    cs = 1;
}

void transfer_data(unsigned char data)
{
    unsigned char k;
    cs = 0;
    rs = 1;
    for (k = 0; k < 8; k++)
    {
        data = data << 1;
        sda = 1;
        sck = 0;
        sck = 1;
    }
    cs = 1;
}


int main()
{
    
    return 0;
}