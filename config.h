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

#define ESP32_AUDIO_KIT

#ifdef ESP32_AUDIO_KIT
/* on board led */
#define LED_PIN 	19 // IO19 -> D5
#else
/* on board led */
#define LED_PIN 	2

/*
 * Define and connect your PINS to DAC here
 */
#define I2S_BCLK_PIN	25
#define I2S_WCLK_PIN	27
#define I2S_DOUT_PIN	26
#endif

/*
 * You can modify the sample rate as you want
 */

#ifdef ESP32_AUDIO_KIT
#define SAMPLE_RATE	44100
#else
#define SAMPLE_RATE	48000
#endif

#define ADC_TO_MIDI_ENABLED
#define ADC_TO_MIDI_LOOKUP_SIZE	8

#endif /* CONFIG_H_ */
