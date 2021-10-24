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
 * @file board_audio_kit_es8388.h
 * @author Marcel Licence
 * @date 22.09.2021
 *
 * @brief Board description for the ESP32 Audio Kit with the ES8388
 */


#ifndef BOARDS_BOARD_AUDIO_KIT_ES8388_H_
#define BOARDS_BOARD_AUDIO_KIT_ES8388_H_


#define MIDI_RX_PIN 19
#define LED_STRIP_PIN         12

#define I2C_SDA 18
#define I2C_SCL 23

#define IIC_CLK                     32
#define IIC_DATA                    33

#define I2S_SDOUT   35
#define I2S_SDIN    25
#define I2S_LRCK    26
#define I2S_BCLK    27
#define I2S_MCLK    0

#define IIC_CLK 32
#define IIC_DATA 33

#define ES8388_ENABLED

#endif /* BOARDS_BOARD_AUDIO_KIT_ES8388_H_ */
