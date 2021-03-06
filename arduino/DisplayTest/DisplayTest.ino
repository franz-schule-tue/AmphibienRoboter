
#include <Wire.h>
#include <TFT_eSPI.h>

// Definition der Mikrocontroller-Pins


TFT_eSPI    disp;
int         count = 0;

void setup()
{
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
    String str("000"+String(++count));
    disp.drawString( str.substring(str.length() - 4), 0, 100, 7);
    delay( 10 );
}
