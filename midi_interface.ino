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
 * @file midi_interface.ino
 * @author Marcel Licence
 * @date 04.10.2021
 *
 * @brief This file contains an implementation of a simple MIDI interpreter to parse incoming messages
 *
 * MIDI_DUMP_Serial1_TO_SERIAL <- when active received data will be output as hex on serial(1)
 * MIDI_SERIAL1_BAUDRATE <- use define to override baud-rate for MIDI, otherwise default of 31250 will be used
 *
 * @see https://www.midi.org/specifications-old/item/table-1-summary-of-midi-message
 */


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


/*
 * look for midi interface using 1N136
 * to convert the MIDI din signal to
 * a uart compatible signal
 */

#ifndef MIDI_SERIAL_BAUDRATE
#define MIDI_SERIAL_BAUDRATE    SERIAL_BAUDRATE
#endif

#ifndef MIDI_SERIAL1_BAUDRATE
#define MIDI_SERIAL1_BAUDRATE   31250
#endif

#ifndef MIDI_SERIAL2_BAUDRATE
#define MIDI_SERIAL2_BAUDRATE   31250
#endif

/* use define to dump midi data */
//#define MIDI_DUMP_SERIAL2_TO_SERIAL


#if (defined MIDI_RX_PIN) || (defined MIDI_RECV_FROM_SERIAL)
#define MIDI_PORT_ACTIVE
#endif

#ifdef MIDI_RX1_PIN
#define MIDI_PORT1_ACTIVE
#endif

#ifdef MIDI_RX2_PIN
#define MIDI_PORT2_ACTIVE
#endif

struct midi_port_s
{
    Stream *serial; /* this can be software or hardware serial */
    uint32_t inMsgWd ;
    uint8_t inMsg[3];
    uint8_t inMsgIndex ;
};

#ifdef ARDUINO_DAISY_SEED
HardwareSerial Serial2(USART1);
#endif

#if (defined ARDUINO_GENERIC_F407VGTX) // || (defined ARDUINO_DISCO_F407VG)
HardwareSerial Serial2(USART2); /* PA3 */
#endif

#if 0
#ifdef ARDUINO_GENERIC_F407VGTX
HardwareSerial Serial2(USART3); /* PB11 */
#endif
#endif

#ifdef MIDI_PORT_ACTIVE
struct midi_port_s MidiPort;
#endif

#ifdef MIDI_PORT1_ACTIVE
struct midi_port_s MidiPort1;
#endif

#ifdef MIDI_PORT2_ACTIVE
struct midi_port_s MidiPort2;
#endif

/*
 * structure is used to build the mapping table
 */
struct midiControllerMapping
{
    uint8_t channel;
    uint8_t data1;
    const char *desc;
    void(*callback_mid)(uint8_t ch, uint8_t data1, uint8_t data2);
#ifdef MIDI_FMT_INT
    void(*callback_val)(uint8_t userdata, uint8_t value);
#else
    void(*callback_val)(uint8_t userdata, float value);
#endif
    uint8_t user_data;
};

#ifdef MIDI_MAP_FLEX_ENABLED
struct midiMapLookUpEntry
{
    const char *desc;
    struct midiControllerMapping *controlMap;
    int controlMapSize;
};
#endif

struct midiMapping_s
{
    void (*rawMsg)(uint8_t *msg);
#ifdef MIDI_FMT_INT
    void (*noteOn)(uint8_t ch, uint8_t note, uint8_t vel);
    void (*noteOff)(uint8_t ch, uint8_t note);
    void (*pitchBend)(uint8_t ch, uint8_t bend);
    void (*modWheel)(uint8_t ch, uint8_t value);
#else
    void (*noteOn)(uint8_t ch, uint8_t note, float vel);
    void (*noteOff)(uint8_t ch, uint8_t note);
    void (*pitchBend)(uint8_t ch, float bend);
    void (*modWheel)(uint8_t ch, float value);
#endif
    void (*rttMsg)(uint8_t msg);
    void (*songPos)(uint16_t pos);

    struct midiControllerMapping *controlMapping;
    int mapSize;

#ifdef MIDI_MAP_FLEX_ENABLED
    /* the following map can be changed  during runtime */
    struct midiControllerMapping *controlMapping_flex;
    int mapSize_flex;
#endif
};

/*
 * following variables shall be defined in z_config.ino
 */
extern struct midiMapping_s midiMapping;
#ifdef MIDI_MAP_FLEX_ENABLED
extern struct midiMapLookUpEntry midiMapLookUp[];
extern int midiMapLookUpCnt;
#endif

/* constant to normalize midi value to 0.0 - 1.0f */
#define NORM127MUL  0.007874f

inline void Midi_NoteOn(uint8_t ch, uint8_t note, uint8_t vel)
{
#ifdef MIDI_BLE_ENABLED
    Ble_NoteOn(ch, note, vel);
#endif
    if (vel > 127)
    {
        /* we will end up here in case of problems with the MIDI connection */
        vel = 127;
        Serial.printf("to loud note detected!!!!!!!!!!!!!!!!!!!!!!!\n");
    }

    if (midiMapping.noteOn != NULL)
    {
#ifdef MIDI_FMT_INT
        midiMapping.noteOn(ch, note, vel);
#else
        midiMapping.noteOn(ch, note, pow(2, ((vel * NORM127MUL) - 1.0f) * 6));
#endif
    }
}

inline void Midi_NoteOff(uint8_t ch, uint8_t note)
{
#ifdef MIDI_BLE_ENABLED
    Ble_NoteOff(ch, note);
#endif
    if (midiMapping.noteOff != NULL)
    {
        midiMapping.noteOff(ch, note);
    }
}

inline void Midi_CC_Map(uint8_t channel, uint8_t data1, uint8_t data2, struct midiControllerMapping *controlMapping, int mapSize)
{
    for (int i = 0; i < mapSize; i++)
    {
        if ((controlMapping[i].channel == channel) && (controlMapping[i].data1 == data1))
        {
            if (controlMapping[i].callback_mid != NULL)
            {
                controlMapping[i].callback_mid(channel, data1, data2);
            }
            if (controlMapping[i].callback_val != NULL)
            {
#ifdef MIDI_FMT_INT
                controlMapping[i].callback_val(controlMapping[i].user_data, data2);
#else
                controlMapping[i].callback_val(controlMapping[i].user_data, (float)data2 * NORM127MUL);
#endif
            }
        }
    }
}

/*
 * this function will be called when a control change message has been received
 */
inline void Midi_ControlChange(uint8_t channel, uint8_t data1, uint8_t data2)
{
#ifdef MIDI_BLE_ENABLED
    Ble_ControlChange(channel, data1, data2);
#endif

    Midi_CC_Map(channel, data1, data2, midiMapping.controlMapping, midiMapping.mapSize);
#ifdef MIDI_MAP_FLEX_ENABLED
    Midi_CC_Map(channel, data1, data2, midiMapping.controlMapping_flex, midiMapping.mapSize_flex);
#endif

    if (data1 == 1)
    {
        if (midiMapping.modWheel != NULL)
        {
#ifdef MIDI_FMT_INT
            midiMapping.modWheel(channel, data2);
#else
            midiMapping.modWheel(channel, (float)data2 * NORM127MUL);
#endif
        }
    }
}

inline void Midi_PitchBend(uint8_t ch, uint16_t bend)
{
#ifdef MIDI_BLE_ENABLED
    Ble_PitchBend(ch, bend);
#endif

    float value = ((float)bend - 8192.0f) * (1.0f / 8192.0f);
    if (midiMapping.pitchBend != NULL)
    {
        midiMapping.pitchBend(ch, value);
    }
}

inline void Midi_SongPositionPointer(uint16_t pos)
{
#ifdef MIDI_BLE_ENABLED
    Ble_SongPos(pos);
#endif

    if (midiMapping.songPos != NULL)
    {
        midiMapping.songPos(pos);
    }
}

/*
 * function will be called when a short message has been received over midi
 */
inline void Midi_HandleShortMsg(uint8_t *data, uint8_t cable __attribute__((unused)))
{
    uint8_t ch = data[0] & 0x0F;

    switch (data[0] & 0xF0)
    {
    /* note on */
    case 0x90:
        if (data[2] > 0)
        {
            Midi_NoteOn(ch, data[1], data[2]);
        }
        else
        {
            Midi_NoteOff(ch, data[1]);
        }
        break;
    /* note off */
    case 0x80:
        Midi_NoteOff(ch, data[1]);
        break;
    case 0xb0:
        Midi_ControlChange(ch, data[1], data[2]);
        break;
    /* pitchbend */
    case 0xe0:
        Midi_PitchBend(ch, ((((uint16_t)data[1])) + ((uint16_t)data[2] << 7)));
        break;
    /* song position pointer */
    case 0xf2:
        Midi_SongPositionPointer(((((uint16_t)data[1])) + ((uint16_t)data[2] << 8)));
        break;
    }
}

inline void Midi_RealTimeMessage(uint8_t msg)
{
    if (midiMapping.rttMsg != NULL)
    {
        midiMapping.rttMsg(msg);
    }
}

static void Midi_PortSetup(struct midi_port_s *port)
{
    /* reset the watchdog variables */
    port->inMsgWd = 0;
    memset(port->inMsg, 0, sizeof(port->inMsg));
    port->inMsgIndex = 0;
}

void Midi_Setup()
{
#ifdef MIDI_RECV_FROM_SERIAL
    MidiPort.serial = &Serial;
    Serial.printf("MIDI listen on Serial with %d baud\n", MIDI_SERIAL_BAUDRATE);
#endif /* MIDI_RECV_FROM_SERIAL */

#ifdef MIDI_PORT_ACTIVE
#ifdef SWAP_SERIAL
    Serial.printf("Switch Serial to Midi!\n");
    delay(20);
    Serial.end();
    Serial.begin(MIDI_BAUDRATE);
    Serial.swap(); /* using alternative rx and tx pin for Midi communication */
    delay(20);
#endif
    MidiPort.serial = &Serial;
    Midi_PortSetup(&MidiPort);
#endif

#ifdef MIDI_PORT1_ACTIVE

#ifdef MIDI_RX1_PIN
#ifdef MIDI_TX1_PIN
    Serial.printf("Setup Serial1 with %d baud with rx: %d and tx %d\n", MIDI_SERIAL1_BAUDRATE, MIDI_RX1_PIN, MIDI_TX1_PIN);
    Serial1.begin(MIDI_SERIAL1_BAUDRATE, SERIAL_8N1, MIDI_RX1_PIN, MIDI_TX1_PIN);
#else
    Serial.printf("Setup Serial1 with %d baud with rx: %d only\n", MIDI_SERIAL1_BAUDRATE, MIDI_RX1_PIN);
    Serial1.begin(MIDI_SERIAL1_BAUDRATE, SERIAL_8N1, MIDI_RX1_PIN);
#endif
    pinMode(MIDI_RX1_PIN, INPUT_PULLUP); /* can be connected to open collector output */
#else
    Serial.printf("Setup Serial1 with %d baud with rx: RX1 pin\n", MIDI_SERIAL1_BAUDRATE);
    Serial1.begin(MIDI_SERIAL1_BAUDRATE);
#endif

#ifdef ARDUINO_SEEED_XIAO_M0
    pinMode(PIN_SERIAL1_RX, INPUT_PULLUP);
#endif

    MidiPort1.serial = &Serial1;
    Midi_PortSetup(&MidiPort1);
#endif /* MIDI_PORT1_ACTIVE */


#ifdef MIDI_PORT2_ACTIVE

#ifdef MIDI_RX2_PIN
#ifdef MIDI_TX2_PIN
    Serial.printf("Setup Serial2 with %d baud with rx: %d and tx %d\n", MIDI_SERIAL2_BAUDRATE, MIDI_RX2_PIN, MIDI_TX2_PIN);
    Serial2.begin(MIDI_SERIAL2_BAUDRATE, SERIAL_8N1, MIDI_RX2_PIN, MIDI_TX2_PIN);
#else
    Serial.printf("Setup Serial2 with %d baud with rx: %d only\n", MIDI_SERIAL2_BAUDRATE, MIDI_RX2_PIN);
    Serial2.begin(MIDI_SERIAL2_BAUDRATE, SERIAL_8N1, MIDI_RX2_PIN);
#endif
    pinMode(MIDI_RX2_PIN, INPUT_PULLUP); /* can be connected to open collector output */
#else
    Serial.printf("Setup Serial2 with %d baud with rx: RX2 pin\n", MIDI_SERIAL2_BAUDRATE);
    Serial2.begin(MIDI_SERIAL2_BAUDRATE);
#endif

    MidiPort2.serial = &Serial2;
    Midi_PortSetup(&MidiPort2);
    Serial.printf("Setup MidiPort2 using Serial2\n");
#endif /* MIDI_PORT2_ACTIVE */

#ifdef USB_MIDI_ENABLED
    UbsMidiSetup();
#endif
}

void Midi_CheckMidiPort(struct midi_port_s *port)
{
    //Choose Serial1 or Serial2 as required

    if (port->serial->available())
    {
        uint8_t incomingByte = port->serial->read();

#ifdef MIDI_DUMP_SERIAL2_TO_SERIAL
        Serial.printf("%02x", incomingByte);
#endif
        /* ignore live messages */
        if ((incomingByte & 0xF0) == 0xF0)
        {
            Midi_RealTimeMessage(incomingByte);
            return;
        }

        if (port->inMsgIndex == 0)
        {
            if ((incomingByte & 0x80) != 0x80)
            {
                port->inMsgIndex = 1;
            }
        }

        port->inMsg[port->inMsgIndex] = incomingByte;
        port->inMsgIndex += 1;

        if ((port->inMsgIndex >= 3) ||
                (
                    (((port->inMsg[0] & 0xF0) == 0xD0)
                     || ((port->inMsg[0] & 0xF0) == 0xC0))
                    && (port->inMsgIndex >= 2))
           )
        {
#ifdef MIDI_DUMP_SERIAL2_TO_SERIAL
            if (port->inMsgIndex >= 3)
            {
                Serial.printf("\n>%02x %02x %02x<\n", port->inMsg[0], port->inMsg[1], port->inMsg[2]);
            }
            else
            {
                Serial.printf("\n>%02x %02x<\n", port->inMsg[0], port->inMsg[1]);
            }
#endif
            Midi_HandleShortMsg(port->inMsg, 0);
            port->inMsgIndex = 0;
        }

        /*
         * reset watchdog to allow new bytes to be received
         */
        port->inMsgWd = 0;
    }
    else
    {
        if (port->inMsgIndex > 0)
        {
            port->inMsgWd++;
            if (port->inMsgWd == 0xFFF)
            {
                port->inMsgIndex = 0;
            }
        }
    }
}

/*
 * this function should be called continuously to ensure that incoming messages can be processed
 */
inline
void Midi_Process()
{
#ifdef MIDI_PORT_ACTIVE
    Midi_CheckMidiPort(&MidiPort);
#endif
#ifdef MIDI_PORT1_ACTIVE
    Midi_CheckMidiPort(&MidiPort1);
#endif
#ifdef MIDI_PORT2_ACTIVE
    Midi_CheckMidiPort(&MidiPort2);
#endif
#ifdef USB_MIDI_ENABLED
    UbsMidiLoop();
#endif
}

#ifndef ARDUINO_SEEED_XIAO_M0
#ifndef SWAP_SERIAL
#ifdef MIDI_TX2_PIN
void Midi_SendShortMessage(uint8_t *msg)
{
    MidiPort2.serial->write(msg, 3);
}

void Midi_SendRaw(uint8_t *msg)
{
    /* sysex */
    if (msg[0] == 0xF0)
    {
        int i = 2;
        while (msg[i] != 0xF7)
        {
            i++;
        }
        MidiPort2.serial->write(msg, i + 1);
    }
    else
    {
        MidiPort2.serial->write(msg, 3);
    }
}
#endif /* MIDI_TX2_PIN */
#endif
#endif

#ifdef MIDI_MAP_FLEX_ENABLED
void Midi_SetMidiMap(struct midiControllerMapping *controlMapping, int mapSize)
{
    midiMapping.controlMapping_flex = controlMapping;
    midiMapping.mapSize_flex = mapSize;
}

void Midi_SetMidiMapByIndex(uint8_t index, float value)
{
    if (index < midiMapLookUpCnt)
    {
        Midi_SetMidiMap(midiMapLookUp[index].controlMap, midiMapLookUp[index].controlMapSize);
        Serial.printf("Midi map %s selected\n", midiMapLookUp[index].desc);
    }
}
#endif

