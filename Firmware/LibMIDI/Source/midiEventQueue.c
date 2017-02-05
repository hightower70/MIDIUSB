/*****************************************************************************/
/* MIDI Event queue                                                          */
/*                                                                           */
/* Copyright (C) 2017 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include <midiEventQueue.h>
#include <sysHelpers.h>

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes MIDI event queue
/// @param in_event_queue Event queue state descriptor
/// @param in_buffer Queue storage for events
/// @param in_buffer_size Queue buffer size in number of events
void midiEventQueueInitialize(midiEventQueueInfo* in_event_queue, USBMIDIEventPacket* in_buffer, uint16_t in_buffer_size)
{
	sysMemZero(in_event_queue, sizeof(midiEventQueueInfo));
	
	in_event_queue->EventQueue = in_buffer;
	in_event_queue->EventQueueSize = in_buffer_size;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Returns the state (empty/non empty) of the MIDI buffer
/// @param in_event_queue Event queue state descriptor
bool midiEventQueueIsEmpty(midiEventQueueInfo* in_event_queue)
{
	return (in_event_queue->PushIndex == in_event_queue->PopIndex);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Clears MIDI event queue
/// @param in_event_queue Event queue state descriptor
void midiEventQueueClear(midiEventQueueInfo* in_event_queue)
{
	in_event_queue->PushIndex = 0;
	in_event_queue->PopIndex = 0;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Pushes MIDI event into the MIDI output queue
/// @param in_event_queue Event queue state descriptor
/// @param in_event MIDI event to push
/// @return true if event was pushed or false is there is no space in the buffer
bool midiEventQueuePush(midiEventQueueInfo* in_event_queue, USBMIDIEventPacket in_event)
{
	uint16_t new_push_index;
	
	// generate new push pointer
	new_push_index = in_event_queue->PushIndex + 1;
	if (new_push_index >= in_event_queue->EventQueueSize)
		new_push_index = 0;
	
	// check for free space in the buffer
	if (new_push_index == in_event_queue->PopIndex)
		return false;
	
	// store event in the buffer
	in_event_queue->EventQueue[new_push_index].EventData = in_event.EventData;
	
	in_event_queue->PushIndex = new_push_index;
	
	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Pops MIDI event from the queue.
/// @param in_event_queue Event queue state descriptor
/// @return MIDI event, invalidated of there is no event in the queue
USBMIDIEventPacket midiEventQueuePop(midiEventQueueInfo* in_event_queue)
{
	USBMIDIEventPacket retval;
	uint16_t new_pop_pointer;
	
	retval.EventData = 0;
	
	// check if buffer contains event
	if (in_event_queue->PopIndex  != in_event_queue->PushIndex)
	{
		// increment pop pointer
		new_pop_pointer = in_event_queue->PopIndex + 1;
		if (new_pop_pointer >= in_event_queue->EventQueueSize)
			new_pop_pointer = 0;		
		
		retval.EventData = in_event_queue->EventQueue[new_pop_pointer].EventData;
		
		in_event_queue->PopIndex = new_pop_pointer;
	}
	
	return retval;
}

