/*
 * The GNU GENERAL PUBLIC LICENSE (GNU GPLv3)
 *
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

/*
 * config.h
 *
 * Put all your project settings here (defines, numbers, etc.)
 * configurations which are requiring knowledge of types etc.
 * shall be placed in z_config.ino (will be included at the end)
 *
 *  Created on: 12.05.2021
 *      Author: Marcel Licence
 */

#ifndef CONFIG_H_
#define CONFIG_H_


//#define ESP32_AUDIO_KIT
#define BOARD_ML_V1

//#define FAKE_ORGAN
#define SAMPLE_BUFFER_SIZE 48

/* use following when you are using the esp32 audio kit v2.2 */
//#define ESP32_AUDIO_KIT /* project has not been tested on other hardware, modify on own risk */
//#define ES8388_ENABLED /* use this if the Audio Kit is equipped with ES8388 instead of the AC101 */

/* this will force using const velocity for all notes, remove this to get dynamic velocity */
#define MIDI_USE_CONST_VELOCITY

/* you can receive MIDI messages via serial-USB connection */
/*
 * you could use for example https://projectgus.github.io/hairless-midiserial/
 * to connect your MIDI device via computer to the serial port
 */
#define MIDI_RECV_FROM_SERIAL

/* activate MIDI via USB */
//#define MIDI_VIA_USB_ENABLED


/*
 * keep in mind that activation of adc will also change your controls on startup!
 */
//#define ADC_TO_MIDI_ENABLED /* this will enable the adc module */
#define ADC_TO_MIDI_LOOKUP_SIZE 8 /* should match ADC_INPUTS */


#ifdef ESP32_AUDIO_KIT
#include "./boards/board_audio_kit_ac101.h"
//#include "./boards/board_audio_kit_es8388.h"

/* on board led */
#define BLINK_LED_PIN     19 // IO19 -> D5

#ifdef ADC_TO_MIDI_ENABLED
#define ADC_INPUTS  8
#define ADC_MUL_S0_PIN  23
#define ADC_MUL_S1_PIN  18
#define ADC_MUL_S2_PIN  14
#define ADC_MUL_S3_PIN  5    /* <- not used, this has not been tested */
#define ADC_MUL_SIG_PIN 12
#endif

#ifdef ES8388_ENABLED
/* i2c shared with codec */
#define I2C_SDA 18
#define I2C_SCL 23
#endif

#elif (defined BOARD_ML_V1)
#include "./boards/board_ml_v1.h"

#else /* ESP32_AUDIO_KIT */

/* on board led */
#define BLINK_LED_PIN     2

/*
 * Define and connect your PINS to DAC here
 */

#ifdef I2S_NODAC
#define I2S_NODAC_OUT_PIN   22  /* noisy sound without DAC, add capacitor in series! */
#else
/*
 * pins to connect a real DAC like PCM5201
 */
#define I2S_BCLK_PIN    25
#define I2S_WCLK_PIN    27
#define I2S_DOUT_PIN    26
#endif

#ifdef ADC_TO_MIDI_ENABLED
#define ADC_INPUTS  8
#define ADC_MUL_S0_PIN  33
#define ADC_MUL_S1_PIN  32
#define ADC_MUL_S2_PIN  13
#define ADC_MUL_SIG_PIN 12
#endif

#endif /* ESP32_AUDIO_KIT */

/*
 * DIN MIDI Pinout
 */
#ifdef ESP32_AUDIO_KIT
#define MIDI_RX_PIN 22 /* U2RRXD */
#elif (defined BOARD_ML_V1)
/* defined by board */
#else
#define MIDI_RX_PIN 16 /* U2RRXD */
#define TXD2 17
#endif


/*
 * You can modify the sample rate as you want
 */

#ifdef ESP32_AUDIO_KIT
#define SAMPLE_RATE 44100
#define SAMPLE_SIZE_16BIT
#else
#define SAMPLE_RATE 48000
#define SAMPLE_SIZE_16BIT
#endif

#endif /* CONFIG_H_ */
