/*
 * this file includes a simple blink task implementation
 *
 * Author: Marcel Licence
 */

inline
void Blink_Setup(void)
{
    pinMode(LED_PIN, OUTPUT);
}


inline
void Blink_Process(void)
{
    static bool ledOn = true;
    if (ledOn)
    {
        digitalWrite(LED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    }
    else
    {
        digitalWrite(LED_PIN, LOW);    // turn the LED off
    }
    ledOn = !ledOn;
}
