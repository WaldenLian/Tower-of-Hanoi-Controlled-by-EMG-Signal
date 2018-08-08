/************************************************************************
* LCD.h
* Header file for the LCD Driver
************************************************************************/
#include <p32xxxx.h>
/* define macros for LCD instructions*/
#define LCD_IDLE 0x33
#define LCD_2_LINE_4_BITS 0x28
#define LCD_2_LINE_8_BITS 0x38
#define LCD_DSP_CSR 0x0c
#define LCD_CLR_DSP 0x01
#define LCD_CSR_INC 0x06
#define LCD_SFT_MOV 0x14
/* define macros for interfacing ports*/
// RS => RD0 J11, 19
// E => RD1 J11, 20
/* Data:
DB7 => RE7 J10, 5
DB6 => RE6 J10, 6
DB5 => RE5 J10, 7
DB4 => RE4 J10, 8
*/
#define RS PORTDbits.RD0
#define E PORTDbits.RD1
#define Data PORTE

typedef unsigned char uchar;

/* define constant strings for display */
const uchar startStr1[] = "Digital Clock";
const uchar startStr2[] = "SJTU JI - LAB4";

/* Function prototypes */
void MCU_init(void);
void LCD_init(void);
void LCD_putchar(uchar c);
void LCD_puts(const uchar *s);
void LCD_goto(uchar addr);
void DelayUsec(int num);
void DelayMsec(int num);
/*****************end of LCD.h**********************/
