/*
 * This module is run adc with a multiplexer
 * tested with ESP32 Audio Kit V2.2
 * Only tested with 8 inputs
 *
 * Define your adc mapping in the lookup table
 *
 * Reference: https://youtu.be/l8GrNxElRkc
 *
    Copyright (C) 2021  Marcel Licence

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifdef __CDT_PARSER__
#include <cdt.h>
#endif

#ifdef ADC_TO_MIDI_ENABLED

struct adc_to_midi_s
{
    uint8_t ch;
    uint8_t cc;
};

struct adc_to_midi_mapping_s
{
    struct adc_to_midi_s *adcToMidi;
    int adcToMidiCount;
    void(*callback)(uint8_t ch, uint8_t cc, uint8_t value);
};

/*
 * the following structure is for directly linking an ADC input to
 * a control change with a specific channel number and control number
 *
 * adcToMidiLookUp shall be defined as an array with the size of ADC_TO_MIDI_LOOKUP_SIZE
 *
 * the entries are directly mapped to the ADC input
 *
 * example:
 * in case ADC_TO_MIDI_LOOKUP_SIZE = 4 you can define 4 entries.
 * struct adc_to_midi_s adcToMidiLookUp[ADC_TO_MIDI_LOOKUP_SIZE] =
 * {
 *     {0, 0x10},
 *     {0, 0x11},
 *     {0, 0x12},
 *     {1, 0x13},
 * };
 *
 * The ADC input 0 will be mapped to channel 0, control change number 0x10 (16).
 * The connected function should be defined in the MIDI map finally to get an effect.
 *
 * ADC input 1 will be mapped to channel 0, control change number 0x11 (17)
 * ADC input 2 will be mapped to channel 0, control change number 0x12 (18)
 * ADC input 3 will be mapped to channel 1, control change number 0x13 (19)
 *
 * Finally there is no direct linking from ADC to the synthesizer. It creates only the control change messages.
 */
extern struct adc_to_midi_s adcToMidiLookUp[]; /* definition in z_config.ino */
extern struct adc_to_midi_mapping_s adcToMidiMapping;

uint8_t lastSendVal[ADC_TO_MIDI_LOOKUP_SIZE];  /* define ADC_TO_MIDI_LOOKUP_SIZE in top level file */
#define ADC_INVERT
#define ADC_THRESHOLD       (1.0f/200.0f)
#ifdef ADC_MCP_CTRL_ENABLED
#define ADC_OVERSAMPLING   16
#else
#endif

//#define ADC_DYNAMIC_RANGE
//#define ADC_DEBUG_CHANNEL0_DATA

static float adcChannelValue[ADC_INPUTS];

void AdcMul_Init(void)
{
    for (int i = 0; i < ADC_INPUTS; i++)
    {
        adcChannelValue[i] = 0.5f;
    }

    memset(lastSendVal, 0xFF, sizeof(lastSendVal));

    analogReadResolution(10);
    analogSetAttenuation(ADC_11db);
#if 0
    analogSetCycles(1);
#endif
    analogSetClockDiv(1);

    adcAttachPin(ADC_MUL_SIG_PIN);

#ifndef ADC_MCP_CTRL_ENABLED
    pinMode(ADC_MUL_S0_PIN, OUTPUT);
#if ADC_INPUTS > 2
    pinMode(ADC_MUL_S1_PIN, OUTPUT);
#endif
#if ADC_INPUTS > 4
    pinMode(ADC_MUL_S2_PIN, OUTPUT);
#endif
#if ADC_INPUTS > 8
    pinMode(ADC_MUL_S3_PIN, OUTPUT);
#endif
#endif
}

void AdcMul_Process(void)
{
    static float readAccu = 0;
    static float adcMin = 0;//4000;
#ifdef ADC_MCP_CTRL_ENABLED
    static float adcMax = 16350;
#else
    static float adcMax = 420453;//410000;
#endif

#ifdef ADC_MCP_CTRL_ENABLED
    MCP23_Select(SPI_SEL_MCP23S17);
#endif
    for (int j = 0; j < ADC_INPUTS; j++)
    {
#ifdef ADC_MCP_CTRL_ENABLED
        MCP23_SelAdc(j);
#else
        digitalWrite(ADC_MUL_S0_PIN, ((j & (1 << 0)) > 0) ? HIGH : LOW);
#if ADC_INPUTS > 2
        digitalWrite(ADC_MUL_S1_PIN, ((j & (1 << 1)) > 0) ? HIGH : LOW);
#endif
#if ADC_INPUTS > 4
        digitalWrite(ADC_MUL_S2_PIN, ((j & (1 << 2)) > 0) ? HIGH : LOW);
#endif
#if ADC_INPUTS > 8
        digitalWrite(ADC_MUL_S3_PIN, ((j & (1 << 3)) > 0) ? HIGH : LOW);
#endif

        /* give some time for transition */
        delay(1);
#endif

        readAccu = 0;
#if 0
        adcStart(ADC_MUL_SIG_PIN);
        for (int i = 0 ; i < ADC_OVERSAMPLING; i++)
        {

            if (adcBusy(ADC_MUL_SIG_PIN) == false)
            {
                readAccu += adcEnd(ADC_MUL_SIG_PIN);
                adcStart(ADC_MUL_SIG_PIN);
            }
        }
        adcEnd(ADC_MUL_SIG_PIN);
#else
        for (int i = 0 ; i < ADC_OVERSAMPLING; i++)
        {
            readAccu += analogRead(ADC_MUL_SIG_PIN);
        }
#endif

#ifdef ADC_DYNAMIC_RANGE
        if (readAccu < adcMin - 0.5f)
        {
            adcMin = readAccu + 0.5f;
            Serial.printf("adcMin: %0.3f\n", readAccu);
        }

        if (readAccu > adcMax + 0.5f)
        {
            adcMax = readAccu - 0.5f;
            Serial.printf("adcMax: %0.3f\n", readAccu);
        }
#endif

        if (adcMax > adcMin)
        {
            /*
             * normalize value to range from 0.0 to 1.0
             */
            float readValF = (readAccu - adcMin) / ((adcMax - adcMin));
            readValF *= (1 + 2.0f * ADC_THRESHOLD); /* extend to go over thresholds */
            readValF -= ADC_THRESHOLD; /* shift down to allow go under low threshold */

            bool midiMsg = false;

            /* check if value has been changed */
            if (readValF > adcChannelValue[j] + ADC_THRESHOLD)
            {
                adcChannelValue[j] = (readValF - ADC_THRESHOLD);
                midiMsg = true;
            }
            if (readValF < adcChannelValue[j] - ADC_THRESHOLD)
            {
                adcChannelValue[j] = (readValF + ADC_THRESHOLD);
                midiMsg = true;
            }

            /* keep value in range from 0 to 1 */
            if (adcChannelValue[j] < 0.0f)
            {
                adcChannelValue[j] = 0.0f;
            }
            if (adcChannelValue[j] > 1.0f)
            {
                adcChannelValue[j] = 1.0f;
            }

            /* MIDI adoption */
            if (midiMsg)
            {
                uint32_t midiValueU7 = (adcChannelValue[j] * 127.999);
                if (j < ADC_TO_MIDI_LOOKUP_SIZE)
                {
#ifdef ADC_INVERT
                    uint8_t idx = (ADC_INPUTS - 1) - j;
#else
                    uint8_t idx = j;
#endif
                    if (lastSendVal[idx] != midiValueU7)
                    {
                        if (adcToMidiMapping.callback != NULL)
                        {
                            adcToMidiMapping.callback(adcToMidiLookUp[idx].ch, adcToMidiLookUp[idx].cc, midiValueU7);
                        }
                        // Midi_ControlChange(adcToMidiLookUp[idx].ch, adcToMidiLookUp[idx].cc, midiValueU7);
                        lastSendVal[idx] = midiValueU7;
                    }
                }
#ifdef ADC_DEBUG_CHANNEL0_DATA
                if (j == 0)
                {
                    float adcValFrac = (adcChannelValue[j] * 127.999) - midiValueU7;
                    Serial.printf("adcChannelValue[j]: %f -> %0.3f -> %0.3f-> %d, %0.3f\n", readAccu, readValF, adcChannelValue[j], midiValueU7, adcValFrac);
                }
#endif
            }
        }
    }
}

float *AdcMul_GetValues(void)
{
    return adcChannelValue;
}

#endif /* ADC_TO_MIDI_ENABLED */

