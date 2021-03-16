
#include <Wire.h>
#include <TFT_eSPI.h>
#include "motor.hpp"

// Definition der Mikrocontroller-Pins
#define PIN_ANAIN_BATT          37
#define PIN_ENC_LEFT            34
#define PIN_ENC_RIGHT           38
#define PIN_MOTOR_P_LEFT        15
#define PIN_MOTOR_M_LEFT        2
#define PIN_MOTOR_P_RIGHT       4
#define PIN_MOTOR_M_RIGHT       0
#define PWM_CHAN_LEFT           0
#define PWM_CHAN_RIGHT          1  
#define PWM_FREQ                1000
#define PWM_RES                 8          


#define DISP_FONT_NORMAL     2
#define DISP_FROM_LEFT       10
#define DISP_FROM_TOP        2
#define DISP_CENTER          64


TFT_eSPI    disp;
hw_timer_t* timer = nullptr;
Motor       motorLeft{  PIN_MOTOR_P_LEFT,  PIN_MOTOR_M_LEFT,  PWM_CHAN_LEFT,  0 };
Motor       motorRight{ PIN_MOTOR_P_RIGHT, PIN_MOTOR_M_RIGHT, PWM_CHAN_RIGHT, 1 };


int count = 0;
float vBatt = 0;

// Variablen, die im Interrupt verändert werden, müssen mit "volatile" deklariert werden, sonst werden die Änderungen evtl. nicht sichtbar
volatile int  countLeft  = 0;
volatile int  countRight = 0;
volatile int  lastCountLeft  = 0;
volatile int  lastCountRight = 0;
volatile int  speedLeft  = 0;
volatile int  speedRight = 0;
volatile bool changed    = true;


// Interrupt-Funktion (Interrupt Service Function = ISR) für den linken Sensor
void IRAM_ATTR isrEncLeft()
{
    ++countLeft;
}


// Interrupt-Funktion für den rechten Sensor
void IRAM_ATTR isrEncRight()
{
    ++countRight;
}


// Interrupt-Funktion für den Timer
void IRAM_ATTR isrTimer()
{
    speedLeft = ( countLeft - lastCountLeft );
    lastCountLeft = countLeft;

    speedRight = ( countRight - lastCountRight );
    lastCountRight = countRight;

    changed = true;
}




void initDisplay()
{
    disp.init();
    disp.fillScreen( TFT_BLACK );
    
    disp.drawString( "Akku   ",  DISP_FROM_LEFT, DISP_FROM_TOP, DISP_FONT_NORMAL );
    disp.drawString( "Abst. V",  DISP_FROM_LEFT, DISP_FROM_TOP + 1 * disp.fontHeight( DISP_FONT_NORMAL ), DISP_FONT_NORMAL );
    disp.drawString( "Abst. H",  DISP_FROM_LEFT, DISP_FROM_TOP + 2 * disp.fontHeight( DISP_FONT_NORMAL ), DISP_FONT_NORMAL );
    disp.drawString( "Imp.  L",  DISP_FROM_LEFT, DISP_FROM_TOP + 3 * disp.fontHeight( DISP_FONT_NORMAL ), DISP_FONT_NORMAL );
    disp.drawString( "Imp.  R",  DISP_FROM_LEFT, DISP_FROM_TOP + 4 * disp.fontHeight( DISP_FONT_NORMAL ), DISP_FONT_NORMAL );
    disp.drawString( "GeschwL",  DISP_FROM_LEFT, DISP_FROM_TOP + 5 * disp.fontHeight( DISP_FONT_NORMAL ), DISP_FONT_NORMAL );
    disp.drawString( "GeschwR",  DISP_FROM_LEFT, DISP_FROM_TOP + 6 * disp.fontHeight( DISP_FONT_NORMAL ), DISP_FONT_NORMAL );
}


float measureBatt()
{
    // so wäre es nach der Theorie:
    //return analogRead( PIN_ANAIN_BATT ) /** 3.3 / 4095.0 * ( 5600.0 + 3300.0 ) / 3300.0*/;

    // tatsächlich scheint der ESP32 laut Datenblatt keine so genaue interne Referenzspannung zu haben und auch
    // zu den höherne Werten hin etwas ungenauer zu sein. Daher haben wir die genauen Werte durch Kalibrierung ermittelt,
    // wobei wir sogar einen Offset beobachtet haben:
    return 0.00224966 * analogRead( PIN_ANAIN_BATT ) + 0.3233;
}


void displayBatt( float value )
{
    if ( value < 6.0 )
    {
        disp.setTextColor( TFT_BLACK, TFT_RED );
    }
    else if ( value < 6.5 ) 
    {
        disp.setTextColor( TFT_BLACK, TFT_ORANGE );
    }
    else if ( value < 7 ) 
    {
        disp.setTextColor( TFT_BLACK, TFT_YELLOW );
    }
    else
    {
        disp.setTextColor( TFT_BLACK, TFT_GREEN );
    }
    

    disp.drawString( String( value ), TFT_WIDTH / 2, DISP_FROM_TOP, DISP_FONT_NORMAL );

    disp.setTextColor( TFT_WHITE, TFT_BLACK );    
    disp.drawString( String( value ), DISP_FROM_LEFT, DISP_FROM_TOP + 10 + 7 * disp.fontHeight( DISP_FONT_NORMAL ), 4 );
}


void displayValue( int line, int value )
{
    disp.drawString( String( value ) + " ", TFT_WIDTH / 2, DISP_FROM_TOP + ( line - 1 ) * disp.fontHeight( DISP_FONT_NORMAL ), DISP_FONT_NORMAL );
}



void setup()
{
    initDisplay();

    pinMode( PIN_ENC_LEFT,  INPUT );
    pinMode( PIN_ENC_RIGHT, INPUT );

    // Pin-Change-Interrupts setzen und die Interrupt-Funktionen zuweisen
    attachInterrupt( digitalPinToInterrupt( PIN_ENC_LEFT  ), isrEncLeft,  CHANGE );
    attachInterrupt( digitalPinToInterrupt( PIN_ENC_RIGHT ), isrEncRight, CHANGE );
    
    // Timer erstellen und initialisieren:
    // - Timer0 verwenden
    // - Vorteiler (Prescaler) auf 80. Bei einer Taktfrequenz des Mikrocontrollers von 80MHz ergibt das eine Taktfrequenz des Timers von 1MHz
    // - hochzählen
    timer = timerBegin( 0, 80, true );

    // mit Interrupt-Funktion verknüpfen
    timerAttachInterrupt( timer, isrTimer, true );

    // Ende-Wert auf 1000000, also 1000ms setzen mit automatischer Wiederholung
    timerAlarmWrite( timer, 1000000, true );

    // Timer starten
    timerAlarmEnable( timer );
}


void loop()
{
    float vBattNew = measureBatt();
    if ( vBattNew != vBatt )
    {
        vBatt = vBattNew;
        displayBatt( vBatt ); 
    }

    if ( changed )
    {
        changed = false;

        displayValue( 4, countLeft );
        displayValue( 5, countRight );
        displayValue( 6, speedLeft * 60 / 96 );
        displayValue( 7, speedRight * 60 / 96 );
    }

    // Motorsteuerung weiterschalten
    motorLeft.process();
    motorRight.process();

    // Power des linken Motors anzeigen
    displayValue( 8, motorLeft.getPower() );
    
    delay( 100 );
}
