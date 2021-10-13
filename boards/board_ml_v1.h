/*
 * board_ml_v1.h
 *
 *  Created on: 22.09.2021
 *      Author: PC
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
