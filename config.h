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
 * @file config.h
 * @author Marcel Licence
 * @date 12.05.2021
 *
 * @brief This file contains the project configuration
 *
 * All definitions are visible in the entire project
 *
 * Put all your project settings here (defines, numbers, etc.)
 * configurations which are requiring knowledge of types etc.
 * shall be placed in z_config.ino (will be included at the end)
 */


#ifndef CONFIG_H_
#define CONFIG_H_


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


//#define NOTE_ON_AFTER_SETUP /* used to get a test tone without MIDI input. Can be deactivated */


#define SERIAL_BAUDRATE 115200

//#define OUTPUT_SAW_TEST /*!< enable this to test the codec only. Should result in a saw wav with length of SAMPLE_BFFER_SIZE samples */

/*
 * you can select one of the pre-defined boards
 * look into ML_SynthTools in ml_boards.h for more information
 * @see https://github.com/marcel-licence/ML_SynthTools
 */
//#define BOARD_ML_V1 /* activate this when using the ML PCB V1 */
//#define BOARD_ESP32_AUDIO_KIT_AC101 /* activate this when using the ESP32 Audio Kit v2.2 with the AC101 codec */
#define BOARD_ESP32_AUDIO_KIT_ES8388 /* activate this when using the ESP32 Audio Kit v2.2 with the ES8388 codec */
//#define BOARD_ESP32_DOIT /* activate this when using the DOIT ESP32 DEVKIT V1 board */

/* can be used to pass line in through audio processing to output */
//#define AUDIO_PASS_THROUGH

/* this changes latency but also speed of processing */
#define SAMPLE_BUFFER_SIZE 48

/* this will force using const velocity for all notes, remove this to get dynamic velocity */
#define MIDI_USE_CONST_VELOCITY

/* this variable defines the max length of the delay and also the memory consumption */
#define MAX_DELAY   (SAMPLE_RATE/2) /* 1/2s -> @ 44100 samples */

/* you can receive MIDI messages via serial-USB connection */
/*
 * you could use for example https://projectgus.github.io/hairless-midiserial/
 * to connect your MIDI device via computer to the serial port
 */
#define MIDI_RECV_FROM_SERIAL

/* activate MIDI via USB (please look into usbMidiHost.ino for more information) */
//#define MIDI_VIA_USB_ENABLED

/* use this to display a scope on the oled display */
//#define OLED_OSC_DISP_ENABLED

/*
 * keep in mind that activation of adc will also change your controls on startup!
 */
//#define ADC_TO_MIDI_ENABLED /* this will enable the adc module */
#define ADC_TO_MIDI_LOOKUP_SIZE 8 /* should match ADC_INPUTS */

#define ARP_MODULE_ENABLED /* allow using arp module */
#define MIDI_SYNC_MASTER /* turn this off to use external midi clock signal */
#define MIDI_CTRL_ENABLED /* used for virtual split point */


//#define MIDI_STREAM_PLAYER_ENABLED /* activate this to use the midi stream playback module */


/*
 * include the board configuration
 * there you will find the most hardware depending pin settings
 */
#include <ml_boards.h> /* requires library ML_SynthTools from https://github.com/marcel-licence/ML_SynthTools */

#ifdef BOARD_ML_V1
#elif (defined BOARD_ESP32_AUDIO_KIT_AC101)
#elif (defined BOARD_ESP32_AUDIO_KIT_ES8388)
#elif (defined BOARD_ESP32_DOIT)

#define MIDI_PORT2_ACTIVE
#define MIDI_RX2_PIN RXD2 /* U2RRXD */
#define MIDI_TX2_PIN TXD2

#else
/* there is room left for other configurations */

/*
 * DIN MIDI Pinout
 */
#define MIDI_PORT2_ACTIVE
#define MIDI_RX2_PIN 16 /* U2RRXD */
#define MIDI_TX2_PIN 17

#endif

/*
 * You can modify the sample rate as you want
 */
#ifdef ESP32_AUDIO_KIT
#define SAMPLE_RATE 44100
#define SAMPLE_SIZE_16BIT
#else
#define SAMPLE_RATE 48000
#define SAMPLE_SIZE_16BIT /* 32 bit seems not to work at the moment */
#endif


#endif /* CONFIG_H_ */

