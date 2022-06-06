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
 * @file midi_stream_player.ino
 * @author Marcel Licence
 * @date 22.05.2022
 *
 * @brief This file contains code to access the midi file stream player
 *
 * The midi file stream player will be part of the ML_SynthTools library
 * This file is required to define all file accessing functions which are not part of the library
 * In addition to that they are dependend to the different platform you are using
 */


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


#ifdef MIDI_STREAM_PLAYER_ENABLED


//#define MIDI_STREAM_PLAYER_DATA_DUMP /*!< optional to dump event data from midi file */


#ifdef ARDUINO_DAISY_SEED
#include <STM32SD.h>

extern Sd2Card card;
extern SdFatFs fatFs;

#define SD_MMC  SD

#define FST SDClass
#else
#define FST fs::FS
#endif

#define MIDI_FS_LITTLE_FS   0
#define MIDI_FS_SD_MMC  1


#define FORMAT_LITTLEFS_IF_FAILED true


#include <FS.h>
#ifdef ARDUINO_RUNNING_CORE /* tested with arduino esp32 core version 2.0.2 */
#include <LittleFS.h> /* Using library LittleFS at version 2.0.0 from https://github.com/espressif/arduino-esp32 */
#else
#include <LITTLEFS.h> /* Using library LittleFS_esp32 at version 1.0.6 from https://github.com/lorol/LITTLEFS */
#define LittleFS LITTLEFS
#endif
#include <SD_MMC.h>

#include <ml_midi_file_stream.h>


#define MIDI_STREAM_PLAYER_CTRL_PAUSE 0
#define MIDI_STREAM_PLAYER_CTRL_STOP    1
#define MIDI_STREAM_PLAYER_CTRL_PLAY 2
#define MIDI_STREAM_PLAYER_CTRL_SKIP 3
#define MIDI_STREAM_PLAYER_CTRL_START 4


static uint64_t tickCnt = 0;
bool midiAutoLoop = false;


static void listDir(FST &fs, const char *dirname, uint8_t levels);


struct file_access_f mdiCallbacks =
{
    MIDI_open,
    MIDI_read,
    MIDI_write,
    MIDI_close,
    0,
    MIDI_getc,
    MIDI_putc,
    MIDI_tell,
    MIDI_seek,
};

fs::File midiFile;

uint8_t MIDI_open(const char *path, const char *mode)
{
    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED))
    {
        Serial.println("LITTLEFS Mount Failed");
        return 0;
    }
    midiFile = LittleFS.open(path);
    if (!midiFile)
    {
        Serial.println("- failed to open file");
        return 0;
    }
    else
    {
        Serial.printf("File opened: %s\n", path);
    }

    return 1;
}

int MIDI_read(void *buf, uint8_t unused, size_t size, struct file_access_f *ff)
{
    File *file = &midiFile;//ff->file;
    for (int i = 0; i < size; i++)
    {
        ((uint8_t *)buf)[i] = file->read();
        ff->file ++;
    }
    return size;
}

int MIDI_write(void *buf, uint8_t unused, size_t size, struct file_access_f *ff)
{
    return 0;
}

void MIDI_close(struct file_access_f *ff)
{
    File *file = &midiFile;//ff->file;
    file->close();
}

char MIDI_getc(struct file_access_f *ff)
{
    File file = midiFile;//ff->file;
    return file.read();
}

char MIDI_putc(char c, struct file_access_f *ff)
{
    return 0;
}

int MIDI_tell(struct file_access_f *ff)
{
#if 0
    File *file = &midiFile;//ff->file;
    return file->size();
#else
    return ff->file - 1;
#endif
}

char MIDI_seek(struct file_access_f *ff, int pos, uint8_t mode)
{
    File *file = &midiFile;//ff->file;
    if (mode == SEEK_SET)
    {
        file->seek(pos, SeekSet);
        ff->file = pos + 1;
    }
    else
    {
        file->seek(pos, SeekCur);
        ff->file += pos;
    }
    //ff->file = file->size() - pos;
    return 0;
}

void MidiStreamPlayer_Init()
{
    MidiStreamPlayer_ListFiles(MIDI_FS_LITTLE_FS);
    MidiStreamPlayer_ListFiles(MIDI_FS_SD_MMC);
}

void MidiStreamPlayer_PlayFile(char *midi_filename)
{
    MidiStreamPlayer_PlayMidiFile_fromLittleFS(midi_filename, 0);
}

static void listDir(FST &fs, const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                listDir(fs, file.name(), levels - 1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

static bool midiPlaying = false;

#ifdef MIDI_STREAM_PLAYER_DATA_DUMP
void MidiDataCallback(uint8_t *data, uint8_t data_len)
{
    printf("d:");
    for (uint8_t n = 0; n < data_len; n++)
    {
        printf(" %02x", data[n]);
    }
    printf("\n");
}
#endif

uint64_t duration = 0;
struct midi_proc_s midiStreamPlayerHandle;

void MidiStreamPlayer_NoteOn(uint8_t ch, uint8_t note, uint8_t vel)
{
    Midi_NoteOn(ch, note, vel);
}

void MidiStreamPlayer_NoteOff(uint8_t ch, uint8_t note, uint8_t vel)
{
    Midi_NoteOff(ch, note);
}

void MidiStreamPlayer_ControlChange(uint8_t ch, uint8_t number, uint8_t value)
{
    Midi_ControlChange(ch, number, value);
}

void MidiStreamPlayer_PlayMidiFile_fromLittleFS(char *filename, uint8_t trackToPlay)
{
    Serial.printf("Try to open %s from LittleFS\n", filename);

    memset(&midiStreamPlayerHandle, 0, sizeof(midiStreamPlayerHandle));

#ifdef MIDI_STREAM_PLAYER_DATA_DUMP
    midiStreamPlayerHandle.raw = MidiDataCallback;
#endif
    midiStreamPlayerHandle.noteOn = MidiStreamPlayer_NoteOn;
    midiStreamPlayerHandle.noteOff = MidiStreamPlayer_NoteOff;
    midiStreamPlayerHandle.controlChange = MidiStreamPlayer_ControlChange;
    midiStreamPlayerHandle.ff = &mdiCallbacks;

    midiStreamPlayerHandle.midi_tempo = (60000000.0 / 100.0);

    midi_file_stream_load(filename, &midiStreamPlayerHandle);

    printf("number_of_tracks: %d\n", midiStreamPlayerHandle.number_of_tracks);
    printf("file_format: %d\n", midiStreamPlayerHandle.file_format);
    printf("division_type_and_resolution: %d\n", interpret_uint16(midiStreamPlayerHandle.division_type_and_resolution));

    if (midiStreamPlayerHandle.file_format == 1)
    {
        MidiStreamParseTrack(&midiStreamPlayerHandle);

        float temp_f = (60000000.0 / midiStreamPlayerHandle.midi_tempo);
        printf("midi_tempo: %d\n", midiStreamPlayerHandle.midi_tempo);
        printf("tempo: %0.3f\n", temp_f);

        for (uint8_t n = 1; n < trackToPlay; n++)
        {
            MidiStreamSkipTrack(&midiStreamPlayerHandle);
        }
    }

    MidiStreamReadTrackPrepare(&midiStreamPlayerHandle);

    duration = 0;
    long shortDuration;
    midiPlaying = MidiStreamReadSingleEventTime(&midiStreamPlayerHandle, &shortDuration);
    if (midiPlaying)
    {
        Serial.printf("Started midi file playback\n");
    }
    else
    {
        Serial.printf("Couldn't start midi file playback\n");
    }

    duration = shortDuration;
    duration *= SAMPLE_RATE;
    duration *= midiStreamPlayerHandle.midi_tempo;
}

#ifdef MIDI_FMT_INT
void MidiStreamPlayerCtrl(uint8_t setting, uint8_t value)
#else
void MidiStreamPlayerCtrl(uint8_t setting, float value)
#endif
{
    if (value > 0)
    {
        switch (setting)
        {
        case MIDI_STREAM_PLAYER_CTRL_PAUSE:
            MidiStreamPlayer_PausePlayback();
            break;
        case MIDI_STREAM_PLAYER_CTRL_STOP:
            MidiStreamPlayer_StopPlayback();
            break;
        case MIDI_STREAM_PLAYER_CTRL_PLAY:
            {
                char midiFile[] = "/song.mid";
                MidiStreamPlayer_PlayMidiFile_fromLittleFS(midiFile, 1);
                tickCnt = 0;
            }
            break;
        case MIDI_STREAM_PLAYER_CTRL_SKIP:
            tickCnt += 100000;
            break;
        case MIDI_STREAM_PLAYER_CTRL_START:
            MidiStreamPlayer_StartPlayback();
            break;
        }
    }
}

#ifdef MIDI_FMT_INT
void MidiStreamPlayerTempo(uint8_t unused __attribute__((unused)), uint8_t value)
#else
void MidiStreamPlayerTempo(uint8_t unused __attribute__((unused)), float value)
#endif
{
    float tempo_f = value;
#ifdef MIDI_FMT_INT
    tempo_f /= 127;
#endif
    tempo_f = 60.0f + (tempo_f * 180.0f);
    tempo_f = (60000000.0 / tempo_f);
    midiStreamPlayerHandle.midi_tempo = tempo_f;
}

void MidiStreamPlayer_PausePlayback(void)
{
    midiPlaying = !midiPlaying;
}

void MidiStreamPlayer_StopPlayback(void)
{
    midiPlaying = false;
    for (uint8_t n = 0; n < 16; n++)
    {
        for (uint32_t i = 0; i < 128; i++)
        {
            Midi_NoteOff(n, i);
        }
    }
}

void MidiStreamPlayer_StartPlayback(void)
{
    MidiStreamPlayer_StopPlayback();

    MidiStreamRewind(&midiStreamPlayerHandle);

    tickCnt = 0;

    long shortDuration;
    midiPlaying = MidiStreamReadSingleEventTime(&midiStreamPlayerHandle, &shortDuration);
    duration = shortDuration;
    duration *= SAMPLE_RATE;
    duration *= midiStreamPlayerHandle.midi_tempo;
}

void MidiStreamPlayer_Tick(uint32_t ticks)
{
    if (midiPlaying == false)
    {
        if (midiAutoLoop)
        {
            /*
             * this will cause an audible noise for a short moment
             * seeking within files is very slow
             *
             * a new method is required to avoid this problem for better looping
             */
            MidiStreamRewind(&midiStreamPlayerHandle);
            long shortDuration;
            midiPlaying = MidiStreamReadSingleEventTime(&midiStreamPlayerHandle, &shortDuration);
            duration = shortDuration;
            duration *= SAMPLE_RATE;
            duration *= midiStreamPlayerHandle.midi_tempo;
        }
    }

    if (midiPlaying)
    {
        uint64_t longTick = ticks;
        longTick *= (uint64_t)interpret_uint16(midiStreamPlayerHandle.division_type_and_resolution);
        longTick *= 1000000;
        tickCnt += longTick;

        while ((tickCnt > duration) && midiPlaying)
        {
            //printf("%lld\n", tickCnt);
            tickCnt -= duration;

            midiPlaying &= MidiStreamReadSingleEvent(&midiStreamPlayerHandle);

            long shortDuration;
            midiPlaying &= MidiStreamReadSingleEventTime(&midiStreamPlayerHandle, &shortDuration);
            duration = shortDuration;
            duration *= SAMPLE_RATE;
            duration *= midiStreamPlayerHandle.midi_tempo;
            //Serial.printf("duration: %ld\n", shortDuration);
        }
    }
}

void MidiStreamPlayer_ListFiles(uint8_t filesystem)
{
    switch (filesystem)
    {
    case MIDI_FS_LITTLE_FS:
        {
            if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED))
            {
                Serial.println("LittleFS Mount Failed");
                return;
            }
            listDir(LittleFS, "/", 3);
            break;
        }
    case MIDI_FS_SD_MMC:
        {
            if (!SD_MMC.begin())
            {
                Serial.println("Card Mount Failed");
                return;
            }
            uint8_t cardType = SD_MMC.cardType();

            if (cardType == CARD_NONE)
            {
                Serial.println("No SD_MMC card attached");
                return;
            }

            Serial.print("SD_MMC Card Type: ");
            if (cardType == CARD_MMC)
            {
                Serial.println("MMC");
            }
            else if (cardType == CARD_SD)
            {
                Serial.println("SDSC");
            }
            else if (cardType == CARD_SDHC)
            {
                Serial.println("SDHC");
            }
            else
            {
                Serial.println("UNKNOWN");
            }

            uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
            Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);

            listDir(SD_MMC, "/", 0);
        }
        break;
    }
}

#endif

