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

/*
 * Implementation of a simple polyphonic synthesizer module
 * - it supports different waveforms
 * - it supports polyphony
 * - implemented ADSR for velocity and filter
 * - allows usage of multiple oscillators per voice
 *
 */


#ifdef __CDT_PARSER__
#include "cdt.h"
#endif


/* requires the ML_SynthTools library: https://github.com/marcel-licence/ML_SynthTools */
#include <ml_filter.h>
#include <ml_waveform.h>

/*
 * activate the following macro to enable unison mode
 * by default the saw wave form will be used
 * the waveform controllers are remapped to
 * - waveform1 -> detune
 * - waveform2 -> oscillator count
 */
//#define USE_UNISON

#define CHANNEL_MAX 16

/*
 * Param indices for Synth_SetParam function
 */
#define SYNTH_PARAM_VEL_ENV_ATTACK  0
#define SYNTH_PARAM_VEL_ENV_DECAY   1
#define SYNTH_PARAM_VEL_ENV_SUSTAIN 2
#define SYNTH_PARAM_VEL_ENV_RELEASE 3
#define SYNTH_PARAM_FIL_ENV_ATTACK  4
#define SYNTH_PARAM_FIL_ENV_DECAY   5
#define SYNTH_PARAM_FIL_ENV_SUSTAIN 6
#define SYNTH_PARAM_FIL_ENV_RELEASE 7
#ifdef USE_UNISON
#define SYNTH_PARAM_DETUNE_1        8
#define SYNTH_PARAM_UNISON_2        9
#else
#define SYNTH_PARAM_WAVEFORM_1      8
#define SYNTH_PARAM_WAVEFORM_2      9
#endif
#define SYNTH_PARAM_MAIN_FILT_CUTOFF    10
#define SYNTH_PARAM_MAIN_FILT_RESO      11
#define SYNTH_PARAM_VOICE_FILT_RESO     12
#define SYNTH_PARAM_VOICE_NOISE_LEVEL   13

#define SYNTH_PARAM_VOICE_PORT_TIME     14

/*
 * Following defines can be changed for different puprposes
 */
#ifdef USE_UNISON
/* use another setting, because unison supports more than 2 osc per voice */
#define MAX_DETUNE      12 /* 1 + 11 additional tones */
#define MAX_POLY_OSC    36 /* osc polyphony, always active reduces single voices max poly */
#define MAX_POLY_VOICE  3  /* max single voices, can use multiple osc */
#else
#define MAX_POLY_OSC    22 /* osc polyphony, always active reduces single voices max poly */
#define MAX_POLY_VOICE  11 /* max single voices, can use multiple osc */
#endif


#define MIDI_NOTE_CNT 128
static uint32_t midi_note_to_add[MIDI_NOTE_CNT]; /* lookup to playback waveforms with correct frequency */

#ifdef USE_UNISON
uint32_t midi_note_to_add50c[MIDI_NOTE_CNT]; /* lookup for detuning */
#endif

/*
 * set the correct count of available waveforms
 */
#define WAVEFORM_TYPE_COUNT 7

/*
 * add here your waveforms
 */
#if 0
float *sine = NULL;
#else
float sine[WAVEFORM_CNT];
#endif
float *saw = NULL;
float *square = NULL;
float *pulse = NULL;
float *tri = NULL;
float *crappy_noise = NULL;
float *silence = NULL;

/*
 * do not forget to enter the waveform pointer addresses here
 */
float *waveFormLookUp[WAVEFORM_TYPE_COUNT];

struct adsrT
{
    float a;
    float d;
    float s;
    float r;
};

typedef enum
{
    attack, decay, sustain, release
} adsr_phaseT;

/* this prototype is required .. others not -  i still do not know what magic arduino is doing */
inline bool ADSR_Process(const struct adsrT *ctrl, float *ctrlSig, adsr_phaseT *phase);


static struct filterCoeffT filterGlobalC;
static struct filterProcT mainFilterL, mainFilterR;


#define NOTE_STACK_MAX  8


struct channelSetting_s
{
#ifdef USE_UNISON
    float detune; /* detune parameter */
    uint8_t unison; /* additional osc per voice count */
    float *selectedWaveForm;
    float *selectedWaveForm2;
#else
    float *selectedWaveForm;
    float *selectedWaveForm2;
#endif

    float soundFiltReso;
    float soundNoiseLevel;

    struct adsrT adsr_vol;
    struct adsrT adsr_fil;

    /* modulation */
    float modulationDepth;
    float modulationSpeed;
    float modulationPitch;

    /* pitchbend */
    float pitchBendValue;
    float pitchMultiplier;

    /* mono mode variables */
    bool mono;

    float portAdd;
    float port;
    float noteA;
    float noteB;

    uint32_t noteCnt;
    uint32_t noteStack[NOTE_STACK_MAX];
};

static struct channelSetting_s chCfg[CHANNEL_MAX];
static struct channelSetting_s *curChCfg = &chCfg[1];

struct oscillatorT
{
    float **waveForm;
    float *dest;
    uint32_t samplePos;
    uint32_t addVal;
    float pan_l;
    float pan_r;
    struct channelSetting_s *cfg;
};

float voiceSink[2];
struct oscillatorT oscPlayer[MAX_POLY_OSC];

static uint32_t osc_act = 0;

struct notePlayerT
{
    float lastSample[2];

    float velocity;
    bool active;
    adsr_phaseT phase;

    uint8_t midiCh;
    uint8_t midiNote;

    float control_sign;
    float out_level;

    struct filterCoeffT filterC;
    struct filterProcT filterL;
    struct filterProcT filterR;
    float f_control_sign;
    float f_control_sign_slow;
    adsr_phaseT f_phase;

    struct channelSetting_s *cfg;
};



struct notePlayerT voicePlayer[MAX_POLY_VOICE];

uint32_t voc_act = 0;



void Synth_Init()
{
#ifdef ESP32
    randomSeed(34547379);
#endif

    /*
     * we do not check if malloc was successful
     * if there is not enough memory left the application will crash
     */
#if 0
    sine = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
#endif
    saw = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
    square = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
    pulse = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
    tri = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
    crappy_noise = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
    silence = (float *)malloc(sizeof(float) * WAVEFORM_CNT);


    /*
     * let us calculate some waveforms
     * - using lookup tables can save a lot of processing power later
     * - but it does consume memory
     */
    for (int i = 0; i < WAVEFORM_CNT; i++)
    {
        float val = (float)sin(i * 2.0 * PI / WAVEFORM_CNT);
        sine[i] = val;
        saw[i] = (2.0f * ((float)i) / ((float)WAVEFORM_CNT)) - 1.0f;
        square[i] = (i > (WAVEFORM_CNT / 2)) ? 1 : -1;
        pulse[i] = (i > (WAVEFORM_CNT / 4)) ? 1.0f / 4.0f : -3.0f / 4.0f;
        tri[i] = ((i > (WAVEFORM_CNT / 2)) ? (((4.0f * (float)i) / ((float)WAVEFORM_CNT)) - 1.0f) : (3.0f - ((4.0f * (float)i) / ((float)WAVEFORM_CNT)))) - 2.0f;
        crappy_noise[i] = (random(1024) / 512.0f) - 1.0f;
        silence[i] = 0;
    }

    waveFormLookUp[0] = sine;
    waveFormLookUp[1] = saw;
    waveFormLookUp[2] = square;
    waveFormLookUp[3] = pulse;
    waveFormLookUp[4] = tri;
    waveFormLookUp[5] = crappy_noise;
    waveFormLookUp[6] = silence;

    /*
     * initialize all oscillators
     */
    for (int i = 0; i < MAX_POLY_OSC; i++)
    {
        oscillatorT *osc = &oscPlayer[i];
        osc->waveForm = &silence;
        osc->dest = voiceSink;
        osc->cfg = &chCfg[0];
    }

    /*
     * initialize all voices
     */
    for (int i = 0; i < MAX_POLY_VOICE; i++)
    {
        notePlayerT *voice = &voicePlayer[i];
        voice->active = false;
        voice->lastSample[0] = 0.0f;
        voice->lastSample[1] = 0.0f;
        voice->filterL.filterCoeff = &voice->filterC;
        voice->filterR.filterCoeff = &voice->filterC;
        voice->cfg = &chCfg[0];
    }

    /*
     * prepare lookup for constants to drive oscillators
     */
    for (int i = 0; i < MIDI_NOTE_CNT; i++)
    {
        float f = ((pow(2.0f, (float)(i - 69) / 12.0f) * 440.0f));
        uint32_t add = (uint32_t)(f * ((float)(1ULL << 32ULL) / ((float)SAMPLE_RATE)));
        midi_note_to_add[i] = add;
#ifdef USE_UNISON
        /* filling the table which will be used for detuning */
        float f1 = (pow(2.0f, ((float)(i - 69) + 0.5f) / 12.0f) * 440.0f);
        float f2 = (pow(2.0f, ((float)(i - 69) - 0.5f) / 12.0f) * 440.0f);

        midi_note_to_add50c[i] = (uint32_t)((f1 - f2) * ((float)(1ULL << 32ULL) / ((float)SAMPLE_RATE)));
#endif
    }

    /*
     * assign main filter
     */
    mainFilterL.filterCoeff = &filterGlobalC;
    mainFilterR.filterCoeff = &filterGlobalC;

    Filter_Proc_Init(&mainFilterL);
    Filter_Proc_Init(&mainFilterR);
    Filter_Coeff_Init(mainFilterL.filterCoeff);

    Filter_Calculate(1.0f, 1.0f, &filterGlobalC);

    for (int i = 0; i < CHANNEL_MAX; i++)
    {
        Synth_ChannelSettingInit(&chCfg[i]);
    }
}

static struct filterCoeffT mainFilt;

static float filtCutoff = 1.0f;
static float filtReso = 0.5f;

static void Synth_ChannelSettingInit(struct channelSetting_s *setting)
{
#ifdef USE_UNISON
    setting->detune = 0.1; /* detune parameter */
    setting->unison = 0; /* additional osc per voice count */
    setting->selectedWaveForm = saw;
    setting->selectedWaveForm2 = saw;
#else
    setting->selectedWaveForm = pulse;
    setting->selectedWaveForm2 = silence;
#endif

    setting->soundFiltReso = 0.5f;
    setting->soundNoiseLevel = 0.0f;

    struct adsrT adsr_vol_def = {1.0f, 0.25f, 1.0f, 0.01f};
    struct adsrT adsr_fil_def = {1.0f, 0.25f, 1.0f, 0.01f};

    memcpy(&setting->adsr_vol, &adsr_vol_def, sizeof(adsr_vol_def));
    memcpy(&setting->adsr_fil, &adsr_fil_def, sizeof(adsr_vol_def));

    setting->modulationDepth = 0.0f;
    setting->modulationSpeed = 5.0f;
    setting->modulationPitch = 1.0f;

    setting->pitchBendValue = 0.0f;
    setting->pitchMultiplier = 1.0f;

    setting->mono = true;
    setting->portAdd = 0.01f; /*!< speed of portamento */
    setting->port = 1.0f;
    setting->noteA = 0;
    setting->noteB = 0;

    setting->noteCnt = 0;
    /* setting->noteStack[NOTE_STACK_MAX]; can be left uninitialized */
}

/*
 * very bad and simple implementation of ADSR
 * - but it works for the start
 */
inline bool ADSR_Process(const struct adsrT *ctrl, float *ctrlSig, adsr_phaseT *phase)
{
    switch (*phase)
    {
    case attack:
        *ctrlSig += ctrl->a;
        if (*ctrlSig > 1.0f)
        {
            *ctrlSig = 1.0f;
            *phase = decay;
        }
        break;
    case decay:
        *ctrlSig -= ctrl->d;
        if (*ctrlSig < ctrl->s)
        {
            *ctrlSig = ctrl->s;
            *phase = sustain;
        }
        break;
    case sustain:
        break;
    case release:
        *ctrlSig -= ctrl->r;
        if (*ctrlSig < 0.0f)
        {
            *ctrlSig = 0.0f;
            //voice->active = false;
            return false;
        }
    }
    return true;
}

void Voice_Off(uint32_t i)
{
    notePlayerT *voice = &voicePlayer[i];
    for (int f = 0; f < MAX_POLY_OSC; f++)
    {
        oscillatorT *osc = &oscPlayer[f];
        if (osc->dest == voice->lastSample)
        {
            osc->dest = voiceSink;
            osc_act -= 1;
        }
    }
    voc_act -= 1;
}

inline
float SineNorm(float alpha_div2pi)
{
    uint32_t index = ((uint32_t)(alpha_div2pi * ((float)WAVEFORM_CNT))) % WAVEFORM_CNT;
    return sine[index];
}

inline
float GetModulation(uint8_t ch)
{
    float modSpeed = chCfg[ch].modulationSpeed;
    return chCfg[ch].modulationDepth * chCfg[ch].modulationPitch * (SineNorm((modSpeed * ((float)millis()) / 1000.0f)));
}

static uint32_t count = 0;

//[[gnu::noinline, gnu::optimize ("fast-math")]]
inline void Synth_Process(float *left, float *right, uint32_t len)
{
    /*
     * update pitch bending / modulation
     */
    {

        for (int i = 0; i < CHANNEL_MAX; i++)
        {
            float modulation = GetModulation(i);

            chCfg[i].port += chCfg[i].portAdd; /* active portamento */
            chCfg[i].port = chCfg[i].port > 1.0f ? 1.0f : chCfg[i].port; /* limit value to max of 1.0f */

            float portVal = (((float)(chCfg[i].noteA)) * (1.0f - chCfg[i].port) + ((float)(chCfg[i].noteB)) * chCfg[i].port);

            float pitchVar = chCfg[i].pitchBendValue + modulation + portVal;
#if 0
            static float lastPitchVar = 0;
#endif
            chCfg[i].pitchMultiplier = pow(2.0f, pitchVar / 12.0f);
        }
    }

    for (uint32_t n = 0; n < len; n++)
    {

        /* gerenate a noise signal */
        float noise_signal = ((random(1024) / 512.0f) - 1.0f);

        /* counter required to optimize processing */
        count += 1;

        /*
         * destination for unused oscillators
         */
        voiceSink[0] = 0;
        voiceSink[1] = 0;

        /*
         * oscillator processing -> mix to voice
         */
        for (int i = 0; i < MAX_POLY_OSC; i++)
        {
            oscillatorT *osc = &oscPlayer[i];
            {
                osc->samplePos += (uint32_t)(osc->cfg->pitchMultiplier * ((float)osc->addVal));
                float sig = (*osc->waveForm)[WAVEFORM_I(osc->samplePos)];
                osc->dest[0] += osc->pan_l * sig;
                osc->dest[1] += osc->pan_r * sig;
            }
        }

        /*
         * voice processing
         */
        for (int i = 0; i < MAX_POLY_VOICE; i++) /* one loop is faster than two loops */
        {
            notePlayerT *voice = &voicePlayer[i];
            if (voice->active)
            {
                if (n % 4 == 0)
                {
                    voice->active = ADSR_Process(&voice->cfg->adsr_vol, &voice->control_sign, &voice->phase);
                    if (voice->active == false)
                    {
                        Voice_Off(i);
                    }
                    /*
                     * make is slow to avoid bad things .. or crying ears
                     */
                    (void)ADSR_Process(&voice->cfg->adsr_fil, &voice->f_control_sign, &voice->f_phase);
                }

                /* add some noise to the voice */
                voice->lastSample[0] += noise_signal * voice->cfg->soundNoiseLevel;
                voice->lastSample[1] += noise_signal * voice->cfg->soundNoiseLevel;

                voice->lastSample[0] *= voice->control_sign * voice->velocity;
                voice->lastSample[1] *= voice->control_sign * voice->velocity;

                if (count % 32 == 0)
                {
                    voice->f_control_sign_slow = 0.05 * voice->f_control_sign + 0.95 * voice->f_control_sign_slow;
                    Filter_Calculate(voice->f_control_sign_slow, voice->cfg->soundFiltReso, &voice->filterC);
                }

                Filter_Process(&voice->lastSample[0], &voice->filterL);
                Filter_Process(&voice->lastSample[1], &voice->filterR);

                left[n] += voice->lastSample[0];
                right[n] += voice->lastSample[1];
                voice->lastSample[0] = 0.0f;
                voice->lastSample[1] = 0.0f;
            }
        }
    }

    /*
     * process main filter
     */
    Filter_Process_Buffer(left, &mainFilterL, len);
    Filter_Process_Buffer(right, &mainFilterR, len);

    /*
     * reduce level a bit to avoid distortion
     */
    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++)
    {
        left[i] *= 0.4f * 0.25f;
        right[i] *= 0.4f * 0.25f;
    }

#ifdef LIMITER_ACTIVE
    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++)
    {
        left[i] = left[i] > 0.5f ? 0.5 : left[i];
        left[i] = left[i] < -0.5f ? -0.5 : left[i];
        right[i] = right[i] > 0.5f ? 0.5 : right[i];
        right[i] = right[i] < -0.5f ? -0.5 : right[i];
    }
#endif
}

struct oscillatorT *getFreeOsc()
{
    for (int i = 0; i < MAX_POLY_OSC ; i++)
    {
        if (oscPlayer[i].dest == voiceSink)
        {
            return &oscPlayer[i];
        }
    }
    return NULL;
}

static struct notePlayerT *getFreeVoice(void)
{
    for (int i = 0; i < MAX_POLY_VOICE ; i++)
    {
        if (voicePlayer[i].active == false)
        {
            return &voicePlayer[i];
        }
    }
    return NULL;
}

inline void Synth_NoteOn(uint8_t ch, uint8_t note, float vel)
{
    struct notePlayerT *voice = getFreeVoice();
    struct oscillatorT *osc = getFreeOsc();

    /* put note onto stack */
    if (chCfg[ch].mono)
    {
        if (chCfg[ch].noteCnt < (NOTE_STACK_MAX - 1))
        {
            chCfg[ch].noteStack[chCfg[ch].noteCnt] = note;
            chCfg[ch].noteCnt++;
            //Status_ValueChangedIntArr("noteCnt", chCfg[ch].noteCnt, ch);
        }

        if (chCfg[ch].noteCnt > 1)
        {
            for (int i = 0; i < MAX_POLY_VOICE ; i++)
            {
                if ((voicePlayer[i].active) && (voicePlayer[i].midiCh == ch))
                {
                    float diff = note - voicePlayer[i].midiNote;

                    voicePlayer[i].cfg->noteA = voicePlayer[i].cfg->port * ((float)voicePlayer[i].cfg->noteB) + (1.0f - voicePlayer[i].cfg->port) * voicePlayer[i].cfg->noteA;
                    voicePlayer[i].cfg->port = 0.0f;

                    voicePlayer[i].cfg->noteB += diff;
                    voicePlayer[i].midiNote = note;

                    return;
                }
            }
        }
    }

    /*
     * No free voice found, return otherwise crash xD
     */
    if ((voice == NULL) || (osc == NULL))
    {
        //Serial.printf("voc: %d, osc: %d\n", voc_act, osc_act);
        return ;
    }

    voice->cfg = &chCfg[ch];
    voice->midiCh = ch;
    voice->midiNote = note;
#ifdef MIDI_USE_CONST_VELOCITY
    voice->velocity = 1.0f;
#else
    voice->velocity = vel;
#endif
    voice->lastSample[0] = 0.0f;
    voice->lastSample[1] = 0.0f;
    voice->control_sign = 0.0f;

    /* default values to avoid portamento */
    voice->cfg->port = 1.0f;
    voice->cfg->noteB = 0;

#if 1
    voice->f_phase = attack;
    if (voice->cfg->adsr_fil.a == 1)
    {
        voice->f_phase = decay;
    }
#else
    if (voice->cfg->adsr_fil.a < voice->cfg->adsr_fil.s)
    {
        voice->cfg->adsr_fil.a = voice->cfg->adsr_fil.s;
    }
    voice->f_phase = decay;
#endif
    voice->f_control_sign = voice->cfg->adsr_fil.a;
    voice->f_control_sign_slow = voice->cfg->adsr_fil.a;
    voice->active = true;
    voice->phase = attack;

    /* update all values to avoid audible artifacts */
    ADSR_Process(&voice->cfg->adsr_vol, &voice->control_sign, &voice->phase);
    ADSR_Process(&voice->cfg->adsr_fil, &voice->f_control_sign, &voice->f_phase);

    Filter_Calculate(voice->f_control_sign_slow, voice->cfg->soundFiltReso, &voice->filterC);

    voc_act += 1;

    /*
     * add oscillator
     */
#ifdef USE_UNISON
    if (voice->cfg->unison > 0)
    {
        /*
         * shift first oscillator down
         */
        osc->addVal = midi_note_to_add[note] + ((0 - (voice->cfg->unison * 0.5)) * midi_note_to_add50c[note] * voice->cfg->detune / voice->cfg->unison);
    }
    else
#endif
    {
        osc->addVal = midi_note_to_add[note];
    }
    osc->samplePos = 0;
    osc->waveForm = &chCfg[ch].selectedWaveForm;
    osc->dest = voice->lastSample;
    osc->pan_l = 1;
    osc->pan_r = 1;

    osc->cfg = &chCfg[ch];

    osc_act += 1;

#ifdef USE_UNISON

    int8_t pan = 1;

    /*
     * attach more oscillators to voice
     */
    for (int i = 0; i < voice->cfg->unison; i++)
    {
        osc = getFreeOsc();
        if (osc == NULL)
        {
            //Serial.printf("voc: %d, osc: %d\n", voc_act, osc_act);
            return ;
        }

        osc->addVal = midi_note_to_add[note] + ((i + 1 - (voice->cfg->unison * 0.5)) * midi_note_to_add50c[note] * voice->cfg->detune / voice->cfg->unison);
        osc->samplePos = (uint32_t)random(1 << 31); /* otherwise it sounds ... bad!? */
        osc->waveForm = &chCfg[ch].selectedWaveForm2;
        osc->dest = voice->lastSample;

        /*
         * put last osc in the middle
         */
        if ((voice->cfg->unison - 1) == i)
        {
            osc->pan_l = 1;
            osc->pan_r = 1;
        }
        else if (pan == 1)
        {
            osc->pan_l = 1;
            osc->pan_r = 0.5;
        }
        else
        {
            osc->pan_l = 0.5;
            osc->pan_r = 1;
        }
        pan = -pan; /* make a stereo sound by putting the oscillator left/right */

        osc->cfg = &chCfg[ch];

        osc_act += 1;
    }
#else
    osc = getFreeOsc();
    if (osc != NULL)
    {
        if (note + 12 < 128)
        {
            osc->addVal = midi_note_to_add[note + 12];
            osc->samplePos = 0; /* we could add some offset maybe */
            osc->waveForm = &chCfg[ch].selectedWaveForm2;
            osc->dest = voice->lastSample;
            osc->pan_l = 1;
            osc->pan_r = 1;

            osc->cfg = &chCfg[ch];

            osc_act += 1;
        }
    }
#endif

    /*
     * trying to avoid audible suprises
     */
    Filter_Reset(&voice->filterL);
    Filter_Reset(&voice->filterR);
    Filter_Process(&voice->lastSample[0], &voice->filterL);
    Filter_Process(&voice->lastSample[0], &voice->filterL);
    Filter_Process(&voice->lastSample[0], &voice->filterL);

    Filter_Process(&voice->lastSample[1], &voice->filterR);
    Filter_Process(&voice->lastSample[1], &voice->filterR);
    Filter_Process(&voice->lastSample[1], &voice->filterR);
}

inline void Synth_NoteOff(uint8_t ch, uint8_t note)
{
    for (int j = 0; j < chCfg[ch].noteCnt; j++)
    {
        if (chCfg[ch].noteStack[j] == note)
        {
            for (int k = j; k < NOTE_STACK_MAX - 1; k++)
            {
                chCfg[ch].noteStack[k] = chCfg[ch].noteStack[k + 1];
            }
            chCfg[ch].noteCnt = (chCfg[ch].noteCnt > 0) ? (chCfg[ch].noteCnt - 1) : 0;
            Status_ValueChangedIntArr("noteCnt-", chCfg[ch].noteCnt, ch);
        }
    }

    for (int i = 0; i < MAX_POLY_VOICE ; i++)
    {
        if ((voicePlayer[i].active) && (voicePlayer[i].midiNote == note) && (voicePlayer[i].midiCh == ch))
        {
            if ((voicePlayer[i].cfg->noteCnt > 0) && (voicePlayer[i].cfg->mono))
            {
                uint8_t midiNote = voicePlayer[i].cfg->noteStack[voicePlayer[i].cfg->noteCnt - 1];

                float diff = midiNote - voicePlayer[i].midiNote;

                voicePlayer[i].cfg->noteA = voicePlayer[i].cfg->port * ((float)voicePlayer[i].cfg->noteB) + (1.0f - voicePlayer[i].cfg->port) * voicePlayer[i].cfg->noteA;
                voicePlayer[i].cfg->port = 0.0f;

                voicePlayer[i].cfg->noteB += diff;
                voicePlayer[i].midiNote = midiNote;
            }
            else
            {
                voicePlayer[i].phase = release;
            }
        }
    }
}

void Synth_ModulationWheel(uint8_t ch, float value)
{
    chCfg[ch].modulationDepth = value;
}

void Synth_ModulationSpeed(uint8_t ch, float value)
{
    chCfg[ch].modulationSpeed = value * 10;
    //Status_ValueChangedFloat("ModulationSpeed", modulationSpeed);
}

void Synth_ModulationPitch(uint8_t ch, float value)
{
    chCfg[ch].modulationPitch = value * 5;
    //Status_ValueChangedFloat("ModulationDepth", modulationPitch);
}

void Synth_PitchBend(uint8_t ch, float bend)
{
    chCfg[ch].pitchBendValue = bend;
    //Serial.printf("pitchBendValue: %0.3f\n", chCfg[ch].pitchBendValue);
}

void Synth_PortTime(float value)
{
    float min = 0.02f; /* 1/(0.02 * 1000) -> 0.05s */
    float max = 0.0002f; /* 1/(0.0002 * 1000) -> 5s */

    curChCfg->portAdd = (pow(2.0f, value) - 1.0f) * (max - min) + min;
}

void Synth_SetCurCh(uint8_t ch, float value)
{
    if (value > 0)
    {
        if (ch < 16)
        {
            curChCfg = &chCfg[ch];
            Status_ValueChangedInt("Current ch", ch);
        }
    }
}

void Synth_ToggleMono(uint8_t ch, float value)
{
    if (value > 0)
    {
        curChCfg->mono = !curChCfg->mono;
        Status_LogMessage(curChCfg->mono ? "Mono" : "Poly");
    }
}

void Synth_SetParam(uint8_t slider, float value)
{
    switch (slider)
    {
    case SYNTH_PARAM_VEL_ENV_ATTACK:
        if (value == 0)
        {
            curChCfg->adsr_vol.a = 1.0f;
        }
        else
        {
            curChCfg->adsr_vol.a = (0.00005 * pow(5000, 1.0f - value));
        }
        Serial.printf("voice volume attack: %0.6f\n", curChCfg->adsr_vol.a);
        break;
    case SYNTH_PARAM_VEL_ENV_DECAY:
        curChCfg->adsr_vol.d = (0.00005 * pow(5000, 1.0f - value));
        Serial.printf("voice volume decay: %0.6f\n", curChCfg->adsr_vol.d);
        break;
    case SYNTH_PARAM_VEL_ENV_SUSTAIN:
        curChCfg->adsr_vol.s = (0.01 * pow(100, value));
        Serial.printf("voice volume sustain: %0.6f\n", curChCfg->adsr_vol.s);
        break;
    case SYNTH_PARAM_VEL_ENV_RELEASE:
        curChCfg->adsr_vol.r = (0.0001 * pow(100, 1.0f - value));
        Serial.printf("voice volume release: %0.6f\n", curChCfg->adsr_vol.r);
        break;

    case SYNTH_PARAM_FIL_ENV_ATTACK:
#if 1
        if (value == 0)
        {
            curChCfg->adsr_fil.a = 1.0f;
        }
        else
        {
            curChCfg->adsr_fil.a = (0.00005 * pow(5000, 1.0f - value));
        }
#else
        curChCfg->adsr_fil.a = value;
#endif
        Serial.printf("voice filter attack: %0.6f\n", curChCfg->adsr_fil.a);
        break;
    case SYNTH_PARAM_FIL_ENV_DECAY:
        curChCfg->adsr_fil.d = (0.00005 * pow(5000, 1.0f - value));
        Serial.printf("voice filter decay: %0.6f\n", curChCfg->adsr_fil.d);
        break;
    case SYNTH_PARAM_FIL_ENV_SUSTAIN:
        curChCfg->adsr_fil.s = value;
        Serial.printf("voice filter sustain: %0.6f\n", curChCfg->adsr_fil.s);
        break;
    case SYNTH_PARAM_FIL_ENV_RELEASE:
        curChCfg->adsr_fil.r = (0.0001 * pow(100, 1.0f - value));
        Serial.printf("voice filter release: %0.6f\n", curChCfg->adsr_fil.r);
        break;

#ifdef USE_UNISON
    case SYNTH_PARAM_DETUNE_1:
        curChCfg->detune = value;
        Serial.printf("detune: %0.3f cent\n", curChCfg->detune * 50);
        break;
    case SYNTH_PARAM_UNISON_2:
        curChCfg->unison = (uint8_t)(MAX_DETUNE * value);
        Serial.printf("unison: 1 + %d\n", curChCfg->unison);
        break;
#else
    case SYNTH_PARAM_WAVEFORM_1:
        {
            uint8_t selWaveForm = (value) * (WAVEFORM_TYPE_COUNT);
            curChCfg->selectedWaveForm = waveFormLookUp[selWaveForm];
            Serial.printf("selWaveForm: %d\n", selWaveForm);
        }
        break;
    case SYNTH_PARAM_WAVEFORM_2:
        {
            uint8_t selWaveForm = (value) * (WAVEFORM_TYPE_COUNT);
            curChCfg->selectedWaveForm2 = waveFormLookUp[selWaveForm];
            Serial.printf("selWaveForm2: %d\n", selWaveForm);
        }
        break;
#endif
    case SYNTH_PARAM_MAIN_FILT_CUTOFF:
        filtCutoff = value;
        Serial.printf("main filter cutoff: %0.3f\n", filtCutoff);
        Filter_Calculate(filtCutoff, filtReso, &filterGlobalC);
        break;
    case SYNTH_PARAM_MAIN_FILT_RESO:
        filtReso =  0.5f + 10 * value * value * value; /* min q is 0.5 here */
        Serial.printf("main filter reso: %0.3f\n", filtReso);
        Filter_Calculate(filtCutoff, filtReso, &filterGlobalC);
        break;

    case SYNTH_PARAM_VOICE_FILT_RESO:
        curChCfg->soundFiltReso = 0.5f + 10 * value * value * value; /* min q is 0.5 here */
        Serial.printf("voice filter reso: %0.3f\n", curChCfg->soundFiltReso);
        break;

    case SYNTH_PARAM_VOICE_NOISE_LEVEL:
        curChCfg->soundNoiseLevel = value;
        Serial.printf("voice noise level: %0.3f\n", curChCfg->soundNoiseLevel);
        break;

    case SYNTH_PARAM_VOICE_PORT_TIME:
        curChCfg->portAdd = value * value * value * value;
        Serial.printf("voice port time: %0.3f\n", curChCfg->portAdd);
        break;

    default:
        /* not connected */
        break;
    }
}

