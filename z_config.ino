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
 * @file z_config.ino
 * @author Marcel Licence
 * @date 12.05.2021
 *
 * @brief This file contains the mapping configuration
 * Put all your project configuration here (no defines etc)
 * This file will be included at the and can access all
 * declarations and type definitions
 *
 * @see ESP32 Arduino DIY Synthesizer Projects - Little startup guide to get your MIDI synth working - https://youtu.be/ZNxGCB-d68g
 */


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


#ifdef AUDIO_KIT_BUTTON_ANALOG
audioKitButtonCb audioKitButtonCallback = App_ButtonCb;
#endif

/*
 * adc to midi mapping
 */
#ifdef ADC_TO_MIDI_ENABLED
struct adc_to_midi_s adcToMidiLookUp[ADC_TO_MIDI_LOOKUP_SIZE] =
{
    {0, 0x10},
    {1, 0x10},
    {2, 0x10},
    {3, 0x10},
    {4, 0x10},
    {5, 0x10},
    {6, 0x10},
    {7, 0x10},
};


struct adc_to_midi_mapping_s adcToMidiMapping =
{
    adcToMidiLookUp,
    sizeof(adcToMidiLookUp) / sizeof(adcToMidiLookUp[0]),
    //Midi_ControlChange,
#ifdef MIDI_VIA_USB_ENABLED
    UsbMidi_SendControlChange,
#else
    Midi_ControlChange,
#endif
};

#endif

/*
 * this mapping is used for the edirol pcr-800
 * this should be changed when using another controller
 */
struct midiControllerMapping edirolMapping[] =
{
    /* transport buttons */
    { 0x8, 0x52, "back", NULL, Synth_SongPosReset, 0},
    { 0xD, 0x52, "stop", NULL, NULL, 0},
    { 0xe, 0x52, "start", NULL, NULL, 0},
    { 0xe, 0x52, "start", NULL, NULL, 0},
    { 0xa, 0x52, "rec", NULL, NULL, 0},

    /* upper row of buttons */
    { 0x0, 0x50, "A1", NULL, Synth_SetCurCh, 0},
    { 0x1, 0x50, "A2", NULL, Synth_SetCurCh, 1},
    { 0x2, 0x50, "A3", NULL, NULL, 2},
    { 0x3, 0x50, "A4", NULL, NULL, 3},

    { 0x4, 0x50, "A5", NULL, Synth_SetCurCh, 0},
    { 0x5, 0x50, "A6", NULL, Synth_SetCurCh, 1},
    { 0x6, 0x50, "A7", NULL, MidiCtrl_TransposeUp, 0},
    { 0x7, 0x50, "A8", NULL, MidiCtrl_TransposeDown, 0},

    { 0x0, 0x53, "A9", NULL, Synth_ToggleMono, 0},

    /* lower row of buttons */
#ifdef ARP_MODULE_ENABLED
    { 0x0, 0x51, "B1", NULL, Arp_SelectSequence, 0},
    { 0x1, 0x51, "B2", NULL, Arp_SelectSequence, 1},
    { 0x2, 0x51, "B3", NULL, Arp_SelectSequence, 2},
    { 0x3, 0x51, "B4", NULL, Arp_SelectSequence, 3},

    { 0x4, 0x51, "B5", NULL, Arp_SelectSequence, 4},
    { 0x5, 0x51, "B6", NULL, Arp_SelectSequence, 5},
    { 0x6, 0x51, "B7", NULL, Arp_SelectSequence, 6},
    { 0x7, 0x51, "B8", NULL, Arp_SelectSequence, 7},

    { 0x1, 0x53, "B9", NULL, Arp_StartRecord, 8},
#else
    { 0x0, 0x51, "B1", NULL, NULL, 0},
    { 0x1, 0x51, "B2", NULL, NULL, 1},
    { 0x2, 0x51, "B3", NULL, NULL, 2},
    { 0x3, 0x51, "B4", NULL, NULL, 3},

    { 0x4, 0x51, "B5", NULL, NULL, 4},
    { 0x5, 0x51, "B6", NULL, NULL, 5},
    { 0x6, 0x51, "B7", NULL, NULL, 6},
    { 0x7, 0x51, "B8", NULL, NULL, 7},

    { 0x1, 0x53, "B9", NULL, NULL, 8},
#endif

    /* pedal */
    { 0x0, 0x0b, "VolumePedal", NULL, NULL, 0},

    /* slider */
    { 0x0, 0x11, "S1", NULL, Synth_SetParam, SYNTH_PARAM_VEL_ENV_ATTACK},
    { 0x1, 0x11, "S2", NULL, Synth_SetParam, SYNTH_PARAM_VEL_ENV_DECAY},
    { 0x2, 0x11, "S3", NULL, Synth_SetParam, SYNTH_PARAM_VEL_ENV_SUSTAIN},
    { 0x3, 0x11, "S4", NULL, Synth_SetParam, SYNTH_PARAM_VEL_ENV_RELEASE},

    { 0x4, 0x11, "S5", NULL, Synth_SetParam, SYNTH_PARAM_FIL_ENV_ATTACK},
    { 0x5, 0x11, "S6", NULL, Synth_SetParam, SYNTH_PARAM_FIL_ENV_DECAY},
    { 0x6, 0x11, "S7", NULL, Synth_SetParam, SYNTH_PARAM_FIL_ENV_SUSTAIN},
    { 0x7, 0x11, "S8", NULL, Synth_SetParam, SYNTH_PARAM_FIL_ENV_RELEASE},

#ifdef ARP_MODULE_ENABLED
    { 0x1, 0x12, "S9", NULL, Arp_Tempo, 0},
#else
    { 0x1, 0x12, "S9", NULL, Synth_SetParam, 8},
#endif

    /* rotary */
#ifdef USE_UNISON
    { 0x0, 0x10, "R1", NULL, Synth_SetParam, SYNTH_PARAM_DETUNE_1},
    { 0x1, 0x10, "R2", NULL, Synth_SetParam, SYNTH_PARAM_UNISON_2},
#else
    { 0x0, 0x10, "R1", NULL, Synth_SetParam, SYNTH_PARAM_WAVEFORM_1},
    { 0x1, 0x10, "R2", NULL, Synth_SetParam, SYNTH_PARAM_WAVEFORM_2},
#endif
    { 0x2, 0x10, "R3", NULL, Delay_SetLength, 2},
    { 0x3, 0x10, "R4", NULL, Delay_SetOutputLevel, 3},

    { 0x4, 0x10, "R5", NULL, Delay_SetFeedback, 4},
    { 0x5, 0x10, "R6", NULL, Synth_SetParam, SYNTH_PARAM_MAIN_FILT_CUTOFF},
    { 0x6, 0x10, "R7", NULL, Synth_SetParam, SYNTH_PARAM_MAIN_FILT_RESO},
    //{ 0x7, 0x10, "R8", NULL, Synth_SetParam, SYNTH_PARAM_VOICE_FILT_RESO},
    { 0x7, 0x10, "R8", NULL, Synth_SetParam, SYNTH_PARAM_VOICE_PORT_TIME},

    //{ 0x0, 0x12, "R9", NULL, Synth_SetParam, SYNTH_PARAM_VOICE_NOISE_LEVEL},
    { 0x0, 0x12, "R9", NULL, Reverb_SetLevel, SYNTH_PARAM_VOICE_NOISE_LEVEL},

    /* Central slider */
#ifdef MIDI_SYNC_MASTER
    { 0x0, 0x13, "H1", NULL, Synth_SetMidiMasterTempo, 0},
#endif
};

struct midiMapping_s midiMapping =
{
    NULL,
#ifdef MIDI_CTRL_ENABLED
    MidiCtrl_NoteOn,
    MidiCtrl_NoteOff,
#else
    Synth_NoteOn,
    Synth_NoteOff,
#endif
    Synth_PitchBend,
    Synth_ModulationWheel,
    Synth_RealTimeMsg,
    Synth_SongPosition,
    edirolMapping,
    sizeof(edirolMapping) / sizeof(edirolMapping[0]),
};

#ifdef MIDI_VIA_USB_ENABLED
struct usbMidiMappingEntry_s usbMidiMappingEntries[] =
{
    {
        NULL,
        App_UsbMidiShortMsgReceived,
        NULL,
        NULL,
        0xFF,
    },
};

struct usbMidiMapping_s usbMidiMapping =
{
    NULL,
    NULL,
    usbMidiMappingEntries,
    sizeof(usbMidiMappingEntries) / sizeof(usbMidiMappingEntries[0]),
};
#endif /* MIDI_VIA_USB_ENABLED */
