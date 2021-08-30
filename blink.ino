/*
 * this file includes a simple blink task implementation
 *
 * Author: Marcel Licence
 */

#ifdef BLINK_LED_PIN

inline
void Blink_Setup(void)
{
    pinMode(BLINK_LED_PIN, OUTPUT);
}


inline
void Blink_Process(void)
{
    static bool ledOn = true;
    if (ledOn)
    {
        digitalWrite(BLINK_LED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    }
    else
    {
        digitalWrite(BLINK_LED_PIN, LOW);    // turn the LED off
    }
    ledOn = !ledOn;
}

#endif
