
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Definition der Mikrocontroller-Pins
#define PIN_LED          19
#define PIN_ENC_LEFT     9
#define PIN_ENC_RIGHT    10


LiquidCrystal_I2C lcd( 0x27, 16, 2 );

// Variablen, die im INterrupt verändert werden, müssen mit "volatile" deklariert werden, sonst werden die Änderungen evtl. nicht sichtbar
volatile int  countLeft  = 0;
volatile int  countRight = 0;
volatile bool changed    = false;


// Interrupt-Funktion für den linken Sensor
void IRAM_ATTR isrEncLeft()
{
    ++countLeft;
    changed = 1;
}


// Interrupt-Funktion für den rechten Sensor
void IRAM_ATTR isrEncRight()
{
    ++countRight;
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
    }
}
