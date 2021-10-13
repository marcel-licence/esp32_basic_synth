/*
 * this file contains the implementation of the terminal output
 * the output is vt100 compatible and you should use a terminal like teraTerm
 *
 * source: https://ttssh2.osdn.jp/index.html.en​
 *
 * Author: Marcel Licence
 */

bool triggerTerminalOutput = true; /*!< necessary for usage without vt100 compliant terminal */

char statusMsg[128] = ""; /*!< buffer for top line message */

uint32_t statusMsgShowTimer = 0; /*!< counter for timeout to reset top line */

#define VU_MAX  24 /*!< character length of vu meter */

float statusVuLookup[VU_MAX]; /*!< precalculated lookup */

/*
 * prepares the module
 */
void Status_Setup(void)
{
    /*
     * prepare lookup for log vu meters
     */
    float vuValue = 1.0f;
    for (int i = VU_MAX; i > 0; i--)
    {
        int n = i - 1;

        statusVuLookup[n] = vuValue;

        vuValue *= 1.0f / sqrt(2.0f); /* div by 1/sqrt(2) means 3db */
        vuValue *= 1.0f / sqrt(2.0f);
    }
}

/*
 * function which prints the vu meter "line"
 */
void Status_PrintVu(float *value, uint8_t vuMax)
{
    /*
     * first pick a color
     */
    if (*value >= 0.7071f) /* -3dB */
    {
        Serial.printf("\033[0;31m"); /* red */
    }
    else if (*value >= 0.5) /* -6dB */
    {
        Serial.printf("\033[0;33m"); /* yellow */
    }
    else
    {
        Serial.printf("\033[0;32m"); /* green */
    }

    for (int i = 0; i < vuMax; i++)
    {

        if (statusVuLookup[i + (VU_MAX - vuMax)] <= *value)
        {
            Serial.printf("#");
        }
        else
        {
            Serial.printf(" ");
        }
    }

    Serial.printf("\033[0m"); /* switch back to default */

    *value *= 0.5; /* slowly lower the value */
}


/*
 * refresh complete output
 * 32 character width
 * 14 character height
 */
void Status_PrintAll(void)
{
    char emptyLine[] = "                                ";
#if 0
    char emptyLineMin[] = "                               ";
    char emptyLineLong[] = "                                                                ";
#endif
    Serial.printf("\033[?25l");
    Serial.printf("\033[%d;%dH", 0, 0);
    //Serial.printf("--------------------------------\n");
    Serial.printf("%s%s\n", statusMsg, &emptyLine[strlen(statusMsg)]);
#ifdef VT100_ENABLED
    Serial.printf("--------------------------------\n");

    Serial.printf("inL    ");
    Status_PrintVu(vuInL, 8);

    Serial.printf("  outL   ");
    Status_PrintVu(vuOutL, 8);
    Serial.printf("\n");

    Serial.printf("inR    ");
    Status_PrintVu(vuInR, 8);
    Serial.printf("  outR   ");
    Status_PrintVu(vuOutR, 8);
    Serial.printf("\n");


    Serial.printf("--------------------------------\n");
    char memoryUsedMsg[64];
    sprintf(memoryUsedMsg, "%d of %d bytes used", sampleStorageInPos, sampleStorageLen);
    Serial.printf("%s%s\n", memoryUsedMsg, &emptyLine[strlen(memoryUsedMsg)]);

    float relativeStorageUsage = ((float)sampleStorageInPos) / ((float)sampleStorageLen);
    for (int i = 0; i < 32; i++)
    {
        if (i == (int)(relativeStorageUsage * 32.0f))
        {
            Serial.printf("O");
        }
        else
        {
            if (i > (int)(relativeStorageUsage * 32.0f))
            {
                Serial.printf("_");
            }
            else
            {
                Serial.printf("=");
            }
        }
    }
    Serial.println();

    Serial.printf("%s%s%s", emptyLine, currentFile, &emptyLineLong[strlen(currentFile)]);
#endif
}


void Status_Process_Sample(uint32_t inc)
{
    statusMsgShowTimer += inc;
    if (statusMsgShowTimer == SAMPLE_RATE * 3)
    {
        statusMsgShowTimer = SAMPLE_RATE * 3 + 1;
        statusMsg[0] = 0;
#if 1
        triggerTerminalOutput = true;
#else
#ifndef VT100_ENABLED
        Status_PrintAll();
#endif
#endif
    }
}

void Status_Process(void)
{
#ifdef DISPLAY_FROM_STATUS_ENABLED
    if (triggerTerminalOutput)
    {
        if (statusMsg[0] == 0)
        {
            Display_ShowClear();
        }
        Display_DisplayMessage();
    }
#endif

#ifndef VT100_ENABLED
    if (triggerTerminalOutput)
#endif
    {
        Status_PrintAll();
        triggerTerminalOutput = false;
    }

#ifndef VT100_ENABLED
    if (triggerTerminalOutput)
#endif
    {
        Status_PrintAll();
        triggerTerminalOutput = false;
    }
}

/*
 * update top line message including a float value
 */
void Status_ValueChangedFloat(const char *descr, float value)
{
    statusMsgShowTimer = 0;
    sprintf(statusMsg, "%s: %0.3f", descr, value);
    triggerTerminalOutput = true;
#ifdef DISPLAY_FROM_STATUS_ENABLED
    Display_ShowValueFloat(descr, value);
#endif
}

/*
 * update top line message including a float value
 */
void Status_ValueChangedFloatArr(const char *descr, float value, int index)
{
    statusMsgShowTimer = 0;
    sprintf(statusMsg, "%s[%d]: %0.3f", descr, index, value);
    triggerTerminalOutput = true;
#ifdef DISPLAY_FROM_STATUS_ENABLED
    Display_ShowValueFloatArr(descr, value, index);
#endif
}

void Status_ValueChangedIntArr(const char *descr, int value, int index)
{
    statusMsgShowTimer = 0;
    sprintf(statusMsg, "%s[%d]: %d", descr, index, value);
    triggerTerminalOutput = true;
#ifdef DISPLAY_FROM_STATUS_ENABLED
    Display_ShowValueIntArr(descr, value, index);
#endif
}
/*
 * update top line message including an integer value
 */
void Status_ValueChangedInt(const char *descr, int value)
{
    statusMsgShowTimer = 0;
    sprintf(statusMsg, "%s: %d", descr, value);
    triggerTerminalOutput = true;
#ifdef DISPLAY_FROM_STATUS_ENABLED
    Display_ShowValueInt(descr, value);
#endif
}

/*
 * update top line message
 */
void Status_TestMsg(const char *text)
{
    statusMsgShowTimer = 0;
    sprintf(statusMsg, "%s", text);
    triggerTerminalOutput = true;
}

void Status_LogMessage(const char *text)
{
    statusMsgShowTimer = 0;
    sprintf(statusMsg, "%s", text);
    triggerTerminalOutput = true;
}

