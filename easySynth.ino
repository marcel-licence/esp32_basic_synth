/*
 * Implementation of a simple polyphonic synthesizer module
 * - it supports different waveforms
 * - it supports polyphony
 * - implemented ADSR for velocity and filter
 * - allows usage of multiple oscillators per voice
 *
 * Author: Marcel Licence
 */

/*
 * activate the following macro to enable unison mode
 * by default the saw wave form will be used
 * the waveform controllers are remapped to
 * - waveform1 -> detune
 * - waveform2 -> oscillator count
 */
//#define USE_UNISON


/*
 * Following defines can be changed for different puprposes
 */
#ifdef USE_UNISON
/* use another setting, because unison supports more than 2 osc per voice */
#define MAX_DETUNE		12 /* 1 + 11 additional tones */
#define MAX_POLY_OSC	36 /* osc polyphony, always active reduces single voices max poly */
#define MAX_POLY_VOICE	3  /* max single voices, can use multiple osc */
#else
#define MAX_POLY_OSC	24 /* osc polyphony, always active reduces single voices max poly */
#define MAX_POLY_VOICE	12 /* max single voices, can use multiple osc */
#endif


/*
 * this is just a kind of magic to go through the waveforms
 * - WAVEFORM_BIT sets the bit length of the pre calculated waveforms
 */
#define WAVEFORM_BIT	10UL
#define WAVEFORM_CNT	(1<<WAVEFORM_BIT)
#define WAVEFORM_Q4		(1<<(WAVEFORM_BIT-2))
#define WAVEFORM_MSK	((1<<WAVEFORM_BIT)-1)
#define WAVEFORM_I(i)	((i) >> (32 - WAVEFORM_BIT)) & WAVEFORM_MSK


#define MIDI_NOTE_CNT 128
uint32_t midi_note_to_add[MIDI_NOTE_CNT]; /* lookup to playback waveforms with correct frequency */

#ifdef USE_UNISON
uint32_t midi_note_to_add50c[MIDI_NOTE_CNT]; /* lookup for detuning */
#endif

/*
 * set the correct count of available waveforms
 */
#define WAVEFORM_TYPE_COUNT	7

/*
 * add here your waveforms
 */
float *sine = NULL;
float *saw = NULL;
float *square = NULL;
float *pulse = NULL;
float *tri = NULL;
float *noise = NULL;
float *silence = NULL;

/*
 * do not forget to enter the waveform pointer addresses here
 */
float **waveFormLookUp[WAVEFORM_TYPE_COUNT] = {&sine, &saw, &square, &pulse, &tri, &noise, &silence};

/*
 * pre selected waveforms
 */


#ifdef USE_UNISON
static float detune = 0.1; /* detune parameter */
static uint8_t unison = 0; /* additional osc per voice count */
float **selectedWaveForm =  &saw;
float **selectedWaveForm2 =  &saw;
#else
float **selectedWaveForm =  &pulse;
float **selectedWaveForm2 =  &silence;
#endif



struct adsrT
{
    float a;
    float d;
    float s;
    float r;
};

struct adsrT adsr_vol = {0.25f, 0.25f, 1.0f, 0.01f};
struct adsrT adsr_fil = {1.0f, 0.25f, 1.0f, 0.01f};

typedef enum
{
    attack, decay, sustain, release
} adsr_phaseT;

/* this prototype is required .. others not -  i still do not know what magic arduino is doing */
inline bool ADSR_Process(const struct adsrT *ctrl, float *ctrlSig, adsr_phaseT *phase);

struct filterCoeffT
{
    float aNorm[2] = {0.0f, 0.0f};
    float bNorm[3] = {1.0f, 0.0f, 0.0f};
};

struct filterProcT
{
    struct filterCoeffT *filterCoeff;
    float w[3];
};

struct filterCoeffT filterGlobalC;
struct filterProcT mainFilterL, mainFilterR;


struct oscillatorT
{
    float *waveForm;
    float *dest;
    uint32_t samplePos;
    uint32_t addVal;
    float pan_l;
    float pan_r;
};

float voiceSink[2];
struct oscillatorT oscPlayer[MAX_POLY_OSC];

uint32_t osc_act = 0;

struct notePlayerT
{
    float lastSample[2];

    float velocity;
    bool active;
    adsr_phaseT phase;
    uint8_t midiNote;

    float control_sign;
    float out_level;

    struct filterCoeffT filterC;
    struct filterProcT filterL;
    struct filterProcT filterR;
    float f_control_sign;
    float f_control_sign_slow;
    adsr_phaseT f_phase;
};



struct notePlayerT voicePlayer[MAX_POLY_VOICE];

uint32_t voc_act = 0;




void Synth_Init()
{
    randomSeed(34547379);

    /*
     * we do not check if malloc was successful
     * if there is not enough memory left the application will crash
     */
    sine = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
    saw = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
    square = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
    pulse = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
    tri = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
    noise = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
    silence = (float *)malloc(sizeof(float) * WAVEFORM_CNT);

    Delay_Init();

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
        pulse[i] = (i > (WAVEFORM_CNT / 4)) ? 1 : -1;
        tri[i] = ((i > (WAVEFORM_CNT / 2)) ? (((4.0f * (float)i) / ((float)WAVEFORM_CNT)) - 1.0f) : (3.0f - ((4.0f * (float)i) / ((float)WAVEFORM_CNT)))) - 2.0f;
        noise[i] = (random(1024) / 512.0f) - 1.0f;
        silence[i] = 0;
    }

    /*
     * initialize all oscillators
     */
    for (int i = 0; i < MAX_POLY_OSC; i++)
    {
        oscillatorT *osc = &oscPlayer[i];
        osc->waveForm = silence;
        osc->dest = voiceSink;
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
}

struct filterCoeffT mainFilt;

/*
 * filter calculator:
 * https://www.earlevel.com/main/2013/10/13/biquad-calculator-v2/
 *
 * some filter implementations:
 * https://github.com/ddiakopoulos/MoogLadders/blob/master/src/Filters.h
 *
 * some more information about biquads:
 * https://www.earlevel.com/main/2003/02/28/biquads/
 */

static float filtCutoff = 1.0f;
static float filtReso = 0.5f;
static float soundFiltReso = 0.5f;

/*
 * calculate coefficients of the 2nd order IIR filter
 */
inline void Filter_Calculate(float c, float reso, struct filterCoeffT *const  filterC)
{
    float *aNorm = filterC->aNorm;
    float *bNorm = filterC->bNorm;

    float Q = reso;
    float  cosOmega, omega, sinOmega, alpha, a[3], b[3];

    /*
     * change curve of cutoff a bit
     * maybe also log or exp function could be used
     */
    c = c * c * c;

    if (c >= 1.0f)
    {
        omega = 1.0f;
    }
    else if (c < 0.0025f)
    {
        omega = 0.0025f;
    }
    else
    {
        omega = c;
    }

    /*
     * use lookup here to get quicker results
     */
    cosOmega = sine[WAVEFORM_I((uint32_t)((float)((1ULL << 31) - 1) * omega + (float)((1ULL << 30) - 1)))];
    sinOmega = sine[WAVEFORM_I((uint32_t)((float)((1ULL << 31) - 1) * omega))];

    alpha = sinOmega / (2.0 * Q);
    b[0] = (1 - cosOmega) / 2;
    b[1] = 1 - cosOmega;
    b[2] = b[0];
    a[0] = 1 + alpha;
    a[1] = -2 * cosOmega;
    a[2] = 1 - alpha;

    // Normalize filter coefficients
    float factor = 1.0f / a[0];

    aNorm[0] = a[1] * factor;
    aNorm[1] = a[2] * factor;

    bNorm[0] = b[0] * factor;
    bNorm[1] = b[1] * factor;
    bNorm[2] = b[2] * factor;
}

inline void Filter_Process(float *const signal, struct filterProcT *const filterP)
{
    const float out = filterP->filterCoeff->bNorm[0] * (*signal) + filterP->w[0];
    filterP->w[0] = filterP->filterCoeff->bNorm[1] * (*signal) - filterP->filterCoeff->aNorm[0] * out + filterP->w[1];
    filterP->w[1] = filterP->filterCoeff->bNorm[2] * (*signal) - filterP->filterCoeff->aNorm[1] * out;
    *signal = out;
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

static float out_l, out_r;
static uint32_t count = 0;

//[[gnu::noinline, gnu::optimize ("fast-math")]]
inline void Synth_Process(float *left, float *right)
{
#ifdef USE_UNISON
    /* we need random for detuning to avoid producing bad sounds */
    randomSeed(34547379);
#endif

    /*
     * generator simulation, rotate all wheels
     */
    out_l = 0;
    out_r = 0;

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
            osc->samplePos += osc->addVal;
            float sig = osc->waveForm[WAVEFORM_I(osc->samplePos)];
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
            if (count % 4 == 0)
            {
                voice->active = ADSR_Process(&adsr_vol, &voice->control_sign, &voice->phase);
                if (voice->active == false)
                {
                    Voice_Off(i);
                }
                /*
                 * make is slow to avoid bad things .. or crying ears
                 */
                (void)ADSR_Process(&adsr_fil, &voice->f_control_sign, &voice->f_phase);
            }

            voice->lastSample[0] *= voice->control_sign * voice->velocity;
            voice->lastSample[1] *= voice->control_sign * voice->velocity;

            if (count % 32 == 0)
            {
                voice->f_control_sign_slow = 0.05 * voice->f_control_sign + 0.95 * voice->f_control_sign_slow;
                Filter_Calculate(voice->f_control_sign_slow, soundFiltReso, &voice->filterC);
            }

            Filter_Process(&voice->lastSample[0], &voice->filterL);
            Filter_Process(&voice->lastSample[1], &voice->filterR);

            out_l += voice->lastSample[0];
            out_r += voice->lastSample[1];
            voice->lastSample[0] = 0.0f;
            voice->lastSample[1] = 0.0f;
        }
    }

    /*
     * process main filter
     */
    Filter_Process(&out_l, &mainFilterL);
    Filter_Process(&out_r, &mainFilterR);

    /*
     * process delay line
     */
    Delay_Process(&out_l, &out_r);

    /*
     * reduce level a bit to avoid distortion
     */
    out_l *= 0.4f;
    out_r *= 0.4f;

    /*
     * finally output our samples
     */
    *left = out_l;
    *right = out_r;
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

struct notePlayerT *getFreeVoice()
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

inline void Filter_Reset(struct filterProcT *filter)
{
    filter->w[0] = 0.0f;
    filter->w[1] = 0.0f;
    filter->w[2] = 0.0f;
}

inline void Synth_NoteOn(uint8_t note)
{
    struct notePlayerT *voice = getFreeVoice();
    struct oscillatorT *osc = getFreeOsc();

    /*
     * No free voice found, return otherwise crash xD
     */
    if ((voice == NULL) || (osc == NULL))
    {
        //Serial.printf("voc: %d, osc: %d\n", voc_act, osc_act);
        return ;
    }

    voice->midiNote = note;
    voice->velocity = 0.25; /* just something to test */
    voice->lastSample[0] = 0.0f;
    voice->lastSample[1] = 0.0f;
    voice->control_sign = 0.0f;

#if 0
    voice->f_phase = attack;
#else
    if (adsr_fil.a < adsr_fil.s)
    {
        adsr_fil.a = adsr_fil.s;
    }
    voice->f_phase = decay;
#endif
    voice->f_control_sign = adsr_fil.a;
    voice->f_control_sign_slow = adsr_fil.a;
    voice->active = true;
    voice->phase = attack;

    voc_act += 1;

    /*
     * add oscillator
     */
#ifdef USE_UNISON
    if (unison > 0 )
    {
        /*
         * shift first oscillator down
         */
        osc->addVal = midi_note_to_add[note] + ((0 - (unison * 0.5)) * midi_note_to_add50c[note] * detune / unison);
    }
    else
#endif
    {
        osc->addVal = midi_note_to_add[note];
    }
    osc->samplePos = 0;
    osc->waveForm = *selectedWaveForm;
    osc->dest = voice->lastSample;
    osc->pan_l = 1;
    osc->pan_r = 1;

    osc_act += 1;


#ifdef USE_UNISON

    int8_t pan = 1;

    /*
     * attach more oscillators to voice
     */
    for (int i = 0; i < unison; i++)
    {
        osc = getFreeOsc();
        if (osc == NULL)
        {
            //Serial.printf("voc: %d, osc: %d\n", voc_act, osc_act);
            return ;
        }

        osc->addVal = midi_note_to_add[note] + ((i + 1 - (unison * 0.5)) * midi_note_to_add50c[note] * detune / unison);
        osc->samplePos = (uint32_t)random(1 << 31); /* otherwise it sounds ... bad!? */
        osc->waveForm = *selectedWaveForm2;
        osc->dest = voice->lastSample;

        /*
         * put last osc in the middle
         */
        if ((unison - 1) == i)
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
            osc->waveForm = *selectedWaveForm2;
            osc->dest = voice->lastSample;
            osc->pan_l = 1;
            osc->pan_r = 1;

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

inline void Synth_NoteOff(uint8_t note)
{
    for (int i = 0; i < MAX_POLY_VOICE ; i++)
    {
        if ((voicePlayer[i].active) && (voicePlayer[i].midiNote == note))
        {
            voicePlayer[i].phase = release;
        }
    }
}

void Synth_SetRotary(uint8_t rotary, float value)
{
    switch (rotary)
    {
#ifdef USE_UNISON
    case 0:
        detune = value;
        Serial.printf("detune: %0.3f cent\n", detune * 50);
        break;
    case 1:
        unison = (uint8_t)(MAX_DETUNE * value);
        Serial.printf("unison: 1 + %d\n", unison);
        break;
#else
    case 0:
        {
            uint8_t selWaveForm = (value) * (WAVEFORM_TYPE_COUNT);
            selectedWaveForm = waveFormLookUp[selWaveForm];
            Serial.printf("selWaveForm: %d\n", selWaveForm);
        }
        break;
    case 1:
        {
            uint8_t selWaveForm = (value) * (WAVEFORM_TYPE_COUNT);
            selectedWaveForm2 = waveFormLookUp[selWaveForm];
            Serial.printf("selWaveForm2: %d\n", selWaveForm);
        }
#endif
        break;
    case 2:
        Delay_SetLength(value);
        break;
    case 3:
        Delay_SetLevel(value);
        break;
    case 4:
        Delay_SetFeedback(value);
        break;

    case 5:
        filtCutoff = value;
        Serial.printf("main filter cutoff: %0.3f\n", filtCutoff);
        Filter_Calculate(filtCutoff, filtReso, &filterGlobalC);
        break;
    case 6:
        filtReso =  0.5f + 10 * value * value * value; /* min q is 0.5 here */
        Serial.printf("main filter reso: %0.3f\n", filtReso);
        Filter_Calculate(filtCutoff, filtReso, &filterGlobalC);
        break;

    case 7:
        soundFiltReso = 0.5f + 10 * value * value * value; /* min q is 0.5 here */
        Serial.printf("voice filter reso: %0.3f\n", soundFiltReso);
        break;

    default:
        break;
    }
}

void Synth_SetSlider(uint8_t slider, float value)
{
    switch (slider)
    {
    case 0:
        adsr_vol.a = (0.00005 * pow(5000, 1.0f - value));
        Serial.printf("voice volume attack: %0.6f\n", adsr_vol.a);
        break;
    case 1:
        adsr_vol.d = (0.00005 * pow(5000, 1.0f - value));
        Serial.printf("voice volume decay: %0.6f\n", adsr_vol.d);
        break;
    case 2:
        adsr_vol.s = (0.01 * pow(100, value));
        Serial.printf("voice volume sustain: %0.6f\n", adsr_vol.s);
        break;
    case 3:
        adsr_vol.r = (0.0001 * pow(100, 1.0f - value));
        Serial.printf("voice volume release: %0.6f\n", adsr_vol.r);
        break;

    case 4:
#if 0
        adsr_fil.a = (0.00005 * pow(5000, 1.0f - value));
#else
        adsr_fil.a = value;
#endif
        Serial.printf("voice filter attack: %0.6f\n", adsr_fil.a);
        break;
    case 5:
        adsr_fil.d = (0.00005 * pow(5000, 1.0f - value));
        Serial.printf("voice filter decay: %0.6f\n", adsr_fil.d);
        break;
    case 6:
        adsr_fil.s = value;
        Serial.printf("voice filter sustain: %0.6f\n", adsr_fil.s);
        break;
    case 7:
        adsr_fil.r = (0.0001 * pow(100, 1.0f - value));
        Serial.printf("voice filter release: %0.6f\n", adsr_fil.r);
        break;

    default:
        /* not connected */
        break;
    }
}

