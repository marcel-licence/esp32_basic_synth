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
 * @file es8388.ino
 * @author Marcel Licence
 * @date 22.08.2021
 *
 * @brief This module is used to initialize the ES8388
 *
 * @see ESP32 Audio Kit AC101 codec failure - Get synthesizer projects working based on ES8388 - https://youtu.be/8UB3fYPjqSk
 * @see https://github.com/espressif/esp-adf/blob/master/components/audio_hal/driver/es8388/es8388.c
 */


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


#ifdef ES8388_ENABLED
/*
 * http://www.everest-semi.com/pdf/ES8388%20DS.pdf
 */

#include <Wire.h>


/* ES8388 address */
//#define ES8388_ADDR 0x20  /*!< 0x22:CE=1;0x20:CE=0*/
#define ES8388_ADDR 0x10  /*!< 0x22:CE=1;0x20:CE=0*/


/* ES8388 register */
#define ES8388_CONTROL1         0x00
#define ES8388_CONTROL2         0x01

#define ES8388_CHIPPOWER        0x02

#define ES8388_ADCPOWER         0x03
#define ES8388_DACPOWER         0x04

#define ES8388_CHIPLOPOW1       0x05
#define ES8388_CHIPLOPOW2       0x06

#define ES8388_ANAVOLMANAG      0x07

#define ES8388_MASTERMODE       0x08
/* ADC */
#define ES8388_ADCCONTROL1      0x09
#define ES8388_ADCCONTROL2      0x0a
#define ES8388_ADCCONTROL3      0x0b
#define ES8388_ADCCONTROL4      0x0c
#define ES8388_ADCCONTROL5      0x0d
#define ES8388_ADCCONTROL6      0x0e
#define ES8388_ADCCONTROL7      0x0f
#define ES8388_ADCCONTROL8      0x10
#define ES8388_ADCCONTROL9      0x11
#define ES8388_ADCCONTROL10     0x12
#define ES8388_ADCCONTROL11     0x13
#define ES8388_ADCCONTROL12     0x14
#define ES8388_ADCCONTROL13     0x15
#define ES8388_ADCCONTROL14     0x16
/* DAC */
#define ES8388_DACCONTROL1      0x17
#define ES8388_DACCONTROL2      0x18
#define ES8388_DACCONTROL3      0x19
#define ES8388_DACCONTROL4      0x1a
#define ES8388_DACCONTROL5      0x1b
#define ES8388_DACCONTROL6      0x1c
#define ES8388_DACCONTROL7      0x1d
#define ES8388_DACCONTROL8      0x1e
#define ES8388_DACCONTROL9      0x1f
#define ES8388_DACCONTROL10     0x20
#define ES8388_DACCONTROL11     0x21
#define ES8388_DACCONTROL12     0x22
#define ES8388_DACCONTROL13     0x23
#define ES8388_DACCONTROL14     0x24
#define ES8388_DACCONTROL15     0x25
#define ES8388_DACCONTROL16     0x26
#define ES8388_DACCONTROL17     0x27
#define ES8388_DACCONTROL18     0x28
#define ES8388_DACCONTROL19     0x29
#define ES8388_DACCONTROL20     0x2a
#define ES8388_DACCONTROL21     0x2b
#define ES8388_DACCONTROL22     0x2c
#define ES8388_DACCONTROL23     0x2d
#define ES8388_DACCONTROL24     0x2e
#define ES8388_DACCONTROL25     0x2f
#define ES8388_DACCONTROL26     0x30
#define ES8388_DACCONTROL27     0x31
#define ES8388_DACCONTROL28     0x32
#define ES8388_DACCONTROL29     0x33
#define ES8388_DACCONTROL30     0x34


uint8_t ES8388_ReadReg(uint8_t reg)
{
    Wire.beginTransmission(ES8388_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);

    uint8_t val = 0u;
    if (1 == Wire.requestFrom(uint16_t(ES8388_ADDR), uint8_t(1), true))
    {
        val = Wire.read();
    }
    Wire.endTransmission(false);

    return val;
}

bool ES8388_WriteReg(uint8_t reg, uint8_t val)
{
    Wire.beginTransmission(ES8388_ADDR);
    Wire.write(reg);
    Wire.write(val);
    return 0 == Wire.endTransmission(true);
}

bool ES8388_begin(int sda, int scl, uint32_t frequency)
{
    bool ok = Wire.begin(sda, scl, frequency);

    // Reset all registers, readback default as sanity check
    //ok &= WriteReg(CHIP_AUDIO_RS, 0x123);
    delay(100);

    Serial.printf("0x00: 0x%02x\n", ES8388_ReadReg(ES8388_CONTROL1));
    Serial.printf("0x01: 0x%02x\n", ES8388_ReadReg(ES8388_CONTROL2));

    ES8388_WriteReg(ES8388_CONTROL1, 1 << 7); /* do reset! */
    ES8388_WriteReg(ES8388_CONTROL1, 0x06);
    ES8388_WriteReg(ES8388_CONTROL2, 0x50);

    ok &= (0x06 == ES8388_ReadReg(ES8388_CONTROL1));
    ok &= (0x50 == ES8388_ReadReg(ES8388_CONTROL2));
    return ok;
}

void es8388_read_all()
{
    for (int i = 0; i < 53; i++)
    {
        uint8_t reg = 0;
        reg = ES8388_ReadReg(i);
        Serial.printf("Reg 0x%02x = 0x%02x\n", i, reg);
    }
}

void ES8388_SetADCVOL(uint8_t unused, float vol)
{
#ifdef STATUS_ENABLED
    Status_ValueChangedInt("ADC Volume /db", (vol - 1) * 97 + 0.5);
#endif

    vol *= -192;
    vol += 192;

    uint8_t volu8 = vol;

    if (volu8 > 192)
    {
        volu8 = 192;
    }

    ES8388_WriteReg(0x10, volu8); // LADCVOL
    ES8388_WriteReg(0x11, volu8); // RADCVOL
}

void ES8388_SetDACVOL(uint8_t unused, float vol)
{
#ifdef STATUS_ENABLED
    Status_ValueChangedInt("DAC Volume /db", (vol - 1) * 97 + 0.5);
#endif

    vol *= -192;
    vol += 192;

    uint8_t volu8 = vol;

    if (volu8 > 192)
    {
        volu8 = 192;
    }

    ES8388_WriteReg(0x1A, volu8); // LDACVOL
    ES8388_WriteReg(0x1B, volu8); // RDACVOL
}

void ES8388_SetPGAGain(uint8_t unused, float vol)
{
#ifdef STATUS_ENABLED
    Status_ValueChangedInt("PGA Gain /db", vol * 24 + 0.25);
#endif

    vol *= 8;

    uint8_t volu8 = vol;

    if (volu8 > 8)
    {
        volu8 = 8;
    }
    // ES8388_ADCCONTROL1
    ES8388_WriteReg(0x09, volu8 + (volu8 << 4)); // MicAmpL, MicAmpR
}

void ES8388_SetInputCh(uint8_t ch, float var)
{
    if (var > 0)
    {
        uint8_t in;
        switch (ch)
        {
        case 0:
            in = 0;
            Serial.printf("AdcCh0!\n");
            break;

        case 1:
            in = 1;
            Serial.printf("AdcCh1!\n");
            break;

        default:
            Serial.printf("Illegal Input ch!\n");
            return;
        }
        // ES8388_ADCCONTROL2
        ES8388_WriteReg(0x0A, (in << 6) + (in << 4)); // LINSEL , RINSEL , DSSEL , DSR

#ifdef STATUS_ENABLED
        Status_ValueChangedInt("ADC Ch", in);
#endif
    }
}

void ES8388_SetMixInCh(uint8_t ch, float var)
{
    if (var > 0)
    {
        uint8_t in = 0;
        switch (ch)
        {
        case 0:
            in = 0;
            Serial.printf("MixCh0!\n");
            break;

        case 1:
            in = 1;
            Serial.printf("MixCh1!\n");
            break;

        case 2:
            in = 3;
            Serial.printf("MixChAMPL!\n");
            break;

        default:
            Serial.printf("Illegal Mix Input ch!\n");
            return;
        }
        // ES8388_DACCONTROL16
        ES8388_WriteReg(0x26, in + (in << 3)); // LMIXSEL, RMIXSEL
#ifdef STATUS_ENABLED
        Status_ValueChangedInt("Mix In Ch", in);
#endif
    }
}

void ES8388_SetIn2OoutVOL(uint8_t unused, float vol)
{
#ifdef STATUS_ENABLED
    Status_ValueChangedInt("In to out volume /db", (vol - 1) * 16 + 0.5);
#endif

    vol *= -5;
    vol += 7;

    uint8_t volu8 = vol;

    if (volu8 > 7)
    {
        volu8 = 7;
    }

    uint8_t var;

    var = ES8388_ReadReg(0x27) & 0xC0;
    if (volu8 == 7)
    {
        var &= ~ 0x40;
    }
    else
    {
        var |= 0x40;
    }
    ES8388_WriteReg(0x27, (volu8 << 3) + var); // LD2LO, LI2LO, LI2LOVOL

    var = ES8388_ReadReg(0x2A) & 0xC0;
    if (volu8 == 7)
    {
        var &= ~ 0x40;
    }
    else
    {
        var |= 0x40;
    }
    ES8388_WriteReg(0x2A, (volu8 << 3) + var); // RD2RO, RI2RO, RI2ROVOL
}

void ES8388_SetOUT1VOL(uint8_t unused, float vol)
{
#ifdef STATUS_ENABLED
    Status_ValueChangedInt("OUT1VOL /db", (vol - 1) * 31 + 0.5);
#endif

    vol *= 0x1E;

    uint8_t volu8 = vol;

    if (volu8 > 129)
    {
        volu8 = 129;
    }

    ES8388_WriteReg(0x2E, volu8); // LOUT1VOL
    ES8388_WriteReg(0x2F, volu8); // ROUT1VOL
}

void ES8388_SetOUT2VOL(uint8_t unused, float vol)
{
#ifdef STATUS_ENABLED
    Status_ValueChangedInt("OUT2VOL /db", (vol - 1) * 31 + 0.5);
#endif

    vol *= 0x1E;

    uint8_t volu8 = vol;

    if (volu8 > 129)
    {
        volu8 = 129;
    }

    ES8388_WriteReg(0x30, volu8); // LOUT2VOL
    ES8388_WriteReg(0x31, volu8); // ROUT2VOL
}

void ES8388_Setup()
{
    Serial.printf("Connect to ES8388 codec... ");

    while (not ES8388_begin(ES8388_PIN_SDA, ES8388_PIN_SCL, 400000))
    {
        Serial.printf("Failed!\n");
        delay(1000);
    }

    ES8388_WriteReg(ES8388_CHIPPOWER, 0xFF);  //reset and stop es8388


    ES8388_WriteReg(0x00, 0x80); /* reset control port register to default */
    ES8388_WriteReg(0x00, 0x06); /* restore default value */



    /*
     * https://dl.radxa.com/rock2/docs/hw/ds/ES8388%20user%20Guide.pdf
     */

    /*
     * 10.5 Power Down Sequence (To Standby Mode)
     */
    ES8388_WriteReg(0x0F, 0x34); /* ADC Mute */
    ES8388_WriteReg(0x19, 0x36); /* DAC Mute */

    ES8388_WriteReg(0x02, 0xF3); /* Power down DEM and STM */

    /*
     * 10.4 The sequence for Start up bypass mode
     */
    /* Set Chip to Slave Mode */
    ES8388_WriteReg(0x08, 0x00);
    /* Power down DEM and STM */
    ES8388_WriteReg(0x02, 0x3F);
    /* Set same LRCK */
    ES8388_WriteReg(0x2B, 0x80);
    /* Set Chip to Play&Record Mode */
    ES8388_WriteReg(0x00, 0x05);
    /* Power Up Analog and Ibias */
    ES8388_WriteReg(0x01, 0x40);
#if 0
    /*
     * Power down ADC, Power up
     * Analog Input for Bypass
     */
    ES8388_WriteReg(0x03, 0x3F);
#else
    ES8388_WriteReg(0x03, 0x3F); /* adc also on but no bias */

    ES8388_WriteReg(0x03, 0x00); // PdnAINL, PdinAINR, PdnADCL, PdnADCR, PdnMICB, PdnADCBiasgen, flashLP, Int1LP
#endif

#if 0
    /*
     * Power Down DAC, Power up
     * Analog Output for Bypass
     */
    ES8388_WriteReg(0x04, 0xFC);
#else
    /*
     * Power up DAC / Analog Output
     * for Record
     */
    ES8388_WriteReg(0x04, 0x3C);


#if 1
    /*
     * Select Analog input channel for ADC
     */
    ES8388_WriteReg(0x0A, 0x80); // LINSEL , RINSEL , DSSEL , DSR

    /* Select PGA Gain for ADC analog input */
    ES8388_WriteReg(0x09, 0x00); // PGA gain?

    //ES8388_WriteReg(0x0C, 0x18); // DATSEL, ADCLRP, ADCWL, ADCFORMAT
    //ES8388_WriteReg(0x0C, 0x40); // DATSEL, ADCLRP, ADCWL, ADCFORMAT
    ES8388_WriteReg(0x0C, 0x0C); // DATSEL, ADCLRP, ADCWL, ADCFORMAT
    ES8388_WriteReg(0x0D, 0x02); // ADCFsMode , ADCFsRatio
    //ES8388_WriteReg(0x0D, (1<<5) + 0x03); // ADCFsMode , ADCFsRatio Hasan
    //ES8388_WriteReg(0x0D, 0); // ADCFsMode , ADCFsRatio Hasan

    /*
     * Set ADC Digital Volume
     */
#if 1
    ES8388_SetADCVOL(0, 1.0f);
#else
    ES8388_WriteReg(0x10, 0x00); // LADCVOL
    ES8388_WriteReg(0x11, 0x00); // RADCVOL
#endif

    /* UnMute ADC */
    ES8388_WriteReg(0x0F, 0x30); //

    ES8388_WriteReg(0x12, 0x16);

    ES8388_WriteReg(0x17, 0x18); // DACLRSWAP, DACLRP, DACWL, DACFORMAT
    ES8388_WriteReg(0x18, 0x02); // DACFsMode , DACFsRatio
#endif


    /*
     * Set ADC Digital Volume
     */
    ES8388_WriteReg(0x1A, 0x00);
    ES8388_WriteReg(0x1B, 0x02);
    //ES8388_WriteReg(0x1B, (1<<5) + 0x03); // ADCFsMode , ADCFsRatio Hasan
    //ES8388_WriteReg(0x1B, 0); // ADCFsMode , ADCFsRatio Hasan
    /* UnMute DAC */
    ES8388_WriteReg(0x19, 0x32);
#endif
    /*
     * Setup Mixer
     */
    ES8388_WriteReg(0x26, 0x09);//        ES8388_WriteReg(0x26, 0x00);
    ES8388_WriteReg(0x27, 0xD0); // ES8388_DACCONTROL17
    ES8388_WriteReg(0x28, 0x38);
    ES8388_WriteReg(0x29, 0x38);
    ES8388_WriteReg(0x2A, 0xD0);

    /* Set Lout/Rout Volume */
#if 1
    ES8388_SetOUT1VOL(0, 1);
    ES8388_SetOUT2VOL(0, 1);
#else
    ES8388_WriteReg(0x2E, 0x1E);
    ES8388_WriteReg(0x2F, 0x1E);
    ES8388_WriteReg(0x30, 0x1E);
    ES8388_WriteReg(0x31, 0x1E);
#endif

    /* Power up DEM and STM */
#if 0
    ES8388_WriteReg(0x02, 0xF0);
#else
    ES8388_WriteReg(0x02, 0x00);
#endif

    ES8388_SetInputCh(1, 1);
    ES8388_SetMixInCh(2, 1);
    ES8388_SetPGAGain(0, 1);
    ES8388_SetIn2OoutVOL(0, 0);

    Serial.printf("ES8388 setup finished!\n");
    es8388_read_all();
}

#endif

