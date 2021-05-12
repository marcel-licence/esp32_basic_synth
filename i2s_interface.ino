/*
 * this file includes all required function to setup and drive the i2s interface
 *
 * Author: Marcel Licence
 */

#include <driver/i2s.h>

/*
 * no dac not tested within this code
 * - it has the purpose to generate a quasy analog signal without a DAC
 */
//#define I2S_NODAC



const i2s_port_t i2s_num = I2S_NUM_0; // i2s port number

#ifdef ESP32_AUDIO_KIT
#define SAMPLE_SIZE_16BIT
#endif


#ifdef I2S_NODAC

bool writeDAC(float DAC_f);
bool i2s_write_sample(uint32_t sample);

bool i2s_write_sample(uint32_t sample)
{
    static size_t bytes_written = 0;
    i2s_write((i2s_port_t)i2s_num, (const char *)&sample, 4, &bytes_written, portMAX_DELAY);

    if (bytes_written > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

static uint32_t i2sACC;
static uint16_t err;

bool writeDAC(float DAC_f)
{
    uint16_t DAC = 0x8000 + int16_t(DAC_f * 32767.0f);
    for (uint8_t i = 0; i < 32; i++)
    {
        i2sACC = i2sACC << 1;
        if (DAC >= err)
        {
            i2sACC |= 1;
            err += 0xFFFF - DAC;
        }
        else
        {
            err -= DAC;
        }
    }
    bool ret = i2s_write_sample(i2sACC);

    return ret;
}

#else

bool i2s_write_sample_32ch2(uint64_t sample)
{
    static size_t bytes_written = 0;
    i2s_write((i2s_port_t)i2s_num, (const char *)&sample, 8, &bytes_written, portMAX_DELAY);

    if (bytes_written > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}


bool i2s_write_stereo_samples(float *fl_sample, float *fr_sample)
{

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
    sampleDataU.ch[0] = int16_t(*fr_sample * 16383.0f);
    sampleDataU.ch[1] = int16_t(*fl_sample * 16383.0f);

    static size_t bytes_written = 0;
    static size_t bytes_read = 0;

    i2s_write(i2s_num, (const char *)&sampleDataU.sample, 4, &bytes_written, portMAX_DELAY);

    if (bytes_written > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}


#endif


/*
 * i2s configuration
 */

i2s_config_t i2s_config =
{
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX ),
    .sample_rate = SAMPLE_RATE,
#ifdef I2S_NODAC
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_I2S_MSB,
#else
#ifdef ESP32_AUDIO_KIT
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
#else
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
#endif
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
#endif
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = 0
};


#ifdef I2S_NODAC
i2s_pin_config_t pins =
{
    .bck_io_num = I2S_PIN_NO_CHANGE,
    .ws_io_num =  I2S_PIN_NO_CHANGE,
    .data_out_num = 22,
    .data_in_num = I2S_PIN_NO_CHANGE
};
#else
i2s_pin_config_t pins =
{
#ifdef ESP32_AUDIO_KIT
    .bck_io_num = IIS_SCLK,
    .ws_io_num =  IIS_LCLK,
    .data_out_num = IIS_DSIN,
    .data_in_num = IIS_DSOUT
#else
    .bck_io_num = I2S_BCLK_PIN,
    .ws_io_num =  I2S_WCLK_PIN,
    .data_out_num = I2S_DOUT_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE
#endif
};
#endif


void setup_i2s()
{
    i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pins);
    i2s_set_sample_rates(i2s_num, SAMPLE_RATE);
    i2s_start(i2s_num);
}
