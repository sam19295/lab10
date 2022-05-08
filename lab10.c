/*
 * File:   lab10.c
 * Author: Melanie Samayoa
 *
 * Created on 2 de mayo de 2022, 06:07 PM
 */

#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <stdint.h>  // Para poder usar los int de 8 bits

#define _XTAL_FREQ 1000000

// Variables


uint8_t ban;
uint8_t pot;

char seleccion;
char RX;
char pot2 [];

void setup (void);
void Print (unsigned char *word);
void desplegar( char selection);

// interrupciones

void __interrupt() isr (void)
{
    if(PIR1bits.RCIF){         
        RX = RCREG;
    }
    
    if (PIR1bits.ADIF)          
    {
         if (ADCON0bits.CHS == 0b0000){
             pot = ADRESH;
             
         }

        PIR1bits.ADIF = 0;
        
    }
    
}

// Ciclo principal

void main(void) {
    setup();
    while(1){
        
        if(ADCON0bits.GO == 0){             // No hay proceso de conversion
            
            ADCON0bits.GO = 1;              // Iniciamos proceso de conversi?n
        } 
        
        Print("\r *************  \r");
        Print("\r Bienvenido, ¿Qué accion desea realizar? \r");
        Print(" Presione 1 para leer el Potenciometro \r");
        Print(" Presione 2 para desplegar el valor ASCII\r");
        Print("\r *************  \r");

        ban = 1;

        while (ban) {

            while (PIR1bits.RCIF == 0); //Esperar a recibir dato
            
            seleccion = RX;

            switch (seleccion) {
                case ('1'):
                    Print("\r El valor del potenciometro es \r");
                    itoa(pot2,pot,10);
                    desplegar(pot2);
                    Print("\r Listo \r");
                    ban = 0;
                    break;

                case ('2'):
                    Print("\r Ingrese un caracter para mostrar en ASCII: ");
                    while (PIR1bits.RCIF == 0); //Esperar
                    PORTB = RX; //Pasar el valor al puerto A
                    desplegar(RX); //mostrar el caracter en la pantalla
                    Print("\r Listo \r");
                    ban = 0;
                    break;

                default:
                    Print("Error \r");
                    
            }
        }
    }      
    return;
}

// configuracionn
void setup (void){
    
    
    ANSEL = 0b00000001;         // AN0 como entrada anal?gica;;                  
    ANSELH = 0;                 //I/O digitales
    
    
    TRISB = 0;                  // PORTB como salida 
    PORTB = 0;                  // Limpiamos PORTB
    
    // Configuraci?n del oscilador
    OSCCONbits.IRCF = 0b0100;    // IRCF <2:0> -> 100 1 MHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    // Configuraciones de comunicacion serial
    //SYNC = 0, BRGH = 1, BRG16 = 1, SPBRG=25 <- Valores de tabla 12-5
    TXSTAbits.SYNC = 0;         // Comunicaci?n ascincrona (full-duplex)
    TXSTAbits.BRGH = 1;         // Baud rate de alta velocidad 
    BAUDCTLbits.BRG16 = 1;      // 16-bits para generar el baud rate
    
    SPBRG = 25;
    SPBRGH = 0;                 // Baud rate ~9600, error -> 0.16%
    
    RCSTAbits.SPEN = 1;         // Habilitamos comunicaci?n
    TXSTAbits.TX9 = 0;          // Utilizamos solo 8 bits
    TXSTAbits.TXEN = 1;         // Habilitamos transmisor
    RCSTAbits.CREN = 1;         // Habilitamos receptor
    
    //COnfiguraci?n del ADC
    ADCON1bits.ADFM = 0;        // Justificado a la izquierda
    ADCON1bits.VCFG0 = 0;       // Referencia en VDD
    ADCON1bits.VCFG1 = 0;       // Referencia en VSS
    
    ADCON0bits.ADCS = 0b01;     // ADCS <1:0> -> 01 FOSC/8
    ADCON0bits.CHS = 0b0000;    // CHS  <3:0> -> 0000 AN0
    
    ADCON0bits.ADON = 1;        // Encender ADC
    __delay_us(50);
     
    //Configuraci?n de las interrupciones
    INTCONbits.GIE = 1;         // Habilitamos interrupciones globales
    INTCONbits.PEIE = 1;        // Habilitamos interrupciones de los puertos
    
    PIE1bits.ADIE = 1;          // Habilitamos interrupciones del ADC
    PIE1bits.RCIE = 1;          // Habilitamos Interrupciones de recepci?n
    
    PIR1bits.ADIF = 0;          // Flag del ADC en 0
    

    
    
   
}
// funciones

void Print (unsigned char *word)
{
while (*word != '\0')
{
    while (TXIF != 1);
    TXREG = *word;
    *word++;
}
return;    
}

void desplegar( char seleccion) {
    while (TXSTAbits.TRMT == 0);
    TXREG = seleccion;
}