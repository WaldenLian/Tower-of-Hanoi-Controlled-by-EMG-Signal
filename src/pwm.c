#include <p32xxxx.h>
#include <plib.h>

/* Global variables */
unsigned int acc = 40; //acc determines the acceleration rate

/* Function prototypes */
void initIntGlobal(void);
void initPWM(void);

int state = 1;  // 1: brighter
                // -1: darker
int pwm = 0;    // min: 0
                // max: 0xFFFF
int MAX_PWM = 0x0118;   // Should be larger than PR2
int EMG_STATE;

/* Change Notice ISR */
#pragma interrupt CN_ISR ipl7 vector 26
void CN_ISR (void)
{
    IEC1CLR = 0x0001;
    if (EMG_STATE != PORTDbits.RD6) {
        EMG_STATE = PORTDbits.RD6; // Clear the mismatch
		TMR3 = 0;
        while (TMR3 < 10000);
        if (PORTDbits.RD6) {
            if (pwm == 0 || pwm == MAX_PWM) {
                state *= -1; // Toggle the state (brighter/darker)
            }
        } else {
            if (state == 1) {
                pwm += acc;
            } else {
                pwm -= acc;
            }
        }
    }
    IFS1CLR = 0x0001;
    IEC1SET = 0x0001;
}

/* Timer2 ISR - handling OC-PWM module operations */
#pragma interrupt PWM_ISR ipl3 vector 8
void PWM_ISR(void)
{
    OC3RS = pwm;      //update duty cycle register
    IFS0CLR = 0x0100; //clear Timer 2 interrupt flag
}
 
/* Configure interrupt globally */
void initIntGlobal()
{
    INTCONbits.MVEC = 1; // Enable multiple vector interrupt
    asm("ei");           // Enable all interrupts

    // Configure RD0, 1, 2 as output
    TRISDbits.TRISD0 = 0;
    // TRISDbits.TRISD1 = 0;
    // TRISDbits.TRISD2 = 0;

    // Configure RD6, 7, 13 as output
    TRISDbits.TRISD6 = 1;
    // TRISDbits.TRISD7 = 1;
    // TRISDbits.TRISD13 = 1;
}

/* Initialize OC module and timer base - Timer 2 */
void initPWM()
{
    OC3CON = 0x0000; //stop OC3 module
    OC3RS = 0;       //initialize duty cycle register
    OC3R = 0;        //initialize OC1R register for the first time
    OC3CON = 0x0006; //OC3 16-bit, Timer 2, in PWM mode w/o FP
    PR2 = 0x00DF;    //PWM signal period = 0x100*1/PBCLK = 32 us $$$$$$$$$$
    //Thus, PWM Frequency = 32.25 kHz
    IFS0CLR = 0x00000100; //clear Timer 2 interrupt
    IEC0SET = 0x00000100; //enable Timer 2 interrupt
    IPC2SET = 0x0000000F; //Timer 2 interrupt priority 3, subpriority 3
    T2CONSET = 0x8000;    //start Timer 2
    OC3CONSET = 0x8000;   //enable OC3 module for PWM generation
}

void initCN()
{
    asm("di");
    CNCON = 0x8000;
    CNEN = 0x00008000; // RD6 => CNEN15
    CNPUE = 0x00008000;

    EMG_STATE = PORTDbits.RD6;

    IPC6SET = 0x001c0000;
    IPC6SET = 0x00030000;
    IFS1CLR = 0x0001;
    IEC1SET = 0x0001;
    asm("ei");
}

void initTimer()
{
    T3CON = 0x0030;
    TMR3 = 0x0;
    PR3 = 0xFFFF;
    T3CONSET = 0x8000;
}

/* main function */
int main()
{
    OSCSetPBDIV(OSC_PB_DIV_1); //configure PBDIV so PBCLK = SYSCLK
    initIntGlobal();
    initTimer();
    initPWM();
    initCN();
    while (1);
}
