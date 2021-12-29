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
 * @file midi_ctrl.ino
 * @author Marcel Licence
 * @date 19.11.2021
 *
 * @brief Little midi control helper (provides a virtual split point)
 *
 * @see https://youtu.be/o-XjbrZHfWA
 */


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


#ifdef MIDI_CTRL_ENABLED


struct
{
    uint8_t split;
    int transpose[2];
} midiCtrlParam =
{
    60,
    {-12, -12},
};

void MidiCtrl_NoteOn(uint8_t ch, uint8_t note, float vel)
{
    if (note < midiCtrlParam.split)
    {
        if ((note + midiCtrlParam.transpose[0] > 0) && (note + midiCtrlParam.transpose[0] < 128))
        {
            Arp_NoteOn(0, note + midiCtrlParam.transpose[0], vel);

        }
#if 0
        Status_ValueChangedInt("lower", note);
#endif
    }
    else
    {
        if ((note + midiCtrlParam.transpose[1] > 0) && (note + midiCtrlParam.transpose[1] < 128))
        {
            Arp_NoteOn(1, note + midiCtrlParam.transpose[1], vel);
        }
#if 0
        Status_ValueChangedInt("upper", note);
#endif
    }
}

void MidiCtrl_NoteOff(uint8_t ch, uint8_t note)
{
    if (note < midiCtrlParam.split)
    {
        if ((note + midiCtrlParam.transpose[0] > 0) && (note + midiCtrlParam.transpose[0] < 128))
        {
            Arp_NoteOff(0, note + midiCtrlParam.transpose[0]);
        }
    }
    else
    {
        if ((note + midiCtrlParam.transpose[1] > 0) && (note + midiCtrlParam.transpose[1] < 128))
        {
            Arp_NoteOff(1, note + midiCtrlParam.transpose[1]);
        }
    }
}

void MidiCtrl_TransposeUp(uint8_t ch, float value)
{
    if (value > 0)
    {
        midiCtrlParam.transpose[ch] ++;
        Status_ValueChangedIntArr("transpose", midiCtrlParam.transpose[ch], ch);
    }
}

void MidiCtrl_TransposeDown(uint8_t ch, float value)
{
    if (value > 0)
    {
        midiCtrlParam.transpose[ch] --;
        Status_ValueChangedIntArr("transpose", midiCtrlParam.transpose[ch], ch);
    }
}

void MidiCtrl_TransposeReset(uint8_t ch, float value)
{
    if (value > 0)
    {
        midiCtrlParam.transpose[ch] = 0;
        Status_ValueChangedIntArr("transpose", midiCtrlParam.transpose[ch], ch);
    }
}

#endif /* MIDI_CTRL_ENABLED */

