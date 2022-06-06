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
 * @file esp32_audio_kit_module.ino
 * @author Marcel Licence
 * @date 12.10.2021
 *
 * @brief This file contains basic stuff to work with the ESP32 Audio Kit V2.2 module
 *
 * @see ESP32 Audio Kit AC101 codec failure - Get synthesizer projects working based on ES8388 - https://youtu.be/8UB3fYPjqSk
 * @see Instructions: http://myosuploads3.banggood.com/products/20210306/20210306011116instruction.pdf
 * @see Schematic: https://docs.ai-thinker.com/_media/esp32-audio-kit_v2.2_sch.pdf
 */


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


#ifdef ESP32_AUDIO_KIT

#ifdef AC101_ENABLED
#include "AC101.h" /* only compatible with forked repo: https://github.com/marcel-licence/AC101 */
#endif


//#define BUTTON_DEBUG_MSG



#define PIN_LED4    (22)
#define PIN_LED5    (19)


#ifdef AUDIO_KIT_BUTTON_DIGITAL
/*
 * when not modified and R66-R70 are placed on the board
 */
#define PIN_KEY_1                   (36)
#define PIN_KEY_2                   (13)
#define PIN_KEY_3                   (19)
#define PIN_KEY_4                   (23)
#define PIN_KEY_5                   (18)
#define PIN_KEY_6                   (5)

#define PIN_PLAY                    PIN_KEY_4
#define PIN_VOL_UP                  PIN_KEY_5
#define PIN_VOL_DOWN                PIN_KEY_6
#endif

#ifdef AUDIO_KIT_BUTTON_ANALOG
/*
 * modification required:
 * - remove R66-R70
 * - insert R60-R64 (0 Ohm or solder bridge)
 * - insert R55-R59 using 1.8kOhm (recommended but other values might be possible with tweaking the code)
 */
#ifndef PIN_KEY_ANALOG
#define PIN_KEY_ANALOG              (36)
#endif

#define KEY_SETTLE_VAL  9 /* use higher value if anaog buton detection is unstable */

uint32_t keyMin[7] = {4095 - 32, 0, 462 - 32, 925 - 32, 1283 - 32, 1570 - 32, 1800 - 32};
uint32_t keyMax[7] = {4095 + 32, 0 + 32, 525 + 32, 1006 + 32, 1374 + 32, 1570 + 32, 1800 + 32 };
#endif

#define OUTPUT_PIN 0
#define MCLK_CH 0
#define PWM_BIT 1

#ifdef AC101_ENABLED
static AC101 ac;
#endif

/* actually only supporting 16 bit */
#define SAMPLE_SIZE_16BIT
//#define SAMPLE_SIZE_24BIT
//#define SAMPLE_SIZE_32BIT

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 44100
#endif
#define CHANNEL_COUNT   2
#define WORD_SIZE   16
#define I2S1CLK (512*SAMPLE_RATE)
#define BCLK    (SAMPLE_RATE*CHANNEL_COUNT*WORD_SIZE)
#define LRCK    (SAMPLE_RATE*CHANNEL_COUNT)


typedef void(*audioKitButtonCb)(uint8_t, uint8_t);
extern audioKitButtonCb audioKitButtonCallback;

#ifdef AC101_ENABLED
/*
 * this function could be used to set up the masterclock
 * it is not necessary to use the ac101
 */
void ac101_mclk_setup()
{
    // Put a signal out on pin
    uint32_t freq = SAMPLE_RATE * 512; /* The maximal frequency is 80000000 / 2^bit_num */
    Serial.printf("Output frequency: %d\n", freq);
    ledcSetup(MCLK_CH, freq, PWM_BIT);
    ledcAttachPin(OUTPUT_PIN, MCLK_CH);
    ledcWrite(MCLK_CH, 1 << (PWM_BIT - 1)); /* 50% duty -> The available duty levels are (2^bit_num)-1, where bit_num can be 1-15. */
}

/*
 * complete setup of the ac101 to enable in/output
 */
void ac101_setup()
{
    Serial.printf("Connect to AC101 codec... ");
    while (not ac.begin(AC101_PIN_SDA, AC101_PIN_SCL))
    {
        Serial.printf("Failed!\n");
        delay(1000);
    }
    Serial.printf("OK\n");
#ifdef SAMPLE_SIZE_24BIT
    ac.SetI2sWordSize(AC101::WORD_SIZE_24_BITS);
#endif
#ifdef SAMPLE_SIZE_16BIT
    ac.SetI2sWordSize(AC101::WORD_SIZE_16_BITS);
#endif

#if (SAMPLE_RATE==44100)&&(defined(SAMPLE_SIZE_16BIT))
    ac.SetI2sSampleRate(AC101::SAMPLE_RATE_44100);
    /*
     * BCLK: 44100 * 2 * 16 = 1411200 Hz
     * SYSCLK: 512 * fs = 512* 44100 = 22579200 Hz
     *
     * I2S1CLK/BCLK1 -> 512 * 44100 / 44100*2*16
     * BCLK1/LRCK -> 44100*2*16 / 44100 Obacht ... ein clock cycle goes high and low
     * means 32 when 32 bits are in a LR word channel * word_size
     */
    ac.SetI2sClock(AC101::BCLK_DIV_16, false, AC101::LRCK_DIV_32, false);
    ac.SetI2sMode(AC101::MODE_SLAVE);
    ac.SetI2sWordSize(AC101::WORD_SIZE_16_BITS);
    ac.SetI2sFormat(AC101::DATA_FORMAT_I2S);
#endif

    ac.SetVolumeSpeaker(3);
    ac.SetVolumeHeadphone(99);

#if 1
    ac.SetLineSource();
#else
    ac.SetMicSource(); /* handle with care: mic is very sensitive and might cause feedback using amp!!! */
#endif

#if 0
    ac.DumpRegisters();
#endif

    // Enable amplifier
#if 0 /* amplifier only required when speakers attached? */
    pinMode(GPIO_PA_EN, OUTPUT);
    digitalWrite(GPIO_PA_EN, HIGH);
#endif
}
#endif /* #ifdef AC101_ENABLED */

/*
 * pullup required to enable reading the buttons (buttons will connect them to ground if pressed)
 */
void button_setup()
{
#ifdef AUDIO_KIT_BUTTON_DIGITAL
    // Configure keys on ESP32 Audio Kit board
    pinMode(PIN_PLAY, INPUT_PULLUP);
    pinMode(PIN_VOL_UP, INPUT_PULLUP);
    pinMode(PIN_VOL_DOWN, INPUT_PULLUP);
#endif
#ifdef AUDIO_KIT_BUTTON_ANALOG_OLD
    adcAttachPin(PIN_KEY_ANALOG);
    analogReadResolution(10);
    analogSetAttenuation(ADC_11db);
#endif
}

#ifdef AC101_ENABLED
/*
 * selects the microphone as audio source
 * handle with care: mic is very sensitive and might cause feedback using amp!!!
 */
void ac101_setSourceMic(void)
{
    ac.SetMicSource();
}

/*
 * selects the line in as input
 */
void ac101_setSourceLine(void)
{
    ac.SetLineSource();
}
#endif /* #ifdef AC101_ENABLED */

/*
 * very bad implementation checking the button state
 * there is some work required for a better functionality
 */
void button_loop()
{
#ifdef AUDIO_KIT_BUTTON_DIGITAL
    if (digitalRead(PIN_PLAY) == LOW)
    {
        Serial.println("PIN_PLAY pressed");
        if (buttonMapping.key4_pressed != NULL)
        {
            buttonMapping.key4_pressed();
        }
    }
    if (digitalRead(PIN_VOL_UP) == LOW)
    {
        Serial.println("PIN_VOL_UP pressed");
        if (buttonMapping.key5_pressed != NULL)
        {
            buttonMapping.key5_pressed();
        }
    }
    if (digitalRead(PIN_VOL_DOWN) == LOW)
    {
        Serial.println("PIN_VOL_DOWN pressed");
        if (buttonMapping.key6_pressed != NULL)
        {
            buttonMapping.key6_pressed();
        }
    }
#endif
#ifdef AUDIO_KIT_BUTTON_ANALOG
    static uint32_t lastKeyAD = 0xFFFF;
    static uint32_t keyAD = 0;
    static uint8_t pressedKey = 0;
    static uint8_t newPressedKey = 0;
    static uint8_t pressedKeyLast = 0;
    static uint8_t keySettle = 0;

    keyAD = analogRead(PIN_KEY_ANALOG);
    if (keyAD != lastKeyAD)
    {
        //Serial.printf("keyAd: %d\n", keyAD);
        lastKeyAD = keyAD;

        //pressedKey = 0;
        for (int i = 0; i < 7; i ++)
        {
            if ((keyAD >= keyMin[i]) && (keyAD < keyMax[i]))
            {
                newPressedKey = i;
            }
        }
        if (newPressedKey != pressedKey)
        {
            keySettle = KEY_SETTLE_VAL;
            pressedKey = newPressedKey;
        }
    }

    if (keySettle > 0)
    {
        keySettle--;
        if (keySettle == 0)
        {
            if (pressedKey != pressedKeyLast)
            {
                if (pressedKeyLast > 0)
                {
#ifdef BUTTON_DEBUG_MSG
                    Serial.printf("Key %d up\n", pressedKeyLast);
#endif
                    if (audioKitButtonCallback != NULL)
                    {
                        audioKitButtonCallback(pressedKeyLast - 1, 0);
                    }
                }
                if (pressedKey > 0)
                {
#ifdef BUTTON_DEBUG_MSG
                    Serial.printf("Key %d down\n", pressedKey);
#endif
                    if (audioKitButtonCallback != NULL)
                    {
                        audioKitButtonCallback(pressedKey - 1, 1);
                    }
                }
                pressedKeyLast = pressedKey;
            }
        }
    }
#endif
}

#endif

