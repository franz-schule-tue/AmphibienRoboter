
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Definition der Mikrocontroller-Pins
#define PIN_LED          19
#define PIN_ENC_LEFT     9
#define PIN_ENC_RIGHT    10


LiquidCrystal_I2C lcd( 0x27, 16, 2 );
hw_timer_t* timer = nullptr;


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

    changed = 1;
}




void setup()
{
    // Mikrocontroller-Pins als Ausgang oder Eingang definieren
    pinMode( PIN_LED,       OUTPUT );
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
    
    lcd.init();
    lcd.backlight();
    lcd.setCursor( 0, 0 );
}


void loop()
{
    if ( changed )
    {
        changed = false;
        
        lcd.setCursor( 0, 0 );
        lcd.print( countLeft );
        lcd.print( "  " );
        
        lcd.setCursor( 8, 0 );
        lcd.print( countRight );
        lcd.print( "  " );

        // Geschwindigkeitsausgabe von U/min
        lcd.setCursor( 0, 1 );
        lcd.print( speedLeft * 60 / 96 );
        lcd.print( "  " );
        
        lcd.setCursor( 8, 1 );
        lcd.print( speedRight * 60 / 96 );
        lcd.print( "  " );
    }
}
