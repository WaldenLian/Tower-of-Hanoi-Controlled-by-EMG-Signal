#include <p32xxxx.h>
#include <plib.h>

/*
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
*/

/*
SDI1(RF7) SDA
RS (A0) 
RES
CS
D7-D4 Vdd
D3-D1 SDA
D0(SCK) SCL1
E Vdd
R/W Vdd
*/

#define LCD_DC PORTEbits.RE0
#define LCD_SDI PORTEbits.RE1
#define LCD_SCK PORTEbits.RE2
#define LCD_REST PORTEbits.RE3

/**
void DelayUsec(int num);
void DelayMsec(int num);
**/

/* initialize the PIC32 MCU */
void MCU_init()
{
    /* setup I/O ports to connect to the LCD module */
    /* Timer control */ 
    TRISD = 0xFF00;
    TRISE = 0xFFF0;
    TRISF = 0XFFF0;
    T2CON = 0x0;
    T3CON = 0x0;
    T2CON = 0x0038;     // Stop Timer2 and clear control register
                        // prescale 1:8, internal clock source
    PR2 = 0xFFFFFFFF;
    TMR2 = 0x0;
    T2CONSET = 0x8000;
}

/* configure timer SFRs to generate num us delay*/
void DelayUsec(int num)
{
    TMR2 = 0x0;
    while (TMR2 < 10*num);
}
/* Call GenMsec() num times to generate num ms delay*/
void DelayMsec(int num)
{
    TMR2 = 0x0;
    while (TMR2 < 10000*num);
}

void transfer_cmd(unsigned char cmd)
{
    int k;
    int transfer_bit;
    cs = 0;
    rs = 0;
    for (k = 0; k < 8; k++)
    {
        transfer_bit = cmd & 0x80;
        cmd = cmd << 1;
        sck = 0;
        sda = transfer_bit | transfer_bit << 1 | transfer_bit << 2 | transfer_bit << 3;
        sck = 1;
    }
    cs = 1;
}

void transfer_data(unsigned char data)
{
    unsigned char k;
    int transfer_bit;
    cs = 0;
    rs = 1;
    for (k = 0; k < 8; k++)
    {
        transfer_bit = data & 0x80;
        data = data << 1;
        sck = 0;
        sda = transfer_bit | transfer_bit << 1 | transfer_bit << 2 | transfer_bit << 3;
        sck = 1;
    }
    cs = 1;
}

void LCD_init(void)
{
    reset = 0;
    DelayMsec(100);
    reset = 1;
    DelayMsec(100);

    transfer_cmd(0x30); // EXT = 0
    transfer_cmd(0x94); // Sleep out
    transfer_cmd(0x31); // EXT = 1
    transfer_cmd(0xD7); // Autoread disable
    transfer_data(0x9F);

    transfer_cmd(0x32); // Analog SET
    transfer_data(0x00); // OSC Frequency adjustment
    transfer_data(0x01); // Frequency on booster capacitors->6KHz
    transfer_data(0x03); // Bias = 1/11

    transfer_cmd(0x20); // Gray level
    unsigned char gray_data[] = {0x01, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0D, 0x10, 0x11, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1D, 0x1F};
    int i = 0;
    for (i = 0; i < sizeof(gray_data)/sizeof(unsigned char); ++i)
    {
        transfer_data(gray_data[i]);
    }

    transfer_cmd(0x30); // EXT = 0
    transfer_cmd(0x75); // Page Address setting
    transfer_data(0x00); // XS = 0
    transfer_data(0x28); // XE = 159 0x28
    transfer_cmd(0x15); // Column Address setting
    transfer_data(0x00); // XS = 0
    transfer_data(0xFF); // XE = 256

    transfer_cmd(0xBC); // Data scan direction
    transfer_data(0x00); // MX.MY = Normal
    transfer_data(0xA6);

    transfer_cmd(0xCA); // Display Control
    transfer_data(0x00);
    transfer_data(0x9F); // Duty = 160
    transfer_data(0x20); // Nline = off

    transfer_cmd(0xF0); // Display Mode
    transfer_data(0x10); // 10 = Monochrome Mode, 11 = 4Gray

    transfer_cmd(0x81); // EV control
    transfer_data(0x38); // VPR[5-0]
    transfer_data(0x04); // VPR[8-6]

    transfer_cmd(0x20); // Power control
    transfer_data(0x0B); // D0 = regulator; D1 = follower; D3 = booste, on:1 off:0
    DelayUsec(100);
    transfer_cmd(0xAF); // Display on
}

void LCD_addr(int x, int y, int x_total, int y_total)
{
    x--;
    y--;

    transfer_cmd(0x15); // Set Column Address
    transfer_data(x);
    transfer_data(x+x_total-1);

    transfer_cmd(0x75); // Set Page Address
    transfer_data(y);
    transfer_data(y+y_total-1);

    transfer_cmd(0x30);
    transfer_cmd(0x5C);
}

void clear_screen(int x, int y)
{
    int i, j;
    LCD_addr(x, y, 256, 21);
    for (i = 0; i < 21; ++i)
    {
        for (j = 0; j < 256; ++j)
        {
            transfer_data(0x12);
        }
    }
}

void test(unsigned char data1, unsigned char data2)
{
    int i, j;
    LCD_addr(1, 1, 256, 21);
    for (i = 0; i < 20; ++i)
    {
        for (j = 0; j < 255; ++j)
        {
            transfer_data(data1);
            transfer_data(data2);
        }
    }
}

int main()
{
    MCU_init();
    LCD_init();

    while (1)
    {
		int i = 0;
        clear_screen(1, 1);
        test(0xAA, 0x55);
		int j = 0;
    }
    
    return 0;
}