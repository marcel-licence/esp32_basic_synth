/*
 * Copyright (c) 2022 Marcel Licence
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
 * der GNU General Public License, wie von der Free Software Foundation,
 * Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
 * veröffentlichten Version, weiter verteilen und/oder modifizieren.
 *
 * Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
 * OHNE JEDE GEWÄHR,; sogar ohne die implizite
 * Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
 * Siehe die GNU General Public License für weitere Einzelheiten.
 *
 * Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
 * Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
 */

/**
 * @file blink.ino
 * @author Marcel Licence
 * @date 12.05.2021
 *
 * @brief this file includes a simple blink task implementation
 */


#ifdef __CDT_PARSER__
#include "cdt.h"
#endif


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

void Blink_Fast(uint8_t cnt)
{
    delay(500);
    for (int i = 0; i < cnt; i++)
    {
        digitalWrite(BLINK_LED_PIN, HIGH);
        delay(50);
        digitalWrite(BLINK_LED_PIN, LOW);
        delay(200);
    }
}

void Blink_Slow(uint8_t cnt)
{
    delay(500);
    for (int i = 0; i < cnt; i++)
    {

        digitalWrite(BLINK_LED_PIN, HIGH);
        delay(200);
        digitalWrite(BLINK_LED_PIN, LOW);
        delay(100);
    }
}


#endif
