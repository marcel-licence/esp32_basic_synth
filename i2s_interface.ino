/*
 * this file includes all required function to setup and drive the i2s interface
 *
    Copyright (C) 2021  Marcel Licence

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifdef __CDT_PARSER__
#include <cdt.h>
#endif

#include <driver/i2s.h>


/*
 * no dac not tested within this code
 * - it has the purpose to generate a quasy analog signal without a DAC
 */
//#define I2S_NODAC


const i2s_port_t i2s_port_number = I2S_NUM_0;

/*
 * please refer to https://www.hackster.io/janost/audio-hacking-on-the-esp8266-fa9464#toc-a-simple-909-drum-synth-0
 * for the following implementation
 */
#ifdef I2S_NODAC
/* todo integrate code, or external module from hack a day using i2s signal wire as DAC */
#else

bool i2s_write_sample_32ch2(uint8_t *sample);

bool i2s_write_sample_32ch2(uint8_t *sample)
{
    static size_t bytes_written = 0;
    static size_t bytes_read = 0;
    i2s_read(i2s_port_number, (char *)sample, 8, &bytes_read, portMAX_DELAY);

    i2s_write(i2s_port_number, (const char *)sample, 8, &bytes_written, portMAX_DELAY);

    if (bytes_written > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

#ifdef SAMPLE_SIZE_24BIT

bool i2s_write_sample_24ch2(uint8_t *sample);

bool i2s_write_sample_24ch2(uint8_t *sample)
{
    static size_t bytes_written1 = 0;
    static size_t bytes_written2 = 0;
    i2s_write(i2s_port_number, (const char *)&sample[1], 3, &bytes_written1, portMAX_DELAY);
    i2s_write(i2s_port_number, (const char *)&sample[5], 3, &bytes_written2, portMAX_DELAY);

    if ((bytes_written1 + bytes_written2) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

#endif

bool i2s_write_stereo_samples(float *fl_sample, float *fr_sample)
{
#ifdef SAMPLE_SIZE_32BIT
    static union sampleTUNT
    {
        uint64_t sample;
        int32_t ch[2];
    } sampleDataU;
#endif
#ifdef SAMPLE_SIZE_24BIT
#if 0
    static union sampleTUNT
    {
        uint8_t sample[8];
        int32_t ch[2];
    } sampleDataU;
#else
    static union sampleTUNT
    {
        int32_t ch[2];
        uint8_t bytes[8];
    } sampleDataU;
#endif
#endif
#ifdef SAMPLE_SIZE_16BIT
    static union sampleTUNT
    {
        uint32_t sample;
        int16_t ch[2];
    } sampleDataU;
#endif

    /*
     * using RIGHT_LEFT format
     */
#ifdef SAMPLE_SIZE_16BIT
    sampleDataU.ch[0] = int16_t(*fr_sample * 16383.0f); /* some bits missing here */
    sampleDataU.ch[1] = int16_t(*fl_sample * 16383.0f);
#endif
#ifdef SAMPLE_SIZE_32BIT
    sampleDataU.ch[0] = int32_t(*fr_sample * 1073741823.0f); /* some bits missing here */
    sampleDataU.ch[1] = int32_t(*fl_sample * 1073741823.0f);
#endif

    static size_t bytes_written = 0;

#ifdef SAMPLE_SIZE_16BIT
    i2s_write(i2s_port_number, (const char *)&sampleDataU.sample, 4, &bytes_written, portMAX_DELAY);
#endif
#ifdef SAMPLE_SIZE_32BIT
    i2s_write(i2s_port_number, (const char *)&sampleDataU.sample, 8, &bytes_written, portMAX_DELAY);
#endif

    if (bytes_written > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

#ifdef SAMPLE_BUFFER_SIZE
bool i2s_write_stereo_samples_buff(float *fl_sample, float *fr_sample, const int buffLen)
{
#ifdef SAMPLE_SIZE_32BIT
    static union sampleTUNT
    {
        uint64_t sample;
        int32_t ch[2];
    } sampleDataU[SAMPLE_BUFFER_SIZE];
#endif
#ifdef SAMPLE_SIZE_24BIT
#if 0
    static union sampleTUNT
    {
        uint8_t sample[8];
        int32_t ch[2];
    } sampleDataU[SAMPLE_BUFFER_SIZE];
#else
    static union sampleTUNT
    {
        int32_t ch[2];
        uint8_t bytes[8];
    } sampleDataU[SAMPLE_BUFFER_SIZE];
#endif
#endif
#ifdef SAMPLE_SIZE_16BIT
    static union sampleTUNT
    {
        uint32_t sample;
        int16_t ch[2];
    } sampleDataU[SAMPLE_BUFFER_SIZE];
#endif

    for (int n = 0; n < buffLen; n++)
    {
        /*
         * using RIGHT_LEFT format
         */
#ifdef SAMPLE_SIZE_16BIT
        sampleDataU[n].ch[0] = int16_t(fr_sample[n] * 16383.0f); /* some bits missing here */
        sampleDataU[n].ch[1] = int16_t(fl_sample[n] * 16383.0f);
#endif
#ifdef SAMPLE_SIZE_32BIT
        sampleDataU[n].ch[0] = int32_t(fr_sample[n] * 1073741823.0f); /* some bits missing here */
        sampleDataU[n].ch[1] = int32_t(fl_sample[n] * 1073741823.0f);
#endif
    }

    static size_t bytes_written = 0;

    i2s_write(i2s_port_number, (const char *)&sampleDataU[0].sample, 4 * buffLen, &bytes_written, portMAX_DELAY);

    if (bytes_written > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
#endif /* #ifdef SAMPLE_BUFFER_SIZE */

void i2s_read_stereo_samples(float *fl_sample, float *fr_sample)
{
    static size_t bytes_read = 0;

    static union
    {
        uint32_t sample;
        int16_t ch[2];
    } sampleData;


    i2s_read(i2s_port_number, (char *)&sampleData.sample, 4, &bytes_read, portMAX_DELAY);

    //sampleData.ch[0] &= 0xFFFE;
    //sampleData.ch[1] &= 0;

    /*
     * using RIGHT_LEFT format
     */
    *fr_sample = ((float)sampleData.ch[0] * (5.5f / 65535.0f));
    *fl_sample = ((float)sampleData.ch[1] * (5.5f / 65535.0f));
}

#ifdef SAMPLE_BUFFER_SIZE
void i2s_read_stereo_samples_buff(float *fl_sample, float *fr_sample, const int buffLen)
{
#ifdef I2S_DIN_PIN
    static size_t bytes_read = 0;

#ifdef SAMPLE_SIZE_16BIT
    static union
    {
        uint32_t sample;
        int16_t ch[2];
    } sampleData[SAMPLE_BUFFER_SIZE];
#endif

    i2s_read(i2s_port_number, (char *)&sampleData[0].sample, 4 * buffLen, &bytes_read, portMAX_DELAY);

    //sampleData.ch[0] &= 0xFFFE;
    //sampleData.ch[1] &= 0;

    for (int n = 0; n < buffLen; n++)
    {
        /*
         * using RIGHT_LEFT format
         */
        //fr_sample[n] = ((float)sampleData[n].ch[0] * (5.5f / 65535.0f));
        //fl_sample[n] = ((float)sampleData[n].ch[1] * (5.5f / 65535.0f));
        fr_sample[n] = ((float)sampleData[n].ch[0] / (16383.0f));
        fl_sample[n] = ((float)sampleData[n].ch[1] / (16383.0f));
    }
#endif
}
#endif /* #ifdef SAMPLE_BUFFER_SIZE */

#endif

/*
 * i2s configuration
 */

#ifdef ESP32_AUDIO_KIT

#ifndef ES8388_ENABLED
#define I2S_BCLK_PIN IIS_SCLK
#define I2S_WCLK_PIN IIS_LCLK
#define I2S_DOUT_PIN IIS_DSIN
#define I2S_DIN_PIN IIS_DSOUT
#else
#define I2S_MCLK_PIN ES8388_PIN_MCLK
#define I2S_BCLK_PIN ES8388_PIN_SCLK
#define I2S_WCLK_PIN ES8388_PIN_LRCK
#define I2S_DOUT_PIN ES8388_PIN_DIN
#define I2S_DIN_PIN ES8388_PIN_DOUT
#endif

#endif

i2s_config_t i2s_configuration =
{
#ifdef I2S_DIN_PIN
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX), // | I2S_MODE_DAC_BUILT_IN
#else
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
#endif
    .sample_rate = SAMPLE_RATE,
#ifdef I2S_NODAC
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_I2S_MSB,
#else
#ifdef SAMPLE_SIZE_32BIT
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, /* the DAC module will only take the 8bits from MSB */
#endif
#ifdef SAMPLE_SIZE_24BIT
    .bits_per_sample = I2S_BITS_PER_SAMPLE_24BIT, /* the DAC module will only take the 8bits from MSB */
#endif
#ifdef SAMPLE_SIZE_16BIT
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, /* the DAC module will only take the 8bits from MSB */
#endif
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
#endif
    .intr_alloc_flags = 0, // default interrupt priority
    .dma_buf_count = 8,
    .dma_buf_len = 64,
#ifdef ES8388_ENABLED
    .use_apll = true,
#else
    .use_apll = false,
#endif
};


#ifdef I2S_NODAC
i2s_pin_config_t pins =
{
    .bck_io_num = I2S_PIN_NO_CHANGE,
    .ws_io_num =  I2S_PIN_NO_CHANGE,
    .data_out_num = I2S_NODAC_OUT_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE
};
#else
i2s_pin_config_t pins =
{
    .bck_io_num = I2S_BCLK_PIN,
    .ws_io_num =  I2S_WCLK_PIN,
    .data_out_num = I2S_DOUT_PIN,
#ifdef I2S_DIN_PIN
    .data_in_num = I2S_DIN_PIN
#else
    .data_in_num = I2S_PIN_NO_CHANGE
#endif
};
#endif

void setup_i2s()
{
    i2s_driver_install(i2s_port_number, &i2s_configuration, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pins);
    i2s_set_sample_rates(i2s_port_number, SAMPLE_RATE);
    i2s_start(i2s_port_number);
#ifdef ES8388_ENABLED
    REG_WRITE(PIN_CTRL, 0xFFFFFFF0);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
#endif
}
