/**
 * Klasse USSensor für die Ansteuerung eines SR04-Ultraschallsensors
 */

#define US_SENSOR_MAX_COUNT     2
 

class USSensor
{
public:

    /**
     * Initialisiert die Daten und die Hardware
     */
    void init( int pinTrigger )
    {
        m_pinTrigger = pinTrigger;
        pinMode( pinTrigger,  OUTPUT );
    }


    void setDistance( int cm )
    {
        if ( cm != m_lastMeasurement )
        {
            m_hasChanged = true;
            m_lastMeasurement = cm;
        }
    }
    

    int getDistance()
    {
        // Änderungs-Flag zurücksetzen, denn jetzt ist es kein neuer Messwert mehr
        m_hasChanged = false;
        
        return m_lastMeasurement;
    }
    

    /**
     * Gibt zurück, ob sich der Messwert seit der letzten Abfrage mit getDistance() geändert hat
     */
    bool hasChanged()
    {
        return m_hasChanged;
    }
    
    
    void trigger()
    {
        digitalWrite( m_pinTrigger, HIGH );
        delayMicroseconds( 10 );
        digitalWrite( m_pinTrigger, LOW );
    }
    
    
private:

    int     m_pinTrigger;
    int     m_lastMeasurement;
    bool    m_hasChanged;
};



static void IRAM_ATTR isrUsEcho();
static void IRAM_ATTR isrUsTimer();


class USSensorControl
{
public:

    USSensorControl( int pinEcho, int timerNr )
    {
        // es darf nur ein Objekt geben, also machen wir das nur beim ersten Objekt
        if ( m_instance == nullptr )
        {
            m_instance = this;
            m_pinEcho = pinEcho;

            pinMode( pinEcho, INPUT );
            attachInterrupt( digitalPinToInterrupt( pinEcho ), isrUsEcho, CHANGE );

            // Timer erstellen und initialisieren:
            // - Vorteiler (Prescaler) auf 80. Bei einer Taktfrequenz des Mikrocontrollers von 80MHz ergibt das eine Taktfrequenz des Timers von 1MHz
            // - hochzählen
            m_timer = timerBegin( timerNr, 80, true );
            
            // erstmal nicht laufen lassen, denn das machen wir später, wenn wir die Verzögerung brauchen
            timerStop( m_timer );
            
            // mit Interrupt-Funktion verknüpfen
            timerAttachInterrupt( m_timer, isrUsTimer, true );

            // Timer-Interrupt freigeben
            timerAlarmEnable( m_timer );
        }
    }


    /**
     * Rückgabe des einen und einzigen US-Sensor-Control-Objekts
     */
    static USSensorControl* getInstance()
    {
        return m_instance;
    }


    /**
     * Rückgabe eines weiteren Sensor-Objektes. Genau genommen wird es hier nicht erzeugt, sondern einfach das nächste noch nicht verwendete zurückgeliefert.
     */
    USSensor* createSensor( int pinTrigger )
    {
        if ( m_sensorCount < US_SENSOR_MAX_COUNT )
        {
            m_sensors[ m_sensorCount ].init( pinTrigger );
            return &m_sensors[ m_sensorCount++ ];
        }

        // Fehler: es ist kein Objekt mehr verfügbar, also einen Null-Pointer zurückliefern
        return nullptr;
    }


    /**
     * Rückgabe des Echo-Pins. Wird von der Interrupt-Funktion benötigt, um die steigende/fallende Flanke zu erkennen
     */
    int getPinEcho()
    {
        return m_pinEcho;
    }


    /**
     * Bescheid geben, dass der Echo-Impuls begonnen hat.
     * Hier müssen wir uns nur die aktuelle Zeit merken (Mikrosekunden seit Systemstart)
     */
    void notifyStart()
    {
        // nur dann ausführen, wenn wirklich getriggert wurde, sonst sind es Fehlimpulse
        if ( m_hasTriggered )
        {
            m_startTime = esp_timer_get_time();
        }
    }


    /**
     * Bescheid geben, dass der Echo-Impuls geendet hat.
     * Aus der Zeitdifferenz zwischen jetzt und dem gespeicherten Startzeitpunkt die Entfernung berechnen 
     */
    void notifyStop()
    {
        // nur dann ausführen, wenn wirklich getriggert wurde, sonst sind es Fehlimpulse
        if ( m_hasTriggered )
        {
            // OK, Impuls fertig, also Trigger-Flag zurücksetzen
            m_hasTriggered = false;
            
            // kleiner Trick: statt "Zeitdifferenz / 2 / 29.4" rechnen wir mit dem Zehnfachen,
            //                also  "Zeitdifferenz * 5 / 294", um das Rechnen mit Fließkommazahlen zu vermeiden:
            
            int64_t cm = ( ( esp_timer_get_time() - m_startTime ) * 5 ) / 294;
    
            // berechneten Abstand an das Sensor-Objekt weitergeben, damit es später von dort abgefragt werden kann
            m_sensors[ m_currSensorIdx ].setDistance( (int) cm );
    
            // nächsten Sensor vormerken
            if ( ++m_currSensorIdx >= US_SENSOR_MAX_COUNT )
            {
                m_currSensorIdx = 0;
            }
    
            // Bevor wir den nächsten Sensor triggern, warten wir kurz, um alle Echos loszuwerden. Dafür wird der Timer gestartet
            triggerTimer( 100000 );      // Mikrosekunden!
        }
    }


    /**
     * Bescheid geben, dass der Timer abgelaufen ist.
     * Es wird der aktuelle Sensor getriggert. 
     */
    void notifyTimer()
    {
        timerStop( m_timer ); 
        triggerCurrentSensor();
    }


    /**
     * Starten der "Kettenreaktion" durch Triggern des ersten Sensors
     */
    void start()
    {
        m_currSensorIdx = 0;
        triggerCurrentSensor();
    }
    

private:
    static USSensorControl* m_instance;
    USSensor                m_sensors[ US_SENSOR_MAX_COUNT ];
    int                     m_sensorCount = 0;
    int                     m_pinEcho;
    hw_timer_t*             m_timer;
    int                     m_currSensorIdx = 0;
    int64_t                 m_startTime;

    /** Wenn der Sensor kein Echo empfängt, ist die fallende Flanke des Echo-Ausgangs recht langsam,
     *  was zu Fehlauslösungen des Pin-Change-Interrupts führt. Über dieses Flag wird erreicht, dass nur 
     *  der jeweils erste Interrupt wirklich zählt und alle weiteren ignoriert werden.
     */
    bool                    m_hasTriggered = false;



    /**
     * Bereit machen und starten des Timers
     */
    void triggerTimer( uint64_t micros )
    {
        // Timer-Zähler auf 0 setzen
        timerRestart( m_timer );
        
        // Alarmzeit setzen
        timerAlarmWrite( m_timer, micros, true );
        
        // Timer laufen lassen
        timerStart( m_timer ); 
    }


    /**
     * Triggert den aktuell eingestellten Sensor durch Aufrufen dessen trigger()-Funktion
     */
    void triggerCurrentSensor()
    {
        m_sensors[ m_currSensorIdx ].trigger();
        m_hasTriggered = true;
    }
};


// Interrupt-Funktion für das Sensor-Echo
static void IRAM_ATTR isrUsEcho()
{
    USSensorControl* control = USSensorControl::getInstance();

    // wir wissen zwar, dass sich der Pin geändert hat, aber nun müssen wir schauen, ob er auf HIGH oder auf LOW gewechselt ist
    if ( digitalRead( control->getPinEcho() ) == HIGH )
    {
        // Pin geht hoch -> Messung starten
        control->notifyStart();
    }
    else
    {
        // Pin geht runter -> Echo angekommen, also Entfernung berechnen
        control->notifyStop();
    }
}


// Interrupt-Funktion für den Timer-Ablauf (Erreichen des Alarmwertes)
static void IRAM_ATTR isrUsTimer()
{
    USSensorControl::getInstance()->notifyTimer();
}
