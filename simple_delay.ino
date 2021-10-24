/*
 * Copyright (c) 2021 Marcel Licence
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
 * @file simple_delay.ino
 * @author Marcel Licence
 * @date 04.10.2021
 *
 * @brief This is a simple implementation of a delay line
 * - level adjustable
 * - feedback
 * - length adjustable
 */


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


/* max delay can be changed but changes also the memory consumption */
#define MAX_DELAY   11100

/*
 * module variables
 */
float *delayLine_l;
float *delayLine_r;
float delayToMix = 0;
float delayFeedback = 0;
uint32_t delayLen = 11098;
uint32_t delayIn = 0;
uint32_t delayOut = 0;

void Delay_Init(void)
{
    delayLine_l = (float *)malloc(sizeof(float) * MAX_DELAY);
    if (delayLine_l == NULL)
    {
        Serial.printf("No more heap memory!\n");
    }
    delayLine_r = (float *)malloc(sizeof(float) * MAX_DELAY);
    if (delayLine_r == NULL)
    {
        Serial.printf("No more heap memory!\n");
    }
    Delay_Reset();
}

void Delay_Reset(void)
{
    for (int i = 0; i < MAX_DELAY; i++)
    {
        delayLine_l[i] = 0;
        delayLine_r[i] = 0;
    }
}

void Delay_Process(float *signal_l, float *signal_r)
{
    delayLine_l[delayIn] = *signal_l;
    delayLine_r[delayIn] = *signal_r;

    delayOut = delayIn + (1 + MAX_DELAY - delayLen);

    if (delayOut >= MAX_DELAY)
    {
        delayOut -= MAX_DELAY;
    }

    *signal_l += delayLine_l[delayOut] * delayToMix;
    *signal_r += delayLine_r[delayOut] * delayToMix;

    delayLine_l[delayIn] += delayLine_l[delayOut] * delayFeedback;
    delayLine_r[delayIn] += delayLine_r[delayOut] * delayFeedback;

    delayIn ++;

    if (delayIn >= MAX_DELAY)
    {
        delayIn = 0;
    }
}

void Delay_SetFeedback(uint8_t unused, float value)
{
    delayFeedback = value;
    Serial.printf("delay feedback: %0.3f\n", value);
}

void Delay_SetLevel(uint8_t unused, float value)
{
    delayToMix = value;
    Serial.printf("delay level: %0.3f\n", value);
}

void Delay_SetLength(uint8_t unused, float value)
{
    delayLen = (uint32_t)(((float)MAX_DELAY - 1.0f) * value);
    Serial.printf("delay length: %0.3fms\n", delayLen * (1000.0f / ((float)SAMPLE_RATE)));
}
