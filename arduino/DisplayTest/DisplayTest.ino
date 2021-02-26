
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TFT_eSPI.h>

// Definition der Mikrocontroller-Pins


LiquidCrystal_I2C lcd( 0x27, 16, 2 );
TFT_eSPI          disp;


void setup()
{
    
    lcd.init();
    lcd.backlight();
    lcd.setCursor( 0, 0 );
    lcd.print( "Hello World! :-)" );

    disp.init();
    disp.fillScreen( TFT_BLACK );
    disp.drawLine( 0, 0, 50, 100, TFT_BLUE );
    disp.drawRect( 0, 0, 127, 159, TFT_RED );
    disp.drawString("Font1", 10, 2, 1);
    disp.drawString("Font2", 64, 2, 2);
    disp.drawString("Font4", 10, 25, 4);
    disp.drawString("006", 10, 60, 6);      // nur Zahlen
    disp.drawString("8234", 0, 100, 7);     // nur Zahlen; 7-Segment-Darstellung
}


void loop()
{
}
