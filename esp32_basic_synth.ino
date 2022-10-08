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
 * @file esp32_basic_synth.ino
 * @author Marcel Licence
 * @date 06.03.2021
 *
 * @brief   This is the main project file of the basic synthesizer
 *          It should be compatible with ESP32 and ESP8266
 *
 * pinout of ESP32 DevKit found here:
 * @see https://circuits4you.com/2018/12/31/esp32-devkit-esp32-wroom-gpio-pinout/
 */


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


#include "config.h"

/*
 * required include files
 * add also includes used for other modules
 * otherwise arduino generated declaration may cause errors
 */
#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>

/* requires the ml_Synth library */
#include <ml_arp.h>
#include <ml_reverb.h>
#include <ml_midi_ctrl.h>
#include <ml_delay.h>
#ifdef OLED_OSC_DISP_ENABLED
#include <ml_scope.h>
#endif


void setup()
{
    /*
     * this code runs once
     */
    delay(500);

    Serial.begin(SERIAL_BAUDRATE);

    Serial.println();

    Serial.printf("esp32_basic_synth  Copyright (c) 2022  Marcel Licence\n");
    Serial.printf("This program comes with ABSOLUTELY NO WARRANTY;\n");
    Serial.printf("This is free software, and you are welcome to redistribute it\n");
    Serial.printf("under certain conditions; \n");


    Serial.printf("Initialize Synth Module\n");
    Synth_Init();

#ifdef BLE_MIDI
    Serial.printf("Initialize MIDI over Bluetooth\n");
    BLE_setup();
#endif

    Serial.printf("Initialize I2S Module\n");

#ifdef BLINK_LED_PIN
    Blink_Setup();
#endif

    Audio_Setup();


    /*
     * Initialize reverb
     * The buffer shall be static to ensure that
     * the memory will be exclusive available for the reverb module
     */
    static float revBuffer[REV_BUFF_SIZE];
    Reverb_Setup(revBuffer);

    /*
     * Prepare a buffer which can be used for the delay
     */
#ifdef MAX_DELAY
    static int16_t *delBuffer1 = (int16_t *)malloc(sizeof(int16_t) * MAX_DELAY);
    static int16_t *delBuffer2 = (int16_t *)malloc(sizeof(int16_t) * MAX_DELAY);
    Delay_Init2(delBuffer1, delBuffer2, MAX_DELAY);
#endif

    /*
     * setup midi module / rx port
     */
    Midi_Setup();

#ifdef ARP_MODULE_ENABLED
    Arp_Init(24 * 4); /* slowest tempo one step per bar */
#endif

#ifdef ESP32
    Serial.printf("ESP.getFreeHeap() %d\n", ESP.getFreeHeap());
    Serial.printf("ESP.getMinFreeHeap() %d\n", ESP.getMinFreeHeap());
    Serial.printf("ESP.getHeapSize() %d\n", ESP.getHeapSize());
    Serial.printf("ESP.getMaxAllocHeap() %d\n", ESP.getMaxAllocHeap());

    Serial.printf("Total heap: %d\n", ESP.getHeapSize());
    Serial.printf("Free heap: %d\n", ESP.getFreeHeap());

    /* PSRAM will be fully used by the looper */
    Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());
#endif

    Serial.printf("Firmware started successfully\n");

#ifdef NOTE_ON_AFTER_SETUP /* activate this line to get a tone on startup to test the DAC */
    Synth_NoteOn(0, 64, 1.0f);
#endif

#ifdef MIDI_STREAM_PLAYER_ENABLED
    MidiStreamPlayer_Init();

    char midiFile[] = "/song.mid";
    MidiStreamPlayer_PlayMidiFile_fromLittleFS(midiFile, 1);
#endif

#if (defined ADC_TO_MIDI_ENABLED) || (defined MIDI_VIA_USB_ENABLED) || (defined OLED_OSC_DISP_ENABLED)
#ifdef ESP32
    Core0TaskInit();
#else
#error only supported by ESP32 platform
#endif
#endif
}

#ifdef ESP32
/*
 * Core 0
 */
/* this is used to add a task to core 0 */
TaskHandle_t Core0TaskHnd;

inline
void Core0TaskInit()
{
    /* we need a second task for the terminal output */
    xTaskCreatePinnedToCore(Core0Task, "CoreTask0", 8000, NULL, 999, &Core0TaskHnd, 0);
}

inline
void Core0TaskSetup()
{
    /*
     * init your stuff for core0 here
     */

#ifdef OLED_OSC_DISP_ENABLED
    ScopeOled_Setup();
#endif

#ifdef ADC_TO_MIDI_ENABLED
    AdcMul_Init();
#endif

#ifdef MIDI_VIA_USB_ENABLED
    UsbMidi_Setup();
#endif
}

#ifdef ADC_TO_MIDI_ENABLED
static uint8_t adc_prescaler = 0;
#endif

void Core0TaskLoop()
{
    /*
     * put your loop stuff for core0 here
     */
#ifdef ADC_TO_MIDI_ENABLED
#ifdef MIDI_VIA_USB_ENABLED
    adc_prescaler++;
    if (adc_prescaler > 15) /* use prescaler when USB is active because it is very time consuming */
#endif /* MIDI_VIA_USB_ENABLED */
    {
        adc_prescaler = 0;
        AdcMul_Process();
    }
#endif /* ADC_TO_MIDI_ENABLED */
#ifdef MIDI_VIA_USB_ENABLED
    UsbMidi_Loop();
#endif

#ifdef MCP23_MODULE_ENABLED
    MCP23_Loop();
#endif

#ifdef OLED_OSC_DISP_ENABLED
    ScopeOled_Process();
#endif
}

void Core0Task(void *parameter)
{
    Core0TaskSetup();

    while (true)
    {
        Core0TaskLoop();

        /* this seems necessary to trigger the watchdog */
        delay(1);
        yield();
    }
}
#endif /* ESP32 */

static uint32_t midiSyncCount = 0;

void Midi_SyncRecvd()
{
    midiSyncCount += 1;
}

void Synth_RealTimeMsg(uint8_t msg)
{
#ifndef MIDI_SYNC_MASTER
    switch (msg)
    {
    case 0xfa: /* start */
        Arp_Reset();
        break;
    case 0xf8: /* Timing Clock */
        Midi_SyncRecvd();
        break;
    }
#endif
}

#ifdef MIDI_SYNC_MASTER

#define MIDI_PPQ    24
#define SAMPLES_PER_MIN  (SAMPLE_RATE*60)

static float midi_tempo = 120.0f;

void MidiSyncMasterLoop(void)
{
    static float midiDiv = 0;
    midiDiv += SAMPLE_BUFFER_SIZE;
    if (midiDiv >= (SAMPLES_PER_MIN) / (MIDI_PPQ * midi_tempo))
    {
        midiDiv -= (SAMPLES_PER_MIN) / (MIDI_PPQ * midi_tempo);
        Midi_SyncRecvd();
    }
}

void Synth_SetMidiMasterTempo(uint8_t unused, float val)
{
    midi_tempo = 60.0f + val * (240.0f - 60.0f);
}

#endif

void Synth_SongPosition(uint16_t pos)
{
    Serial.printf("Songpos: %d\n", pos);
    if (pos == 0)
    {
        Arp_Reset();
    }
}

void Synth_SongPosReset(uint8_t unused, float var)
{
    if (var > 0)
    {
        Synth_SongPosition(0);
    }
}

/*
 * use this if something should happen every second
 * - you can drive a blinking LED for example
 */
inline void Loop_1Hz(void)
{
#ifdef BLINK_LED_PIN
    Blink_Process();
#endif
}


/*
 * our main loop
 * - all is done in a blocking context
 * - do not block the loop otherwise you will get problems with your audio
 */
static float fl_sample[SAMPLE_BUFFER_SIZE];
static float fr_sample[SAMPLE_BUFFER_SIZE];

void loop()
{
    static uint32_t loop_cnt_1hz;

    loop_cnt_1hz += SAMPLE_BUFFER_SIZE;
    if (loop_cnt_1hz >= SAMPLE_RATE)
    {
        Loop_1Hz();
        loop_cnt_1hz = 0;
    }

    /*
     * Midi does not required to be checked after every processed sample
     * - we divide our operation by 8
     */
    Midi_Process();

#ifdef MIDI_STREAM_PLAYER_ENABLED
    MidiStreamPlayer_Tick(SAMPLE_BUFFER_SIZE);
#endif

#ifdef MIDI_VIA_USB_ENABLED
    UsbMidi_ProcessSync();
#endif
#ifdef BLE_MIDI
    BleMidiProc();
#endif

#ifdef MIDI_SYNC_MASTER
    MidiSyncMasterLoop();
#endif

#ifdef ARP_MODULE_ENABLED
    Arp_Process(midiSyncCount);
    midiSyncCount = 0;
#endif

    /* zero buffer, otherwise you can pass trough an input signal */
    memset(fl_sample, 0, sizeof(fl_sample));
    memset(fr_sample, 0, sizeof(fr_sample));

#ifdef AUDIO_PASS_THROUGH
    Audio_Input(left, right);
#endif

    Synth_Process(fl_sample, fr_sample, SAMPLE_BUFFER_SIZE);

    /*
     * process delay line
     */
#ifdef MAX_DELAY
    Delay_Process_Buff2(fl_sample, fr_sample, SAMPLE_BUFFER_SIZE);
#endif

    /*
     * add some mono reverb
     */
    Reverb_Process(fl_sample, SAMPLE_BUFFER_SIZE);
    memcpy(fr_sample,  fl_sample, sizeof(fr_sample));

    /*
     * Output the audio
     */
    Audio_Output(fl_sample, fr_sample);

#ifdef OLED_OSC_DISP_ENABLED
    ScopeOled_AddSamples(fl_sample, fr_sample, SAMPLE_BUFFER_SIZE);
#endif
}

/*
 * Callbacks
 */
void MidiCtrl_Cb_NoteOn(uint8_t ch, uint8_t note, float vel)
{
    Arp_NoteOn(ch, note, vel);
}

void MidiCtrl_Cb_NoteOff(uint8_t ch, uint8_t note)
{
    Arp_NoteOff(ch, note);
}

void MidiCtrl_Status_ValueChangedIntArr(const char *descr, int value, int index)
{
    Status_ValueChangedIntArr(descr, value, index);
}

void Arp_Cb_NoteOn(uint8_t ch, uint8_t note, float vel)
{
    Synth_NoteOn(ch, note, vel);
}

void Arp_Cb_NoteOff(uint8_t ch, uint8_t note)
{
    Synth_NoteOff(ch, note);
}

void Arp_Status_ValueChangedInt(const char *msg, int value)
{
    Status_ValueChangedInt(msg, value);
}

void Arp_Status_LogMessage(const char *msg)
{
    Status_LogMessage(msg);
}

void Arp_Status_ValueChangedFloat(const char *msg, float value)
{
    Status_ValueChangedFloat(msg, value);
}

void Arp_Cb_Step(uint8_t step)
{
    /* ignore */
}

/*
 * MIDI via USB Host Module
 */
#ifdef MIDI_VIA_USB_ENABLED
void App_UsbMidiShortMsgReceived(uint8_t *msg)
{
    Midi_SendShortMessage(msg);
    Midi_HandleShortMsg(msg, 8);
}
#endif

/*
 * Test functions
 */
#if defined(I2C_SCL) && defined (I2C_SDA)
void  ScanI2C(void)
{
    Wire.begin(I2C_SDA, I2C_SCL);

    byte error, address;
    int nDevices;

    Serial.printf("Scanning...\nSDA: %d\nSCL: %d\n", I2C_SDA, I2C_SCL);

    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            Serial.print("I2C device found at address 0x");
            if (address < 16)
            {
                Serial.print("0");
            }
            Serial.print(address, HEX);
            Serial.println("  !");

            nDevices++;
        }
        else if (error == 4)
        {
            Serial.print("Unknown error at address 0x");
            if (address < 16)
            {
                Serial.print("0");
            }
            Serial.println(address, HEX);
        }
    }
    if (nDevices == 0)
    {
        Serial.println("No I2C devices found\n");
    }
    else
    {
        Serial.println("done\n");
    }
}
#endif

