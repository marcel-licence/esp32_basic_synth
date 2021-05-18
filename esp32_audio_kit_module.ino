/*
 * this file contains basic stuff to work with the ESP32 Audio Kit V2.2 module
 *
 * Author: Marcel Licence
 */

#ifdef ESP32_AUDIO_KIT

/*
 * Instructions: http://myosuploads3.banggood.com/products/20210306/20210306011116instruction.pdf
 *
 * Schmatic: https://docs.ai-thinker.com/_media/esp32-audio-kit_v2.2_sch.pdf
 */

#include "AC101.h" /* only compatible with forked repo: https://github.com/marcel-licence/AC101 */

/* AC101 pins */
#define IIS_SCLK                    27
#define IIS_LCLK                    26
#define IIS_DSIN                    25
#define IIS_DSOUT                   35

#define IIC_CLK                     32
#define IIC_DATA                    33

#define GPIO_PA_EN                  GPIO_NUM_21
#define GPIO_SEL_PA_EN              GPIO_SEL_21


#define PIN_KEY_1					(36)
#define PIN_KEY_2					(13)
#define PIN_KEY_3					(19)
#define PIN_KEY_4					(23)
#define PIN_KEY_5					(18)
#define PIN_KEY_6					(5)


#define PIN_PLAY                    PIN_KEY_4
#define PIN_VOL_UP                  PIN_KEY_5
#define PIN_VOL_DOWN                PIN_KEY_6


#define OUTPUT_PIN 0
#define MCLK_CH	0
#define PWM_BIT	1

static AC101 ac;

/* actually only supporting 16 bit */
//#define SAMPLE_SIZE_16BIT
//#define SAMPLE_SIZE_24BIT
//#define SAMPLE_SIZE_32BIT

#define CHANNEL_COUNT	2
#define WORD_SIZE	16
#define I2S1CLK	(512*SAMPLE_RATE)
#define BCLK	(SAMPLE_RATE*CHANNEL_COUNT*WORD_SIZE)
#define LRCK	(SAMPLE_RATE*CHANNEL_COUNT)

/*
 * this function could be used to set up the masterclock
 * it is not necessary to use the ac101
 */
void ac101_mclk_setup()
{
    // Put a signal out on pin
    uint32_t freq = SAMPLE_RATE * 512; /* The maximal frequency is 80000000 / 2^bit_num */
    Serial.printf("Output frequency: %d\n", freq);
    ledcSetup(MCLK_CH, freq, PWM_BIT);
    ledcAttachPin(OUTPUT_PIN, MCLK_CH);
    ledcWrite(MCLK_CH, 1 << (PWM_BIT - 1)); /* 50% duty -> The available duty levels are (2^bit_num)-1, where bit_num can be 1-15. */
}

/*
 * complete setup of the ac101 to enable in/output
 */
void ac101_setup()
{
    Serial.printf("Connect to AC101 codec... ");
    while (not ac.begin(IIC_DATA, IIC_CLK))
    {
        Serial.printf("Failed!\n");
        delay(1000);
    }
    Serial.printf("OK\n");
#ifdef SAMPLE_SIZE_24BIT
    ac.SetI2sWordSize(AC101::WORD_SIZE_24_BITS);
#endif
#ifdef SAMPLE_SIZE_16BIT
    ac.SetI2sWordSize(AC101::WORD_SIZE_16_BITS);
#endif

#if (SAMPLE_RATE==44100)&&(defined(SAMPLE_SIZE_16BIT))
    ac.SetI2sSampleRate(AC101::SAMPLE_RATE_44100);
    /*
     * BCLK: 44100 * 2 * 16 =
     *
     * I2S1CLK/BCLK1 -> 512 * 44100 / 44100*2*16
     * BCLK1/LRCK -> 44100*2*16 / 44100 Obacht ... ein clock cycle goes high and low
     * means 32 when 32 bits are in a LR word channel * word_size
     */
    ac.SetI2sClock(AC101::BCLK_DIV_16, false, AC101::LRCK_DIV_32, false);
    ac.SetI2sMode(AC101::MODE_SLAVE);
    ac.SetI2sWordSize(AC101::WORD_SIZE_16_BITS);
    ac.SetI2sFormat(AC101::DATA_FORMAT_I2S);
#endif

    ac.SetVolumeSpeaker(3);
    ac.SetVolumeHeadphone(99);

#if 1
    ac.SetLineSource();
#else
    ac.SetMicSource(); /* handle with care: mic is very sensitive and might cause feedback using amp!!! */
#endif

#if 0
    ac.DumpRegisters();
#endif

    // Enable amplifier
    pinMode(GPIO_PA_EN, OUTPUT);
    digitalWrite(GPIO_PA_EN, HIGH);
}

/*
 * pullup required to enable reading the buttons (buttons will connect them to ground if pressed)
 */
void button_setup()
{
    // Configure keys on ESP32 Audio Kit board
    pinMode(PIN_PLAY, INPUT_PULLUP);
    pinMode(PIN_VOL_UP, INPUT_PULLUP);
    pinMode(PIN_VOL_DOWN, INPUT_PULLUP);
}

/*
 * selects the microphone as audio source
 * handle with care: mic is very sensitive and might cause feedback using amp!!!
 */
void ac101_setSourceMic(void)
{
    ac.SetMicSource();
}

/*
 * selects the line in as input
 */
void ac101_setSourceLine(void)
{
    ac.SetLineSource();
}

/*
 * very bad implementation checking the button state
 * there is some work required for a better functionality
 */
void button_loop()
{
#if 0 /* wrong place */
    if (digitalRead(PIN_PLAY) == LOW)
    {
        Serial.println("PIN_PLAY pressed");
    }
    if (digitalRead(PIN_VOL_UP) == LOW)
    {
        Serial.println("PIN_VOL_UP pressed");
    }
    if (digitalRead(PIN_VOL_DOWN) == LOW)
    {
        Serial.println("PIN_VOL_DOWN pressed");
    }
#endif
}

#endif
