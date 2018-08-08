#include <p32xxxx.h>
#include <plib.h>

#define LCD_DC PORTEbits.RE0 // J10 12
#define LCD_SDI PORTEbits.RE1 // J10 11
#define LCD_SCK PORTEbits.RE2 // J10 10
#define LCD_REST PORTEbits.RE3 // J10 9

#define LCD_W 240
#define LCD_H 320


typedef unsigned short u16;
typedef unsigned char u8;

u16 POINT_COLOR = 0x0020;

void MCU_init();
void DelayUsec(int num);
void DelayMsec(int num);
void LCD_Write_Bus(char data);
void LCD_Write_Data8(char data);
void LCD_Write_Data(u16 data);
void LCD_Write_Reg(char data);
void LCD_Write_Reg_Data(int reg, int data);
void Address_set(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_init(void);
void LCD_Clear(u16 Color);
void LCD_DrawPoint(u16 x, u16 y);
void LCD_ShowChar(u16 x, u16 y, u8 num, u8 mode);
void LCD_ShowString(u16 x, u16 y, const u8 *p);
