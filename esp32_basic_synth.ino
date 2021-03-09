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
#include <driver/i2s.h>
#include <WiFi.h>


/*
 * You can modify the sample rate as you want
 */
#define SAMPLE_RATE	48000


/*
 * this is more an experiment required for other data formats
 */
union sampleTUNT
{
    uint64_t sample;
    int32_t ch[2];
} sampleDataU;


void setup()
{
    /*
     * this code runs once
     */
    delay(500);

    Serial.begin(115200);

    Serial.println();

    Serial.printf("Initialize Synth Module\n");
    Synth_Init();
    Serial.printf("Initialize I2S Module\n");
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

}


/*
 * our main loop
 * - all is done in a blocking context
 * - do not block the loop otherwise you will get problems with your audio
 */
void loop()
{
    static uint32_t loop_cnt_1hz;
    static uint8_t loop_count_u8 = 0;

    loop_count_u8++;

    loop_cnt_1hz ++;
    if (loop_cnt_1hz >= SAMPLE_RATE)
    {
        Loop_1Hz();
    }

#ifdef I2S_NODAC
    if (writeDAC(l_sample))
    {
        l_sample = Synth_Process();
    }
#else
    if (i2s_write_sample_32ch2(sampleDataU.sample))  /* function returns always true / it blocks until samples are written to buffer */
    {
        float fl_sample, fr_sample;
        Synth_Process(&fl_sample, &fr_sample);

        sampleDataU.ch[0] = int32_t(fl_sample * 536870911.0f);
        sampleDataU.ch[1] = int32_t(fr_sample * 536870911.0f);
    }
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

