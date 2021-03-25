/**
 * Klasse Motor für die Ansteuerung eines Motors
 */


class Motor
{
public:
    const int PWM_FREQ = 1000;
    const int PWM_RES  = 8;

    
    /**
     * Konstruktor
     */
    Motor( int pinPlus, int pinMinus, int pwmChannel )
    {
        m_pinPlus    = pinPlus;
        m_pinMinus   = pinMinus;
        m_pwmChannel = pwmChannel;
        
        pinMode( pinPlus,  OUTPUT );
        digitalWrite( pinPlus,  LOW );             // LOW: vorwärts, HIGH: rückwärts

        ledcSetup( pwmChannel, PWM_FREQ, PWM_RES ); 
        ledcAttachPin( pinMinus, pwmChannel );

        setPower( 0 );
    }


    /**
     * Setzt die Motorleistung
     * @param value     -100..100; höhere Werte werden abgeschnitten
     */
    void setPower( int value )
    {
        value = saturate( value );
        
        if ( value < 0 )
        {
            // rückwärts
            digitalWrite( m_pinPlus, HIGH );             // LOW: vorwärts, HIGH: rückwärts
    
            // PWM andersrum. Achtung: value ist negativ
            ledcWrite( m_pwmChannel, 255 - ( -value * 255 / 100 ) );
        }
        else
        {
            // vorwärts
            digitalWrite( m_pinPlus, LOW );             // LOW: vorwärts, HIGH: rückwärts
    
            // PWM richtigrum
            ledcWrite( m_pwmChannel, value * 255 / 100 );
        }
    }


private:
    static int saturate( int value )
    {
        int res = value;
           
        if ( value < -100 )
        {
            res = -100;
        }
        else if ( value > 100 )
        {
            res = 100;
        }
            
        return res;
    }


private:

    int m_pinPlus;
    int m_pinMinus;
    int m_pwmChannel;
 };
