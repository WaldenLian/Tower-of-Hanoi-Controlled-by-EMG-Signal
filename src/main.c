#include "LCD.h"
#include <plib.h>
#include <sys/kmem.h>

#define EMG_th (0.6)
int ADCValue0;
int ADCValue1;
int ADCValue2;
int readD;
uchar pkt;
int time_result = 0;
uchar step_reception;
int total_step = 0;
int success_flag = 0;
int reset_flag = 0;
unsigned int acc = 16;
int pwm = 0;
int MAX_PWM = 256;
int cnt = 0;
int put_down_flag = 0;
void MCU_init(void);
void ADC_init(void);
void pressure(void);
void UART_init(void);
void CN_init(void);
void PWM_init(void);
void PWM_CTR(void);


/* Timer2 ISR - handling OC-PWM module operations */
#pragma interrupt PWM_ISR ipl3 vector 8
void PWM_ISR(void)
{
    OC3RS = pwm;      //update duty cycle register
    IFS0CLR = 0x0100; //clear Timer 2 interrupt flag
}

#pragma interrupt CN_ISR ipl7 vector 26
void CN_ISR(void)
{
    IEC1CLR = 0x0001;
    if (readD != PORTDbits.RD6) {
        readD = PORTDbits.RD6; // Clear the mismatch
        if (PORTDbits.RD6) {
            pkt = 'R';
            U1ATXREG = pkt;
            DelayMsec(100);
            reset_flag = 1;
        }
    }
    IFS1CLR = 0x0001;
    IEC1SET = 0x0001;
}

#pragma interrupt UART_RX_ISR ipl4 vector 24
void UART_RX_ISR (void)
{
    asm("di");
	IEC0CLR = 0x08000000;
    IFS0CLR = 0x08000000;
    step_reception = (uchar) U1ARXREG;

    if (step_reception == 'K') {
        // total_step++;
        success_flag = 1;
        IEC0CLR = 0x00001000;
    } else if (step_reception == 'S') {
        // Maximal Step: 99
        total_step++;
        uchar stepStr[] = "Step:   ";
        stepStr[6] = total_step/10 + '0';
        stepStr[7] = total_step%10 + '0';
        LCD_goto(0x00);
        LCD_puts(stepStr);
        put_down_flag = 1;
    }
	IEC0SET = 0x08000000;
    asm("ei");
}

#pragma interrupt T3_ISR ipl6 vector 12
void T3_ISR (void)
{
    IFS0CLR = 0x00001000;
    time_result += 1; // result * 0.1 second
    if (time_result%10 == 0) {
        int total_second = time_result/10;
        int second = total_second%60;
        int minute = total_second/60;
        uchar timeStr[] = "Time:      ";
        timeStr[6] = minute/10 + '0';
        timeStr[7] = minute%10 + '0';
        timeStr[8] = ':';
        timeStr[9] = second/10 + '0';
        timeStr[10] = second%10 + '0';
        LCD_goto(0x40);
        LCD_puts(timeStr);
    }
    // Display the time here in the format 00:00
}

int main()
{
    OSCSetPBDIV(OSC_PB_DIV_1); //configure PBDIV so PBCLK = SYSCLK
    MCU_init();
    ADC_init();
    UART_init();
    CN_init();
    PWM_init();
    LCD_init();
    uchar stepStr[] = "Step: 00";
    LCD_goto(0x00);
    LCD_puts(stepStr);
    while(1){
        if (success_flag == 1) {
            total_step++;
            uchar stepStr[] = "Step:   ";
            stepStr[6] = total_step/10 + '0';
            stepStr[7] = total_step%10 + '0';
            LCD_goto(0x00);
            LCD_puts(stepStr);
            success_flag = 0;
        }
        uchar stepStr[] = "Step:   ";
        stepStr[6] = total_step/10 + '0';
        stepStr[7] = total_step%10 + '0';
        LCD_goto(0x00);
        LCD_puts(stepStr);
        if (reset_flag == 1) {
            success_flag = 0;
            reset_flag = 0;
            time_result = 0;
            total_step = 0;
            IEC0SET = 0x00001000;
            TMR3 = 0;
            uchar stepStr[] = "Step:   ";
            stepStr[6] = total_step/10 + '0';
            stepStr[7] = total_step%10 + '0';
            LCD_goto(0x00);
            LCD_puts(stepStr);
            pwm = 0;
            put_down_flag = 0;
        }
        pressure();
        PWM_CTR();
    }
}

void MCU_init(void)
{
    INTCONbits.MVEC = 1;
    asm("ei");
    TRISDbits.TRISD6 = 1;
    TRISDbits.TRISD0 = 0;
    TRISDbits.TRISD1 = 0;
    TRISE = 0xFF00;

    /* Timer control */
    T4CON = 0x0;
    T5CON = 0x0;
    T4CON = 0x0038;     // Stop Timer4 and clear control register
                        // prescale 1:8, internal clock source
    PR4 = 0xFFFFFFFF;
    TMR4 = 0x0;
    T4CONSET = 0x8000;

    /* Timer 3 */
    IPC3SET = 0x00000018; // priority 6
    IFS0CLR = 0x00001000;
    IEC0SET = 0x00001000;
    T3CON = 0x0;
    T3CON = 0x0070; // 1:256
    PR3 = 31249; // 0.1 second
    TMR3 = 0;

    T3CONSET = 0x8000;
}

/* Initialize OC module and timer base - Timer 2 */
void PWM_init()
{
    OC3CON = 0x0000; //stop OC1 module
    OC3RS = 0;       //initialize duty cycle register
    OC3R = 0;        //initialize OC1R register for the first time
    OC3CON = 0x0006; //OC1 16-bit, Timer 2, in PWM mode w/o FP
    PR2 = 0x00FF;    //PWM signal period = 0x100*1/PBCLK = 32 us
    //Thus, PWM Frequency = 32.25 kHz
    IFS0CLR = 0x00000100; //clear Timer 2 interrupt
    IEC0SET = 0x00000100; //enable Timer 2 interrupt
    IPC2SET = 0x0000000F; //Timer 2 interrupt priority 3, subpriority 3
    T2CONSET = 0x8000;    //start Timer 2
    OC3CONSET = 0x8000;   //enable OC1 module for PWM generation
}

void PWM_CTR(void)
{
    int count = 0;
    if (put_down_flag) count = 5000;
    while (count>0){
        if (pwm <= MAX_PWM) pwm += acc;
        else pwm = 0;
        count--;
    }
    put_down_flag = 0;
}

void CN_init(void)
{
    asm("di");
    CNCON = 0x8000;
    CNEN = 0x00008000; // RD6 => CNEN15
    CNPUE = 0x00008000;

    readD = PORTDbits.RD6;

    IPC6SET = 0x001c0000;
    IPC6SET = 0x00030000;
    IFS1CLR = 0x0001;
    IEC1SET = 0x0001;
    asm("ei");
}

void UART_init(void)
{
    // UART1
    //asm("di");
    U1ABRG = 2082;// Set the Baud rate
    U1ASTA = 0;
    U1AMODESET = 0x8000; // enable the UART module
	U1AMODESET = 0x0008; // BRGH = 1

    // Recevier setting
    U1ASTAbits.URXEN = 1; // enable the UART receiver

    IFS0CLR = 0x08000000; // Clear the interrupt flag
    IEC0SET = 0x08000000; // Enable the interrupt
    IPC6SET = 0x00000010; // priority 4 and sub-group priority 0

    // IFS0CLR = 0x10000000; // Clear the TX interrupt flag
    // IEC0SET = 0x10000000;
    // U1ASTAbits.UTXISEL1 = 0;
    // IEC0CLR = 0x10000000; // Disable the TX interrupt

	//asm("ei");
    U1ASTASET = 0x1400; // Enable transmission
}

void ADC_init()
{
    INTCONbits.MVEC = 1;
    asm("ei");
    AD1PCFG = 0xfff8; //AN0, AN1 analog
    AD1CSSL = 0x0007;

    AD1CON1bits.FORM = 0b000; //integer 16 bits
    AD1CON1bits.SSRC = 0b111; //auto sample
    AD1CON1bits.CLRASAM = 0; //normal operation
    AD1CON1bits.ASAM = 1; //SAMP = 0, sampling

    AD1CON2bits.VCFG = 0; //AVDD-AVSS
    AD1CON2bits.CSCNA = 1; //scan
    AD1CON2bits.SMPI = 0b0010; //3th sample
    AD1CON2bits.BUFM = 0; //16-word buffer
    AD1CON2bits.ALTS = 0; //MUXA

    AD1CON3bits.ADRC = 0; //PBCLK
    AD1CON3bits.SAMC = 0x02; //sample time bits = 4TAD
    AD1CON3bits.ADCS = 0x07; //TAD = 16TPB

    AD1CON1bits.ON = 1;//start ADC module
}

void pressure()
{
	ADCValue0 = ADC1BUF0;
    ADCValue1 = ADC1BUF1;
	ADCValue2 = ADC1BUF2;

    double a0 = ADCValue0/1024.0*5.0;
    double a1 = ADCValue1/1024.0*5.0;
    double a2 = ADCValue2/1024.0*9.0;

    if (success_flag == 0) {
        if (a0 < 3) {
            pkt = 'A';
            U1ATXREG = pkt; 
            DelayMsec(1000);
        }
        if (a1 < 3) {
            pkt = 'C';
            U1ATXREG = pkt; 
            DelayMsec(1000);
        } 
        if (a1 >= 3) {
            pkt = 'D';
            U1ATXREG = pkt; 
            DelayMsec(1000);
        } 
        // if (a2 >= EMG_th) {
        //     pkt = 'C';
        //     U1ATXREG = pkt; 
        //     DelayMsec(1000);
        // }
        // if (a2 < EMG_th) {
        //     pkt = 'D';
        //     U1ATXREG = pkt; 
        //     DelayMsec(1000);
        // }
    }
}

/* initialize the LCD module */
void LCD_init()
{
    DelayMsec(15);   //wait for 15 ms
    RS = 0;          //send command
    Data = LCD_IDLE; //function set - 8 bit interface
    DelayMsec(5);    //wait for 5 ms
    Data = LCD_IDLE; //function set - 8 bit interface
    DelayUsec(100);  //wait for 100 us
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
    E = 0;    //producing falling edge on E
    E = 1;
    Data <<= 4; //sending lower nibble through higher 4 ports
    E = 0;      //producing falling edge on E
}
/* Display a string of characters *s by continuously calling LCD_putchar() */
void LCD_puts(const uchar *s)
{
    RS = 1;
    int str_length = strlen(s);
    int i;
    for (i = 0; i < str_length; i++) {
        LCD_putchar(s[i]);
        DelayUsec(40);
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
    TMR4 = 0x0;
    while (TMR4 < 10*num);
}
/* Call GenMsec() num times to generate num ms delay*/
void DelayMsec(int num)
{
    TMR4 = 0x0;
    while (TMR4 < 10000*num);
}




