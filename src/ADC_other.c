#include <plib.h>
#include "LCD.h"
void initADC(void);
void ADC_ISR(void);

int ADCValue;
int ADCValuea;

//uchar vol[5];
//uchar vola[5];

#pragma interrupt ADC_ISR ipl7 vector 27
void ADC_ISR(void)
{
    IFS1bits.AD1IF = 0;
}

void initADC(void)
{
    //DDPCONbits.JTAGEN = 0;
    //TRISB=0x0;
    //CM1CONbits.COE=0;
    //CM2CONbits.COE=0;
    INTCONbits.MVEC = 1;
    asm("ei");
    
    AD1PCFG = 0xfffc;            //AN0 ~ AN5 & AN8 & AN9 analog
    AD1CSSL = 0x0003;           //scan AN0 ~ AN5 & AN8 & AN9
    AD1CON2 = 0x0404;
    /*AD1CON2bits.VCFG = 0b000;  // Voltage refernces from AVdd and AVss (power rails)
    AD1CON2bits.OFFCAL = 0;    // turn off calibration mode
    AD1CON2bits.CSCNA = 1;     // turn on auto-scanning of inputs
    AD1CON2bits.SMPI = 0b0001; //interrupt every 8 sample conversion
    AD1CON2bits.BUFM = 0;      // one 16-word buffer (not using interrupts)
    AD1CON2bits.ALTS = 0;      // Use Mux A only*/

    AD1CON3 = 0x05FF;
    //AD1CON3bits.ADRC = 0;     //PBCLK
    //AD1CON3bits.SAMC = 0x5;   //5TAD=auto sample bits
    //AD1CON3bits.ADCS = 0xFF; //TAD=32TPB

    AD1CON1 = 0x00e4;
    //AD1CON1bits.SSRC = 0b111; //auto convert at end of Tsample
    //AD1CON1bits.ASAM = 1;      //auto sample at end of Tconvert
    //AD1CON1bits.CLRASAM = 0;   //overwrite buffer on each interrupt
    //AD1CON1bits.FORM = 0b000;  //16 bit integer

    IFS1CLR=0x2;//clear interrupt flag
    IEC1SET=0x2;//enable AD1 interrupt
    IPC6SET=0x1e000000;//priority level 3
    //ConfigIntADC10(ADC_INT_PRI_6 | ADC_INT_SUB_PRI_3 | ADC_INT_ON);
    //mAD1ClearIntFlag();

    AD1CON1bits.ON = 1; //turn on ADC moodule
}

int main()
{
OSCSetPBDIV(OSC_PB_DIV_1); //configure PBDIV so PBCLK = SYSCLK

initADC();
while(1)
{
    
/*
double dec;
dec = ADCValue/1024.0*3.3;
//display char
int temp;
temp = (int)dec%10;
vol[0] = (uchar)temp+48; //int to char
dec = (dec-temp)*10;
vol[1] = '.';
temp = (int)dec%10;
vol[2] = (uchar)temp+48; //int to char
dec = (dec-temp)*10;
temp = (int)dec%10;
vol[3] = (uchar)temp+48; //int to char
vol[4] = '\0';

double deca;
deca = ADCValuea/1024.0*3.3;
//display char
int tempa;
tempa = (int)deca%10;
vola[0] = (uchar)tempa+48; //int to char
deca = (deca-tempa)*10;
vola[1] = '.';
tempa = (int)deca%10;
vola[2] = (uchar)tempa+48; //int to char
deca = (deca-tempa)*10;
tempa = (int)deca%10;
vola[3] = (uchar)tempa+48; //int to char
vola[4] = '\0';

LCD_goto(0x00);
LCD_puts(vol);
LCD_goto(0x40);
LCD_puts(vola);
*/
}
}

/* initialize the PIC32 MCU */
void MCU_init()
{
/* setup I/O ports to connect to the LCD module */
/* Timer control */
TRISD = 0xFF00;
TRISE = 0xFF00;
T2CON = 0x0;
T3CON = 0x0;
T2CON = 0x0038; // Stop Timer2 and clear control register
// prescale 1:8, internal clock source
PR2 = 0xFFFFFFFF;
TMR2 = 0x0;
T2CONSET = 0x8000;
}
/* initialize the LCD module */
void LCD_init()
{
DelayMsec(15); //wait for 15 ms
RS = 0; //send command
Data = LCD_IDLE; //function set - 8 bit interface
DelayMsec(5); //wait for 5 ms
Data = LCD_IDLE; //function set - 8 bit interface
DelayUsec(100); //wait for 100 us
Data = LCD_IDLE; //function set
DelayMsec(5);
Data = LCD_IDLE;
DelayUsec(100);
LCD_putchar(LCD_2_LINE_4_BITS);
DelayUsec(40);
LCD_putchar(LCD_DSP_CSR);
DelayUsec(40);
LCD_putchar(LCD_CLR_DSP);
DelayMsec(5);
LCD_putchar(LCD_CSR_INC);
}
/* Send one byte c (instruction or data) to the LCD */
void LCD_putchar(uchar c)
{
E = 1;
Data = c; //sending higher nibble
E = 0; //producing falling edge on E
E = 1;
Data <<= 4; //sending lower nibble through higher 4 ports
E = 0; //producing falling edge on E
}
/* Display a string of characters *s by continuously calling LCD_putchar()
*/
void LCD_puts(const uchar *s)
{
RS = 1;
int str_length = strlen(s);
int i;
for (i = 0; i < str_length; i++) {
LCD_putchar(s[i]);
DelayUsec(40);
//DelayUsec(100);
}
}
/* go to a specific DDRAM address addr */
void LCD_goto(uchar addr)
{
RS = 0;
addr |= 0x80; // set the DB7 = 1 to indicate that DB6-DB0 denotes an address
LCD_putchar(addr); // send an address to the LCD
DelayUsec(40);
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
