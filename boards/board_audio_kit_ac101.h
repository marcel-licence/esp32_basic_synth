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
 * @file board_audio_kit_ac101.h
 * @author Marcel Licence
 * @date 22.09.2021
 *
 * @brief Board description for the ESP32 Audio Kit with the AC101
 * @see   https://www.makerfabs.com/desfile/files/ESP32-A1S%20Product%20Specification.pdf
 */


#ifndef BOARDS_BOARD_AUDIO_KIT_AC101_H_
#define BOARDS_BOARD_AUDIO_KIT_AC101_H_


#define BLINK_LED_PIN     19 // IO19 -> D5

//#define MIDI_RX_PIN 22 /* U2RRXD */
#define MIDI_RX_PIN 21
#define LED_STRIP_PIN         12

/* AC101 pins */
#define AC101_PIN_SDA               33
#define AC101_PIN_SCL               32

#define IIS_SCLK                    27
#define IIS_LCLK                    26
#define IIS_DSIN                    25
#define IIS_DSOUT                   35
/* #define IIS_MCLK 0 <- is not used here */

#define GPIO_PA_EN                  GPIO_NUM_21
#define GPIO_SEL_PA_EN              GPIO_SEL_21


#define ESP32_AUDIO_KIT
#define AC101_ENABLED


#ifdef ADC_TO_MIDI_ENABLED
#define ADC_INPUTS  8
#define ADC_MUL_S0_PIN  23
#define ADC_MUL_S1_PIN  18
#define ADC_MUL_S2_PIN  14
#define ADC_MUL_S3_PIN  5    /* <- not used, this has not been tested */
#define ADC_MUL_SIG_PIN 12
#endif


/* map selected pins to global */
#define I2S_BCLK_PIN    IIS_SCLK
#define I2S_WCLK_PIN    IIS_LCLK
#define I2S_DOUT_PIN    IIS_DSIN
#define I2S_DIN_PIN     IIS_DSOUT


#endif /* BOARDS_BOARD_AUDIO_KIT_AC101_H_ */
