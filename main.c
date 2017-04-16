/*PWM Roubatel et Glassey*/


#include <xc.h>
#include "test.h"

/**
 * Bits de configuration:
 */
#pragma config FOSC = INTIO67   // Osc. interne, A6 et A7 comme IO.
#pragma config IESO = OFF       // Pas d'osc. au démarrage.
#pragma config FCMEN = OFF      // Pas de monitorage de l'oscillateur.

// Nécessaires pour ICSP / ICD:
#pragma config MCLRE = EXTMCLR  // RE3 est actif comme master reset.
#pragma config WDTEN = OFF      // Watchdog inactif.
#pragma config LVP = OFF        // Single Supply Enable bits off.

typedef enum {
    AVANT = 0b01,
    ARRIERE = 0b10
} Direction;

/**
 * Indique la direction correspondante à la valeur du potentiomètre.
 * @param v Valeur du potentiomètre.
 * @return AVANT ou ARRIERE.
 */
Direction conversionDirection(unsigned char v) {
    return (v > 127) ? AVANT : ARRIERE;          // Si la consigne est strictement plus grande que 127 on retourne avant, sinon arrière
}

/**
 * Indique le cycle de travail PWM correspondant à la valeur du potentiomètre.
 * @param v Valeur du potentiomètre.
 * @return Cycle de travail du PWM.
 */
unsigned char conversionMagnitude(unsigned char v) {
    if(v < 127){                    // Si la consigne est strictement plus petite que 127...
        return (254-2*v);           // ... on retourne une valeur entre 0 et 254 
    }else if(v > 128){              // Si la consigne est strictement plus petite que 128...
        return(2*v-256);            // ... on retourne une valeur entre 0 et 254 
    }else{
    return 0;                       // si la consigne est égale à 127 ou 128, on retourne 0
    }
}

#ifndef TEST

/**
 * Initialise le hardware.
 */
static void hardwareInitialise() {
    
    ANSELA = 0x00;          // On désactive les entrées analogiques du port A
    ANSELB = 0x00;          // On désactive les entrées analogiques du port B
    ANSELC = 0x00;          // On désactive les entrées analogiques du port C
    
    TRISA = 0xFF;           // Port A configuré en entrée
    TRISB = 0xFF;           // Port B configuré en entrée
    TRISC = 0b11111000;     // Port C 8-4 configurés en entrées et 3-0 en sorties

    //Configurer l'entrée AN9 
    ANSELBbits.ANSB3 = 1; // Entrée 3 configurée en analog IN
    ADCON0bits.CHS = 9; // Acquisition sur l'entré AN9
    ADCON0bits.ADON = 1; // Convertissuer A/D activé
    ADCON2bits.ADCS = 0; // Source FOSC/4 et TAD de 4/FOSC => 4us NEW
    ADCON2bits.ACQT = 0; // La conversion démarre tout de suite NEW
    
    
    // Active le PWM sur CCP1:
    CCP1CONbits.P1M = 0; // Utilisation de la sortie P1A uniquement : PWM simple
    CCP1CONbits.CCP1M = 12; // La sortie A est active au niveau haut NEW val 12 avant 0b1100
    CCPTMRS0bits.C1TSEL = 0;  // Le PWM utilise le temporisateur 2
    
    T2CONbits.T2CKPS = 0;  //  Pas de diviseur de fréqu. pour timer 2
    T2CONbits.TMR2ON = 1;  //  Active le timer 2
    T2CONbits.T2OUTPS = 3; //  Divise la frequence des interruptions par 4
    PR2 = 250;             //  Période du timer 2 réglée sur pour avoir une période de 10 ms.
        
    PIE1bits.TMR2IE = 1; // Active les interruptions.
    IPR1bits.TMR2IP = 1; // En haute priorité.
    
    INTCONbits.GIEH = 1;    // Activation des interruptions de haute priorité
    INTCONbits.GIEL = 1;    // Activation des interruptions de basse priorité
    RCONbits.IPEN = 1;      // Activation des niveaux de priorités des interruptions
    
    
    
}

/**
 * Point d'entrée des interruptions.
 */
void low_priority interrupt interruptionsBassePriorite() {
    if (PIR1bits.TMR2IF) {
        PIR1bits.TMR2IF = 0;
        ADCON0bits.GO = 1;
    }
    
    if (PIR1bits.ADIF) {
        PIR1bits.ADIF = 0;
        PORTC = conversionDirection(ADRESH);
        CCPR1L = conversionMagnitude(ADRESH);
    }
}

/**
 * Point d'entrée pour l'émetteur de radio contrôle.
 */
void main(void) {
    hardwareInitialise();
    
    

    while(1);
}
#endif

#ifdef TEST
void testConversionMagnitude() {
    testeEgaliteEntiers("CM01", conversionMagnitude(0),   254);
    testeEgaliteEntiers("CM02", conversionMagnitude(1),   252);
    testeEgaliteEntiers("CM03", conversionMagnitude(2),   250);
    
    testeEgaliteEntiers("CM04", conversionMagnitude(125),   4);
    testeEgaliteEntiers("CM05", conversionMagnitude(126),   2);
    
    testeEgaliteEntiers("CM06", conversionMagnitude(127),   0);
    testeEgaliteEntiers("CM07", conversionMagnitude(128),   0);

    testeEgaliteEntiers("CM08", conversionMagnitude(129),   2);
    testeEgaliteEntiers("CM09", conversionMagnitude(130),   4);
    
    testeEgaliteEntiers("CM10", conversionMagnitude(253), 250);
    testeEgaliteEntiers("CM11", conversionMagnitude(254), 252);
    testeEgaliteEntiers("CM12", conversionMagnitude(255), 254);
}
void testConversionDirection() {
    testeEgaliteEntiers("CD01", conversionDirection(  0), ARRIERE);    
    testeEgaliteEntiers("CD02", conversionDirection(  1), ARRIERE);    
    testeEgaliteEntiers("CD03", conversionDirection(127), ARRIERE);    
    testeEgaliteEntiers("CD04", conversionDirection(128), AVANT);
    testeEgaliteEntiers("CD05", conversionDirection(129), AVANT);
    testeEgaliteEntiers("CD06", conversionDirection(255), AVANT);    
}
void main() {
    initialiseTests();
    testConversionMagnitude();
    testConversionDirection();
    finaliseTests();
    while(1);
}
#endif
