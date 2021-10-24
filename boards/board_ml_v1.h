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

/**
 * @file board_ml_v1.h
 * @author Marcel Licence
 * @date 22.09.2021
 *
 * @brief Board description for my first PCB
 *
 * @see My first PCB for arduino based synthesizer projects - a quick look at the HW for ESP32/DaisySeed - https://youtu.be/Lp65Urhy1-U
 * @see Testing my first PCB with the DaisySeed (STM32), prepare for more arduino based synthesizer projects - Testing my first PCB with the DaisySeed (STM32), prepare for more arduino based synthesizer projects
 */


#ifndef BOARDS_BOARD_ML_V1_H_
#define BOARDS_BOARD_ML_V1_H_

/*
 * MIDI In/Out
 */
#define MIDI_RX_PIN 35
#define MIDI_TX_PIN 34 /* only available on header */


/*
 * I2S Audio In/Out
 */
#define I2S_BCLK_PIN    32
#define I2S_WCLK_PIN    25
#define I2S_DOUT_PIN    33
// #define I2S_DIN_PIN      26 /* optional, extern, should be disabled when not used otherwise  */

/*
 * I2C
 */
#define I2C_SDA 21
#define I2C_SCL 22

/*
 * WS2812/LED
 */
#define WS2812_PIN  27
#define LED_STRIP_PIN   WS2812_PIN

/*
 * SPI with multiplexer
 */
#define SPI_CS  5
#define SPI_MOSI    23
#define SPI_MISO    19
#define SPI_SCK 18

/*
 * SPI multiplexing
 */
#define SPI_SEL_MCP23S17    0
#define SPI_SEL_DISPLAY1    1
#define SPI_SEL_DISPLAY2    2
#define SPI_SEL_EXT         3

/*
 * TFT additional pins
 */
#define TFT_DC  0
#define TFT_RST -1
#define TFT_CS  SPI_CS


/*
 * Analog
 */
#define ADC_MUL_SIG_PIN AMUL_SIG
#define AMUL_SIG    ADC1_CH0

#define PIN_KEY_ANALOG  ADC1_CH3

#define ADC1_CH0    36
#define ADC1_CH3    39 /* header only pin */




#endif /* BOARDS_BOARD_ML_V1_H_ */
