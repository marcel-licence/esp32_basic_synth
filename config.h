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
