#include "LCD.h"
#include <plib.h>
#include <sys/kmem.h>

#define SYS_FREQ (80000000)
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
unsigned int acc = 1000;
int pwm = 0;
int MAX_PWM = 8000;
int cnt = 0;
// int MAX_PWM = 0x2000;
int put_down_flag = 0;
void MCU_init(void);
void ADC_init(void);
void pressure(void);
void UART_init(void);
void CN_init(void);
void DMA_init(void);
void PWM_init(void);
void DelayMs(unsigned int msec);
void PWM_CTR(void);

/*
#pragma interrupt ADC_ISR ipl5 vector 27
void ADC_ISR()
{
	//AD1CON1bits.ON = 0;
	ADCValue0 = ADC1BUF0;
    ADCValue1 = ADC1BUF1;
	IFS1bits.AD1IF = 0;
	//mAD1ClearIntFlag();
    //IFS1CLR = 0x0002;
	//AD1CON1bits.ON = 1;
	//AD1CON1bits.SAMP = 1;
} 
*/
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
            // DCH0CONSET = 0x80;
            // if (DCH0INTbits.CHERIF || DCH0INTbits.CHTAIF || DCH0INTbits.CHBCIF) return;
            DelayMs(10);
            reset_flag = 1;
        }
    }
    IFS1CLR = 0x0001;
    IEC1SET = 0x0001;
}

#pragma interrupt UART_RX_ISR ipl4 vector 24
void UART_RX_ISR (void)
{
	//IEC0CLR = 0x08000000;
    IFS0CLR = 0x08000000;
    //step_reception = (uchar) U1ARXREG;
    DCH0CONSET = 0x80;
    DelayMsec(1000);
	if (step_reception == 'R') {
		LCD_goto(0x00);
    	RS = 1;
    	LCD_putchar(step_reception);
    	DelayMsec(1000);
	}

    if (step_reception == 'S') {
        // Maximal Step: 99
        total_step++;
        uchar stepStr[] = "Step:   ";
        stepStr[6] = total_step/10 + '0';
        stepStr[7] = total_step%10 + '0';
        LCD_goto(0x00);
        LCD_puts(stepStr);
        put_down_flag = 1;
    } else if (step_reception == 'K') {
        success_flag = 1;
        IEC0CLR = 0x00001000;
    }
	//IEC0SET = 0x08000000;
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
    DMA_init();
    PWM_init();
    LCD_init();
    uchar stepStr[] = "Step: 00";
    LCD_goto(0x00);
    LCD_puts(stepStr);
    while(1){
        pressure();
        PWM_CTR();
        if (success_flag == 1) {
            uchar stepStr[] = "Step:   ";
            stepStr[6] = total_step%10 + '0';
            stepStr[7] = (total_step/10) + '0';
            LCD_goto(0x00);
            LCD_puts(stepStr);
        }
        if (reset_flag == 1) {
            success_flag = 0;
            reset_flag = 0;
            time_result = 0;
            total_step = 0;
            pwm = 0;
            put_down_flag = 0;
        }
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
    DelayMsec(10);
    if (put_down_flag) {
        if (pwm <= MAX_PWM) {
            pwm += acc;
            // cnt++;
        }
    }
    if (pwm >= MAX_PWM) {
        pwm = 0;
        put_down_flag = 0;
    }
}

void DMA_init(void)
{
    IEC1CLR = 0x00010000;   // disable DMA channel 0 interrupts
    IFS1CLR = 0x00010000;   // clear existing DMA channel 0 interrupt flag
    DMACONSET = 0x00008000; // enable the DMA controller
    DCH0CON = 0x3;          // channel 0 disabled, priority 3, no chaining
    DCH0ECON = 0;           // no start or stop IRQ, no pattern match

    /* Setup the DMA transfer */
    DCH0SSA = KVA_TO_PA((void *) &U1ARXREG);        // Set the source pointer at the array address in memory
    DCH0DSA = KVA_TO_PA((void *) &step_reception);   // Set the destination pointer at the UART1A transmission buffer
    DCH0SSIZ = sizeof(step_reception);            // Set the source size as the size of the data array
    DCH0CSIZ = 1;                       // Each time, transfer 1 Byte
    DCH0DSIZ = 1;                       // Set the destination size as 1 Byte

    DCH0INTCLR = 0x00ff00ff; // clear existing events, disable all interrupts
    // DCH0CONSET = 0x80;       // turn channel on   
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

    //IFS1CLR = 0x0002; //clear flag
    //IPC6SET = 0x1e000000; //priority 7;subpriority 0
    //IEC1SET = 0x0002;
    AD1CON1bits.ON = 1;//start ADC module
}

void pressure()
{
    // asm("di");
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
            // DCH0CONSET = 0x80;
            // if (DCH0INTbits.CHERIF || DCH0INTbits.CHTAIF || DCH0INTbits.CHBCIF) return;
            DelayMs(1000);
        }
        if (a1 < 3) {
            pkt = 'B';
            U1ATXREG = pkt; 
            // DCH0CONSET = 0x80;
            // register int pollCnt;
            // if (DCH0INTbits.CHERIF || DCH0INTbits.CHTAIF || DCH0INTbits.CHBCIF) return;
            // pollCnt = 100;
            // while (pollCnt--);
            DelayMs(1000);
        }
        if (a2 >= 1.5) {
            pkt = 'C';
            U1ATXREG = pkt; 
            // DCH0CONSET = 0x80;
            // if (DCH0INTbits.CHERIF || DCH0INTbits.CHTAIF || DCH0INTbits.CHBCIF) return;
            DelayMs(1000);
        }
        if (a2 < 1.5) {
            pkt = 'D';
            U1ATXREG = pkt; 
            // DCH0CONSET = 0x80;
            // if (DCH0INTbits.CHERIF || DCH0INTbits.CHTAIF || DCH0INTbits.CHBCIF) return;
            DelayMs(1000);
            // register int pollCnt = 100;
            // while (pollCnt--);
        }
    }
    /*
    if (a0 < 3) {
        pkt = 'A'; U1ATXREG = pkt; 
        DelayMs(1000);
    }
    if (a1 < 3) {pkt = 'B'; U1ATXREG = pkt; DelayMs(1000);}
    if (a2 >= 1) {pkt = 'C'; U1ATXREG = pkt; DelayMs(1000);}
    if (a2 < 1) {pkt = 'D'; U1ATXREG = pkt; DelayMs(1000);}
    */
    // asm("ei");
}

void DelayMs(unsigned int msec)
{
    unsigned int tWait, tStart;
    tWait=(SYS_FREQ/2000)*msec;
    tStart=ReadCoreTimer();
    while((ReadCoreTimer()-tStart)<tWait); // wait for the time to pass
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




