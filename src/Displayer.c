#include "Displayer.h"


/* initialize the PIC32 MCU */
void MCU_init()
{
    /* setup I/O ports to connect to the LCD module */
    /* Timer control */ 
    TRISE = 0xFFF0;
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

// Write from the MSB
void LCD_Write_Bus(char data)
{
    int i;
    for (i = 0; i < 8; ++i)
    {
        LCD_SCK = 0;
        if (data & 0x80) {
            LCD_SDI = 1;
        } else {
            LCD_SDI = 0;
        } 
        LCD_SCK = 1;
        data << 1;
    }
}

void LCD_Write_Data8(char data)
{
    LCD_DC = 1;
    LCD_Write_Bus(data);
}

void LCD_Write_Data(u16 data)
{
    LCD_DC = 1;
    LCD_Write_Bus(data >> 8);
    LCD_Write_Bus(data);
}

void LCD_Write_Reg(char data)
{
    LCD_DC = 0;
    LCD_Write_Bus(data);
}

void LCD_Write_Reg_Data(int reg, int data)
{
    LCD_Write_Reg(reg);
    LCD_Write_Data(data);
}

void Address_set(unsigned int x1, unsigned int y1, 
                unsigned int x2, unsigned int y2)
{
    LCD_Write_Reg(0x2a);
    LCD_Write_Data8(x1>>8);
    LCD_Write_Data8(x1);
    LCD_Write_Data8(x2>>8);
    LCD_Write_Data8(x2);

    LCD_Write_Reg(0x2b);
    LCD_Write_Data8(y1>>8);
    LCD_Write_Data8(y1);
    LCD_Write_Data8(y2>>8);
    LCD_Write_Data8(y2);

    LCD_Write_Reg(0x2c);
}

void LCD_init(void)
{    
    LCD_REST=0;
    DelayMsec(200);
    LCD_REST=1;
    DelayMsec(200);


    //************* Start Initial Sequence **********// 
    LCD_Write_Reg(0xCF);  
    LCD_Write_Data8(0x00); 
    LCD_Write_Data8(0xD9); 
    LCD_Write_Data8(0X30); 

    LCD_Write_Reg(0xED);  
    LCD_Write_Data8(0x64); 
    LCD_Write_Data8(0x03); 
    LCD_Write_Data8(0X12); 
    LCD_Write_Data8(0X81); 

    LCD_Write_Reg(0xE8);  
    LCD_Write_Data8(0x85); 
    LCD_Write_Data8(0x10); 
    LCD_Write_Data8(0x78); 

    LCD_Write_Reg(0xCB);  
    LCD_Write_Data8(0x39); 
    LCD_Write_Data8(0x2C); 
    LCD_Write_Data8(0x00); 
    LCD_Write_Data8(0x34); 
    LCD_Write_Data8(0x02); 

    LCD_Write_Reg(0xF7);  
    LCD_Write_Data8(0x20); 

    LCD_Write_Reg(0xEA);  
    LCD_Write_Data8(0x00); 
    LCD_Write_Data8(0x00); 

    LCD_Write_Reg(0xC0);    //Power control 
    LCD_Write_Data8(0x21);   //VRH[5:0] 

    LCD_Write_Reg(0xC1);    //Power control 
    LCD_Write_Data8(0x12);   //SAP[2:0];BT[3:0] 

    LCD_Write_Reg(0xC5);    //VCM control 
    LCD_Write_Data8(0x32); 
    LCD_Write_Data8(0x3C); 

    LCD_Write_Reg(0xC7);    //VCM control2 
    LCD_Write_Data8(0XC1); 

    LCD_Write_Reg(0x36);    // Memory Access Control 
    LCD_Write_Data8(0xA8); 

    LCD_Write_Reg(0x3A);   
    LCD_Write_Data8(0x55); 

    LCD_Write_Reg(0xB1);   
    LCD_Write_Data8(0x00);   
    LCD_Write_Data8(0x18); 

    LCD_Write_Reg(0xB6);    // Display Function Control 
    LCD_Write_Data8(0x0A); 
    LCD_Write_Data8(0xA2); 


    LCD_Write_Reg(0xF2);    // 3Gamma Function Disable 
    LCD_Write_Data8(0x00); 

    LCD_Write_Reg(0x26);    //Gamma curve selected 
    LCD_Write_Data8(0x01); 

    LCD_Write_Reg(0xE0);    //Set Gamma 
    LCD_Write_Data8(0x0F); 
    LCD_Write_Data8(0x20); 
    LCD_Write_Data8(0x1E); 
    LCD_Write_Data8(0x09); 
    LCD_Write_Data8(0x12); 
    LCD_Write_Data8(0x0B); 
    LCD_Write_Data8(0x50); 
    LCD_Write_Data8(0XBA); 
    LCD_Write_Data8(0x44); 
    LCD_Write_Data8(0x09); 
    LCD_Write_Data8(0x14); 
    LCD_Write_Data8(0x05); 
    LCD_Write_Data8(0x23); 
    LCD_Write_Data8(0x21); 
    LCD_Write_Data8(0x00);     LCD_Write_Reg(0XE1);    //Set Gamma 
    LCD_Write_Data8(0x00); 
    LCD_Write_Data8(0x19); 
    LCD_Write_Data8(0x19); 
    LCD_Write_Data8(0x00); 
    LCD_Write_Data8(0x12); 
    LCD_Write_Data8(0x07); 
    LCD_Write_Data8(0x2D); 
    LCD_Write_Data8(0x28); 
    LCD_Write_Data8(0x3F); 
    LCD_Write_Data8(0x02); 
    LCD_Write_Data8(0x0A); 
    LCD_Write_Data8(0x08); 
    LCD_Write_Data8(0x25); 
    LCD_Write_Data8(0x2D); 
    LCD_Write_Data8(0x0F); 

    LCD_Write_Reg(0x11);    //Exit Sleep 
    DelayMsec(200); 
    LCD_Write_Reg(0x29);    //Display on 
    DelayMsec(50);
}

void LCD_Clear(u16 Color)
{
    u16 i, j;
    Address_set(0, 0, 320-1, 240-1);
    for (i = 0; i < LCD_W; ++i)
    {
        for (j = 0; j < LCD_H; ++j)
        {
            LCD_Write_Data(Color);
        }
    }
}

int main1()
{
    MCU_init();
    LCD_init();
    LCD_Clear(0x0000);
    POINT_COLOR=0xFFFF;
    // LCD_ShowString(10,35,"2.4 TFT SPI 240*320");
    // LCD_DrawPoint(100, 100);

    while (1)
    {
        LCD_init();
        LCD_Clear(0xF800);
        DelayMsec(3000);
        LCD_Clear(0x07E0);
        DelayMsec(3000);
        LCD_Clear(0x001F);
        DelayMsec(3000);
    }
    
    return 0;
}
