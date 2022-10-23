/*
 * File:   Dice_Main.c
 * Author: josue
 *
 * Created on 8 de octubre de 2022, 11:25 PM
 */


// PIC16F877A Configuration Bit Settings

// 'C' source line config statements

// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)        Used for 8 and 16 MHz
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
//#include <pic16f877a.h>

#define _XTAL_FREQ 16000000 //Specify the XTAL crystall FREQ

unsigned char flag = 0;
int seed = 0, Dummy_PrevSeed = 0xAAAA;
unsigned char Dummy_PrevD1 = 3, Dummy_PrevD2 = 6;


/* 
 * To enable the interrupt function yo have to follow this structure
 * __interrupt(INTERRUPT_PRiORITY) void FUNTION_NAME()
 * {
 *      YOUR_CODE
 * }
 * 
 * NOTE: If you do not have enabled the interrupt priority
 * The interrupt priority is not necesary at __interrupt function
*/
__interrupt() void ExternalInterrupt()                      
{
    flag = !flag;                   // Starts or stops the "counter" of the displays
        
    if (flag)
    {
        T1CONbits.TMR1ON = 1;               // Starts the timer
    }
    else
    {
        T1CONbits.TMR1ON = 0;               // Stops the timer
        seed = (TMR1L | (TMR1H << 8));      // get the counts of the timer
    }
    
    INTCONbits.INTF = 0;            // Remember to Clear the flag, if not, the code stays at interrupt function
}

unsigned char Get_Number(int Seed)
{
    unsigned char Dice_Number = 0, D2 = 0, D1 = 0, temp = 0, OOR_flag = 1, OOR_Iteration = 1;
    temp = (Seed >> 8);                                             // Get the 8 upper bits
    temp = (Seed & 0xFF) & temp;                                    // Make an and with the lower bits
    temp = temp & 0b01110111;           // Delete 4th and 8th bit to have a range between 0-7 every 4 bits
    
    D2 = temp >> 4;                                                 // Get the number of Dice 2
    D1 = temp & 0b1111;                                             // Get the number of Dice 1
    
    // Exceptions to 0 and 7 numbers
    while ( OOR_flag )                                  // OutOfRange_flag
    {
        if(D1 == 0 || D1 == 7 || D2 == 0 || D2 == 7)
        {
            switch(OOR_Iteration)
            {
                case 0:                                             // Case 0 Sum Prev D1 and D2 to temp
                    temp = temp + Dummy_PrevD1 + Dummy_PrevD2;
                break;
                case 1:                                             // Case 1 Make and Xor with the prev Seed
                    temp = temp ^ Dummy_PrevSeed;               
                break;
                case 2:
                    temp = temp + (Dummy_PrevSeed & 0xF);
                break;
                case 3:
                    temp = temp + ((Dummy_PrevSeed >> 4) & 0xF);
            }
            OOR_Iteration == 3 ? OOR_Iteration = 0 : OOR_Iteration++;
        }
        else
        {
            Dummy_PrevSeed = Seed;
            Dummy_PrevD1 = D1;
            Dummy_PrevD2 = D2;
            OOR_flag = 0;
        }
        temp = temp & 0b01110111;
        D2 = temp >> 4;
        D1 = temp & 0b1111;        
    }
    
    Dice_Number = (D2 << 4) | D1;
    //PORTD = Dice_Number;
    return Dice_Number;
   
}

void main() //The main function
{
    OPTION_REGbits.INTEDG = 0;                      // Configuration of External Interrupt on falling edge of RB0/INT pin
    INTCONbits.GIE = 1;                             // Global Interrupt -  Enables all interrupts
    INTCONbits.INTE = 1;                            // External interrupt - Enables RB0/INT external interrupt
    
    T1CONbits.TMR1ON = 0;                           // Timer1 On bit - Stops Timer1
    T1CONbits.TMR1CS = 0b00;                        // Timer1 Clock Source - External clock
    T1CONbits.T1CKPS = 0b00;                        // Timer1 input Clock Prescaler - 1:1 prescaler value
    
    TRISD = 0x00;                                   //Instruct the MCU that the PORTD pins are used as Output.
    PORTD = 0x88;                                   //Make all output of PortD to 88

    TRISB = 0xFF;                                   // PORTB pins are used as Input

    unsigned char counter = 0, temp = 0, init = 0;
    while(1)
    {
        while(flag) //Get into the Infinie While loop
        {
            init = 1;
            counter == 0xF ? counter = 0 : counter++;
            temp = (counter << 4) | counter;
            PORTD = temp;
            __delay_ms(50); //Wait
        }
        if(init == 1)
        {
            temp = Get_Number(seed);
            PORTD = temp;
        }
    }
}