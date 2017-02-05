/*****************************************************************************/
/* MIDI Event queue                                                          */
/*                                                                           */
/* Copyright (C) 2017 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __midiEventQueue_h
#define __midiEventQueue_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <midiTypes.h>

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

/// MIDI event queue state description data
typedef struct
{
	USBMIDIEventPacket* EventQueue;
	uint16_t EventQueueSize;
	volatile uint16_t PushIndex;
	volatile uint16_t PopIndex;
} midiEventQueueInfo;

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/

void midiEventQueueInitialize(midiEventQueueInfo* in_event_queue, USBMIDIEventPacket* in_buffer, uint16_t in_buffer_size);
bool midiEventQueueIsEmpty(midiEventQueueInfo* in_event_queue);
bool midiEventQueuePush(midiEventQueueInfo* in_event_queue, USBMIDIEventPacket in_event);
USBMIDIEventPacket midiEventQueuePop(midiEventQueueInfo* in_event_queue);
void midiEventQueueClear(midiEventQueueInfo* in_event_queue);


#endif
