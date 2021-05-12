/*
 * This is a simple implementation of a delay line
 * - level adjustable
 * - feedback
 * - length adjustable
 *
 * Author: Marcel Licence
 */


/* max delay can be changed but changes also the memory consumption */
#define MAX_DELAY	11100

/*
 * module variables
 */
float *delayLine_l;
float *delayLine_r;
float delayToMix = 0;
float delayFeedback = 0;
uint32_t delayLen = 11098;
uint32_t delayIn = 0;
uint32_t delayOut = 0;

void Delay_Init(void)
{
    delayLine_l = (float *)malloc(sizeof(float) * MAX_DELAY);
    if (delayLine_l == NULL)
    {
        Serial.printf("No more heap memory!\n");
    }
    delayLine_r = (float *)malloc(sizeof(float) * MAX_DELAY);
    if (delayLine_r == NULL)
    {
        Serial.printf("No more heap memory!\n");
    }
    Delay_Reset();
}

void Delay_Reset(void)
{
    for (int i = 0; i < MAX_DELAY; i++)
    {
        delayLine_l[i] = 0;
        delayLine_r[i] = 0;
    }
}

void Delay_Process(float *signal_l, float *signal_r)
{
    delayLine_l[delayIn] = *signal_l;
    delayLine_r[delayIn] = *signal_r;

    delayOut = delayIn + (1 + MAX_DELAY - delayLen);

    if (delayOut >= MAX_DELAY)
    {
        delayOut -= MAX_DELAY;
    }

    *signal_l += delayLine_l[delayOut] * delayToMix;
    *signal_r += delayLine_r[delayOut] * delayToMix;

    delayLine_l[delayIn] += delayLine_l[delayOut] * delayFeedback;
    delayLine_r[delayIn] += delayLine_r[delayOut] * delayFeedback;

    delayIn ++;

    if (delayIn >= MAX_DELAY)
    {
        delayIn = 0;
    }
}

void Delay_SetFeedback(float value)
{
    delayFeedback = value;
    Serial.printf("delay feedback: %0.3f\n", value);
}

void Delay_SetLevel(float value)
{
    delayToMix = value;
    Serial.printf("delay level: %0.3f\n", value);
}

void Delay_SetLength(float value)
{
    delayLen = (uint32_t)(((float)MAX_DELAY - 1.0f) * value);
    Serial.printf("delay length: %0.3fms\n", delayLen * (1000.0f / ((float)SAMPLE_RATE)));
}
