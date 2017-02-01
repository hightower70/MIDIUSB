/*****************************************************************************/
/* MIDI Output buffer handling functions                                     */
/*                                                                           */
/* Copyright (C) 2017 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __midiOutBuffer_h
#define __midiOutBuffer_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <midiTypes.h>

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void midiOutputInitialize(void);
bool midiOutputEventPush(USBMIDIEventPacket in_event);
bool midiOutputEventPopAndStore(uint8_t* in_midi_buffer, uint16_t in_midi_buffer_size, uint16_t* in_buffer_pos);
bool midiOutputIsEmpty(void);


#endif