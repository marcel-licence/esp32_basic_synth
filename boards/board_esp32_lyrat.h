/*
 * board_esp32_lyrat.h
 *
 *  Created on: 05.12.2021
 *      Author: PC
 *      @see https://docs.espressif.com/projects/esp-adf/en/latest/design-guide/board-esp32-lyrat-v4.3.html
 */

#ifndef BOARDS_BOARD_ESP32_LYRAT_H_
#define BOARDS_BOARD_ESP32_LYRAT_H_


#define BLINK_LED_PIN   22 /* Green LED indicator */

#define ES8388_PIN_DOUT 35
#define ES8388_PIN_DIN  26
#define ES8388_PIN_LRCK 25
#define ES8388_PIN_SCLK 5
#define ES8388_PIN_MCLK 0

#define ES8388_PIN_SDA  18
#define ES8388_PIN_SCL  23

#define MIDI_RX_PIN 22



#endif /* BOARDS_BOARD_ESP32_LYRAT_H_ */
