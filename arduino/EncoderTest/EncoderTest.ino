
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define PIN_LED         19
#define PIN_ENC1        9
#define PIN_ENC2        10


LiquidCrystal_I2C lcd( 0x27, 16, 2 );
volatile int count = 0;
int lastCount = -1;

void IRAM_ATTR isrEnc1()
{
    digitalWrite( PIN_LED, HIGH );
    
    if ( digitalRead( PIN_ENC1 ) == digitalRead( PIN_ENC2 ) )
    {
        ++count;
    }
    else
    {
        --count;
    }

    digitalWrite( PIN_LED, LOW );
}


void setup()
{
    pinMode( PIN_LED, OUTPUT );
    pinMode( PIN_ENC1, INPUT );
    pinMode( PIN_ENC2, INPUT );

    attachInterrupt( digitalPinToInterrupt( PIN_ENC1 ), isrEnc1, CHANGE );
    
    lcd.init();
    lcd.backlight();
    lcd.setCursor( 0, 0 );
    lcd.print( "Hello World!" );
}


void loop()
{
    if ( count != lastCount )
    {
        lastCount = count;
        
        lcd.setCursor( 0, 1 );
        lcd.print( lastCount );
        lcd.print( "   " );
    }
}
