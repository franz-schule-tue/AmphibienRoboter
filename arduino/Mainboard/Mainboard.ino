
#include <Wire.h>
#include <TFT_eSPI.h>

// Definition der Mikrocontroller-Pins
#define PIN_ANAIN_BATT  37

#define DISP_FONT_NORMAL     2
#define DISP_FROM_LEFT       10
#define DISP_FROM_TOP        2
#define DISP_CENTER          64


TFT_eSPI    disp;
int count = 0;
float vBatt = 0;



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
    return analogRead( PIN_ANAIN_BATT ) * 3.3 / 4095.0 * ( 4700.0 + 3300.0 ) / 3300.0;
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
}



void setup()
{
    initDisplay();
}


void loop()
{
    float vBattNew = measureBatt();
    if ( vBattNew != vBatt )
    {
        vBatt = vBattNew;
        displayBatt( vBatt );
    }
 
    delay( 10 );
}
