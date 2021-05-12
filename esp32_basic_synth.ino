/*
 * pinout of ESP32 DevKit found here:
 * https://circuits4you.com/2018/12/31/esp32-devkit-esp32-wroom-gpio-pinout/
 *
 * Author: Marcel Licence
 */


/*
 * required include files
 */
#include <arduino.h>
#include <WiFi.h>


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

void setup()
{
    /*
     * this code runs once
     */
    delay(500);

    Serial.begin(115200);

    Serial.println();

    Delay_Init();

    Serial.printf("Initialize Synth Module\n");
    Synth_Init();
    Serial.printf("Initialize I2S Module\n");

    // setup_reverb();

    Blink_Setup();

#ifdef ESP32_AUDIO_KIT
    ac101_setup();
#endif

    setup_i2s();
    Serial.printf("Initialize Midi Module\n");
    Midi_Setup();

    Serial.printf("Turn off Wifi/Bluetooth\n");
#if 0
    setup_wifi();
#else
    WiFi.mode(WIFI_OFF);
#endif

#ifndef ESP8266
    btStop();
    // esp_wifi_deinit();
#endif



    Serial.printf("ESP.getFreeHeap() %d\n", ESP.getFreeHeap());
    Serial.printf("ESP.getMinFreeHeap() %d\n", ESP.getMinFreeHeap());
    Serial.printf("ESP.getHeapSize() %d\n", ESP.getHeapSize());
    Serial.printf("ESP.getMaxAllocHeap() %d\n", ESP.getMaxAllocHeap());

    Serial.printf("Firmware started successfully\n");

#if 0 /* activate this line to get a tone on startup to test the DAC */
    Synth_NoteOn(64);
#endif
}


/*
 * use this if something should happen every second
 * - you can drive a blinking LED for example
 */
inline void Loop_1Hz(void)
{
    Blink_Process();
}


/*
 * our main loop
 * - all is done in a blocking context
 * - do not block the loop otherwise you will get problems with your audio
 */
float fl_sample, fr_sample;

void loop()
{
    static uint32_t loop_cnt_1hz;
    static uint8_t loop_count_u8 = 0;

    loop_count_u8++;

    loop_cnt_1hz ++;
    if (loop_cnt_1hz >= SAMPLE_RATE)
    {
        Loop_1Hz();
        loop_cnt_1hz = 0;
    }

#ifdef I2S_NODAC
    if (writeDAC(l_sample))
    {
        l_sample = Synth_Process();
    }
#else

#ifdef ESP32_AUDIO_KIT
    if (i2s_write_stereo_samples(&fl_sample, &fr_sample))
    {
        /* nothing for here */
    }
    Synth_Process(&fl_sample, &fr_sample);
    /*
     * process delay line
     */
    Delay_Process(&fl_sample, &fr_sample);
#else
    if (i2s_write_sample_32ch2(sampleDataU.sample))  /* function returns always true / it blocks until samples are written to buffer */
    {
        Synth_Process(&fl_sample, &fr_sample);
        /*
         * process delay line
         */
        Delay_Process(&fl_sample, &fr_sample);
        sampleDataU.ch[0] = int32_t(fl_sample * 536870911.0f);
        sampleDataU.ch[1] = int32_t(fr_sample * 536870911.0f);
    }
#endif

#endif

    /*
     * Midi does not required to be checked after every processed sample
     * - we divide our operation by 8
     */
    if (loop_count_u8 % 8 == 0)
    {
        Midi_Process();
    }
}

