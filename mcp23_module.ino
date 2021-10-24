/*
 * Copyright (c) 2021 Marcel Licence
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
 * @file mcp23_module.ino
 * @author Marcel Licence
 * @date 08.07.2021
 *
 * @brief This module is used to control the MCP23017 and MCP23S17
 *
 * @see My first PCB for arduino based synthesizer projects - a quick look at the HW for ESP32/DaisySeed - https://youtu.be/Lp65Urhy1-U
 * @see ESP32 / ESP8266 playground arduino project (using switches, encoders, displays, etc..) - https://youtu.be/juwm6o3j8XA
 */


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


#ifdef MCP23_MODULE_ENABLED


#include <Wire.h>
#include <SPI.h>



/*
 * nice ressource for using mcp and wire
 *
 * https://elektro.turanis.de/html/prj128/index.html
 */

#define MCP32_ADDR  0x20

#define IODIRA  0x00
#define IODIRB  0x01
#define IPOLA   0x02
#define IPOLB   0x03
#define GPINTENA 0x04
#define GPINTENB 0x05
#define DEFVALA 0x06
#define DEFVALB 0x07
#define INTCONA 0x08
#define INTCONB 0x09
#define GPPUA   0x0C
#define GPPUB   0x0D
#define INTFA   0x0E
#define INTFB   0x0F
#define INTCAPA 0x10
#define INTCAPB 0x11
#define REG_GPIOA   0x12
#define REG_GPIOB   0x13

#define IODIR_INPUT 0xFF
#define IODIR_OUTPUT    0x00

#define GPPU_PULLUP_ENABLED 0xFF
#define GPPU_PULLUP_DISABLED 0x00


inline
void MCP23_WriteReg(uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(MCP32_ADDR);
    Wire.write(reg); // IODIRA register
    Wire.write(value);
    Wire.endTransmission();
}

uint8_t MCP23_ReadReg(uint8_t reg)
{
    Wire.beginTransmission(MCP32_ADDR);
    Wire.write(reg); // set MCP23017 memory pointer to reg address
    Wire.endTransmission();

    Wire.requestFrom(MCP32_ADDR, 1); // request one byte of data from MCP20317
    return Wire.read(); // store the incoming byte into "inputs"
}

#define    OPCODEW       (0b01000000)  // Opcode for MCP23S17 with LSB (bit0) set to write (0), address OR'd in later, bits 1-3
#define    OPCODER       (0b01000001)  // Opcode for MCP23S17 with LSB (bit0) set to read (1), address OR'd in later, bits 1-3
#define    ADDR_ENABLE   (0b00001000)  // Configuration register for MCP23S17, the only thing we change is enabling hardware addressing

#define ENC_CLK_1   (1<<5)
#define ENC_DIR_1   (1<<6)
#define ENC_BTN_1   (1<<7)


#define ENC_CLK_2   (1<<2)
#define ENC_DIR_2   (1<<1)
#if 1
#define MCP_CS  SPI_CS
void MCP23_SpiWriteReg(uint8_t reg, uint8_t value)
{
    digitalWrite(MCP_CS, LOW);
    SPI.transfer(OPCODEW | (MCP32_ADDR << 1));             // Send the MCP23S17 opcode, chip address, and write bit
    SPI.transfer(reg);                                   // Send the register we want to write
    SPI.transfer(value);                                 // Send the byte
    digitalWrite(MCP_CS, HIGH);
}

uint8_t MSP23_SpiReadReg(uint8_t reg)
{
    digitalWrite(MCP_CS, LOW);
    SPI.transfer(OPCODER | (MCP32_ADDR << 1));             // Send the MCP23S17 opcode, chip address, and write bit
    SPI.transfer(reg);                                   // Send the register we want to write
    uint8_t res = SPI.transfer(0x00);                                 // Send the byte
    digitalWrite(MCP_CS, HIGH);
    return res;
}

void MCP23_Setup(void)
{
    MCP23_WriteReg(IODIRA, IODIR_INPUT);

    MCP23_Select(1);
    MCP23_WriteReg(IODIRB, IODIR_OUTPUT);

    MCP23_Select(1);
    MCP23_SpiWriteReg(IODIRA, IODIR_OUTPUT);
    MCP23_SpiWriteReg(IODIRA, IODIR_OUTPUT);

    MCP23_WriteReg(GPPUA, GPPU_PULLUP_ENABLED);

    for (int i = 0; i < 10; i++)
    {
#if 0
        MCP23_WriteReg(REG_GPIOA, 0x00);
        MCP23_WriteReg(REG_GPIOB, 0x00);
#endif
        MCP23_SpiWriteReg(REG_GPIOA, 0x00);

        delay(50);

#if 0
        MCP23_WriteReg(REG_GPIOA, 0xFF);
        MCP23_WriteReg(REG_GPIOB, 0xFF);
#endif
        MCP23_SpiWriteReg(REG_GPIOA, 0xFF);

        delay(50);
    }

    MCP23_WriteReg(GPINTENA, ENC_CLK_1 + ENC_CLK_2 + ENC_BTN_1); /* enable interrupts for enc1, enc2 and button press */
    MCP23_WriteReg(INTCONA, 0x00); /* Interrupt on 0: pin change */
}

uint8_t valu8 = 0;
uint8_t valu8b = 0;
bool btnDown = false;





//#define HALF_CLOCK_PER_STEP1
//#define MCP23_ENC_DEBUG

#define mcp23_printf(...) Serial.printf(__VA_ARGS__)

void MCP23_Loop()
{
    static uint8_t lastInput = 0xCC;


    uint8_t intfa = MCP23_ReadReg(INTFA);
    uint8_t intcapa = MCP23_ReadReg(INTCAPA);

#ifdef MCP23_CHECK_PORT /* check PORT a only */
    uint8_t inputs = MCP23_ReadReg(REG_GPIOA);

    if (inputs != lastInput)
    {
        lastInput = inputs;
        Status_ValueChangedInt("I2C_IOA", lastInput);
    }
#endif

#if 0
    if (inputs > 0)
    {
        // if a button was pressed
        Serial.println(inputs, BIN); // display the contents of the REG_GPIOB register in binary
        delay(200); // for debounce
    }
#else
#if 0
    if (/*(lastInput != intcapa) && */(intcapa != 00))
#else
    if (intfa != 0)
#endif
    {


        btnDown = ((intcapa & ENC_BTN_1) == 0);


        if (((lastInput & ENC_CLK_1) != (intcapa & ENC_CLK_1)) && ((intfa & ENC_CLK_1) > 0))
        {
            mcp23_printf("in: 0x%02x 0x%02x 0x%02x %s\n", intfa & ENC_CLK_1, intcapa & ENC_DIR_1, intcapa & ENC_CLK_1, btnDown ? "down" : "up");
            if (intcapa & ENC_DIR_1)
            {
                if (!(intcapa & ENC_CLK_1))
                {
#ifdef HALF_CLOCK_PER_STEP1
                    valu8++;
                    mcp23_printf("%d right1\n", valu8);
#endif
                }
                else
                {
                    valu8--;
                    mcp23_printf("%d left2\n", valu8);
                }
            }
            else
            {
                if ((intcapa & ENC_CLK_1))
                {
                    valu8++;
                    mcp23_printf("%d right3\n", valu8);
                }
                else
                {
#ifdef HALF_CLOCK_PER_STEP1
                    valu8--;
                    mcp23_printf("%d left4\n", valu8);
#endif
                }
            }

            Status_ValueChangedInt("valu8", valu8);
        }



        if (((lastInput & ENC_CLK_2) != (intcapa & ENC_CLK_2)) && ((intfa & ENC_CLK_2) > 0))
        {
            mcp23_printf("in: 0x%02x 0x%02x 0x%02x %s\n", intfa & ENC_CLK_2, intcapa & ENC_DIR_2, intcapa & ENC_CLK_2, btnDown ? "down" : "up");
            if (intcapa & ENC_DIR_2)
            {
                if ((intcapa & ENC_CLK_2))
                {
                    valu8b++;
                    mcp23_printf("%d right1b\n", valu8b);
                }
                else
                {
                    valu8b--;
                    mcp23_printf("%d left2b\n", valu8b);
                }
            }
            else
            {
                if (!(intcapa & ENC_CLK_2))
                {
                    valu8b++;
                    mcp23_printf("%d right3b\n", valu8b);
                }
                else
                {
                    valu8b--;
                    mcp23_printf("%d left4b\n", valu8b);
                }
            }
            Status_ValueChangedInt("valu8b", valu8b);
        }

        //Status_ValueChangedInt("lastInput", lastInput);
        lastInput = intcapa;
    }
#endif
}

static uint8_t adcCnt = 0;
static bool ledOn = true;

void MCP23_SetAdcCh(uint8_t adcCh)
{
    adcCnt = adcCh;
    MCP23_UpdateOutput();
}

void MCP23_UpdateOutput()
{
    MCP23_Select(1);
    MCP23_SpiWriteReg(IODIRA, IODIR_OUTPUT); /* shouldn't be necessary but it looks like we do not have the output active otherwise */
    if (ledOn)
    {
        MCP23_SpiWriteReg(REG_GPIOA, 0x00 + adcCnt);
#ifdef SHOW_MCP_READBACK
        mcp23_printf("spi: 0x%02x\n", MSP23_SpiReadReg(REG_GPIOA));
#endif
    }
    else
    {
        MCP23_SpiWriteReg(REG_GPIOA, 0xF0 + adcCnt);
#ifdef SHOW_MCP_READBACK
        mcp23_printf("spi: 0x%02x\n", MSP23_SpiReadReg(REG_GPIOA));
#endif
    }
}

void MCP23_loop1Hz(void)
{
    ledOn = !ledOn;
    MCP23_UpdateOutput();
}
#endif
void MCP23_Select(uint8_t id)
{
    if (id < 8)
    {
        MCP23_WriteReg(REG_GPIOB, (1 << id));
    }
    else
    {
        Serial.printf("id invalid!\n");
    }
}

void MCP23017_Test()
{
    static uint8_t gpioa_in = 0xCC;
    static uint8_t gpiob_in = 0xCC;

    uint8_t input;

    MCP23_WriteReg(GPPUA, GPPU_PULLUP_ENABLED);
    MCP23_WriteReg(GPPUB, GPPU_PULLUP_ENABLED);

    input = MCP23_ReadReg(REG_GPIOA);
    if (input != gpioa_in)
    {
        gpioa_in = input;
        Serial.printf("gpioa_in: 0x%02x\n", gpioa_in);
    }

    input = MCP23_ReadReg(REG_GPIOB);
    if (input != gpiob_in)
    {
        gpiob_in = input;
        Serial.printf("gpiob_in: 0x%02x\n", gpiob_in);
    }
}

void MCP23017_TestCS()
{
    static uint8_t gpioa_in = 0xCC;
    static uint8_t gpiob_in = 0xCC;

    uint8_t input;

    MCP23_WriteReg(GPPUA, GPPU_PULLUP_ENABLED);
    MCP23_WriteReg(GPPUB, GPPU_PULLUP_DISABLED);
    MCP23_WriteReg(IODIRB, IODIR_OUTPUT);



    digitalWrite(SPI_CS, HIGH);
    pinMode(SPI_CS, OUTPUT);

#if 0
    for (int id = 0; id < 8; id++)
    {
        MCP23_WriteReg(REG_GPIOB, (1 << id));
        delay(100);
        digitalWrite(SPI_CS, LOW);
        delay(200);
        digitalWrite(SPI_CS, HIGH);
        delay(100);
    }
#endif

    //MCP23_WriteReg(REG_GPIOB, 0xFF);
    MCP23_Select(SPI_SEL_MCP23S17);

    MCP23_SpiWriteReg(IODIRA, IODIR_OUTPUT);
    MCP23_SpiWriteReg(IODIRB, IODIR_OUTPUT);
    static uint8_t outVar = 0xAA;

    MCP23_SpiWriteReg(REG_GPIOA, outVar);
    outVar = ~outVar;
    MCP23_SpiWriteReg(REG_GPIOB, outVar);

    input = MSP23_SpiReadReg(REG_GPIOA);
    if (input != gpioa_in)
    {
        gpioa_in = input;
        Serial.printf("gpioa_in: 0x%02x\n", gpioa_in);
    }

    input = MSP23_SpiReadReg(REG_GPIOB);
    if (input != gpiob_in)
    {
        gpiob_in = input;
        Serial.printf("gpiob_in: 0x%02x\n", gpiob_in);
    }


    /*
     * setup int for encoder
     */
    MCP23_WriteReg(GPINTENA, ENC_CLK_1 + ENC_CLK_2 + ENC_BTN_1); /* enable interrupts for enc1, enc2 and button press */
    MCP23_WriteReg(INTCONA, 0x00); /* Interrupt on 0: pin change */
}

void MCP23_SelAdc(uint8_t i)
{
    uint32_t var = 0;
    var &= 0xF0;
    var |= i;
    MCP23_SpiWriteReg(REG_GPIOB, var);
}

#endif /* MCP23_MODULE_ENABLED */

