/*
 * z_config.ino
 *
 * Put all your project configuration here (no defines etc)
 * This file will be included at the and can access all
 * declarations and type definitions
 *
 *  Created on: 12.05.2021
 *      Author: Marcel Licence
 */

/*
 * adc to midi mapping
 */
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

/*
 * this mapping is used for the edirol pcr-800
 * this should be changed when using another controller
 */
struct midiControllerMapping edirolMapping[] =
{
    /* transport buttons */
    { 0x8, 0x52, "back", NULL, NULL, 0},
    { 0xD, 0x52, "stop", NULL, NULL, 0},
    { 0xe, 0x52, "start", NULL, NULL, 0},
    { 0xe, 0x52, "start", NULL, NULL, 0},
    { 0xa, 0x52, "rec", NULL, NULL, 0},

    /* upper row of buttons */
    { 0x0, 0x50, "A1", NULL, NULL, 0},
    { 0x1, 0x50, "A2", NULL, NULL, 1},
    { 0x2, 0x50, "A3", NULL, NULL, 2},
    { 0x3, 0x50, "A4", NULL, NULL, 3},

    { 0x4, 0x50, "A5", NULL, NULL, 0},
    { 0x5, 0x50, "A6", NULL, NULL, 1},
    { 0x6, 0x50, "A7", NULL, NULL, 2},
    { 0x7, 0x50, "A8", NULL, NULL, 3},

    { 0x0, 0x53, "A9", NULL, NULL, 0},

    /* lower row of buttons */
    { 0x0, 0x51, "B1", NULL, NULL, 0},
    { 0x1, 0x51, "B2", NULL, NULL, 1},
    { 0x2, 0x51, "B3", NULL, NULL, 2},
    { 0x3, 0x51, "B4", NULL, NULL, 3},

    { 0x4, 0x51, "B5", NULL, NULL, 4},
    { 0x5, 0x51, "B6", NULL, NULL, 5},
    { 0x6, 0x51, "B7", NULL, NULL, 6},
    { 0x7, 0x51, "B8", NULL, NULL, 7},

    { 0x1, 0x53, "B9", NULL, NULL, 8},

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

    { 0x1, 0x12, "S9", NULL, Synth_SetParam, 8},

    /* rotary */
#ifdef USE_UNISON
    { 0x0, 0x10, "R1", NULL, Synth_SetParam, SYNTH_PARAM_DETUNE_1},
    { 0x1, 0x10, "R2", NULL, Synth_SetParam, SYNTH_PARAM_UNISON_2},
#else
    { 0x0, 0x10, "R1", NULL, Synth_SetParam, SYNTH_PARAM_WAVEFORM_1},
    { 0x1, 0x10, "R2", NULL, Synth_SetParam, SYNTH_PARAM_WAVEFORM_2},
#endif
    { 0x2, 0x10, "R3", NULL, Delay_SetLength, 2},
    { 0x3, 0x10, "R4", NULL, Delay_SetLevel, 3},

    { 0x4, 0x10, "R5", NULL, Delay_SetFeedback, 4},
    { 0x5, 0x10, "R6", NULL, Synth_SetParam, SYNTH_PARAM_MAIN_FILT_CUTOFF},
    { 0x6, 0x10, "R7", NULL, Synth_SetParam, SYNTH_PARAM_MAIN_FILT_RESO},
    { 0x7, 0x10, "R8", NULL, Synth_SetParam, SYNTH_PARAM_VOICE_FILT_RESO},

    { 0x0, 0x12, "R9", NULL, Synth_SetParam, SYNTH_PARAM_VOICE_NOISE_LEVEL},

    /* Central slider */
    { 0x0, 0x13, "H1", NULL, NULL, 0},
};

struct midiMapping_s midiMapping =
{
    Synth_NoteOn,
    Synth_NoteOff,
    Synth_PitchBend,
    Synth_ModulationWheel,
    edirolMapping,
    sizeof(edirolMapping) / sizeof(edirolMapping[0]),
};
