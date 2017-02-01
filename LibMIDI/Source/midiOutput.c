/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include <midiOutput.h>
#include <midiEventQueue.h>
#include <sysConfig.h>

/*****************************************************************************/
/* Module local variables                                                    */
/*****************************************************************************/

// MIDI out event buffer
static USBMIDIEventPacket l_midi_output_queue_buffer[midiOUT_BUFFER_MAX_EVENT_COUNT];
static midiEventQueueInfo l_midi_output_queue;

static MidiMessageType l_running_status;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize MIDI output
void midiOutputInitialize(void)
{
	midiEventQueueInitialize(&l_midi_output_queue, l_midi_output_queue_buffer, midiOUT_BUFFER_MAX_EVENT_COUNT);
	l_running_status = MIDI_INVALID_TYPE;
}

///////////////////////////////////////////////////////////////////////////////
/// @param Returns the state of the MIDI Out buffer
bool midiOutputIsEmpty(void)
{
	return midiEventQueueIsEmpty(&l_midi_output_queue);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Pushes MIDI event into the MIDI output queue
/// @param in_event MIDI event to push
/// @return true if event was pushed or false is there is no space in the buffer
bool midiOutputEventPush(USBMIDIEventPacket in_event)
{
	return midiEventQueuePush(&l_midi_output_queue, in_event);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Pops MIDI event from the queue and converts it to a regular midi message and stores it in the given midi message buffer.
/// @param in_midi_buffer MIDI message buffer to store the popped and converted event
/// @param in_midi_buffer_size Size of the MIDI message buffer in bytes
/// @param in_buffer_pos Address of the index of the first free byte in the MIDI buffer. The pointer will be updated to point again to the first free byte.
bool midiOutputEventPopAndStore(uint8_t* in_midi_buffer, uint16_t in_midi_buffer_size, uint16_t* in_buffer_pos)
{
	uint16_t buffer_pos = *in_buffer_pos;
	USBMIDIEventPacket event;
	uint16_t new_pop_pointer;
	
	// check free space in the target buffer (MIDI event is not longer than 3 bytes)
	if (buffer_pos + 3 >= in_midi_buffer_size)
		return false;	
	
	// check if buffer contains event
	if (midiEventQueueIsEmpty(&l_midi_output_queue))
		return false;
	
	// pop event
	event = midiEventQueuePop(&l_midi_output_queue);

	// convert data from USB event format to MIDI message and store it in the buffer
	switch (event.PacketHeader)
	{
		// Miscellaneous function codes.Reserved for future extensions.
		case CIN_MISC:
			l_running_status = MIDI_INVALID_TYPE;
			break;
			
		// Cable events.Reserved for future expansion.			
		case CIN_CABLE_EVENT:
			l_running_status = MIDI_INVALID_TYPE;
			break;
			
		// Two - byte System Common messages like MTC, SongSelect, etc.
		case CIN_TWO_BYTE_SYSTEM_COMMON_MESSAGE:
			in_midi_buffer[buffer_pos++] = event.Data0;
			in_midi_buffer[buffer_pos++] = event.Data1;
			l_running_status = MIDI_INVALID_TYPE;
			break;
			
		// Three - byte System Common messages like SPP, etc.
		case CIN_THREE_BYTE_SYSTEM_COMMON_MESSAGE:	
			in_midi_buffer[buffer_pos++] = event.Data0;
			in_midi_buffer[buffer_pos++] = event.Data1;
			in_midi_buffer[buffer_pos++] = event.Data2;
			l_running_status = MIDI_INVALID_TYPE;
			break;
		
		// SysEx starts or continues
		case CIN_SYSEX:
			in_midi_buffer[buffer_pos++] = event.Data0;
			in_midi_buffer[buffer_pos++] = event.Data1;
			in_midi_buffer[buffer_pos++] = event.Data2;
			l_running_status = MIDI_INVALID_TYPE;
			break;
			
		// Single byte System Common Message or SysEx ends with following single byte.
		case CIN_SYSEX_END_1:							
			in_midi_buffer[buffer_pos++] = event.Data0;
			l_running_status = MIDI_INVALID_TYPE;
			break;
			
		// SysEx ends with following two bytes.
		case CIN_SYSEX_END_2:
			in_midi_buffer[buffer_pos++] = event.Data0;
			in_midi_buffer[buffer_pos++] = event.Data1;
			l_running_status = MIDI_INVALID_TYPE;
			break;
			
		// SysEx ends with following three bytes.
		case CIN_SYSEX_END_3:
			in_midi_buffer[buffer_pos++] = event.Data0;
			in_midi_buffer[buffer_pos++] = event.Data1;
			in_midi_buffer[buffer_pos++] = event.Data2;
			l_running_status = MIDI_INVALID_TYPE;
			break;
			
		// Note - off
		case CIN_NOTE_OFF:
			if (l_running_status != event.Data0)					// send status if needed
				in_midi_buffer[buffer_pos++] = event.Data0;
			in_midi_buffer[buffer_pos++] = event.Data1;
			in_midi_buffer[buffer_pos++] = event.Data2;
			l_running_status = event.Data0;
			break;
			
		// Note - on
		case 	CIN_NOTE_ON:
			if (l_running_status != event.Data0)					// send status if needed
				in_midi_buffer[buffer_pos++] = event.Data0;
			in_midi_buffer[buffer_pos++] = event.Data1;
			in_midi_buffer[buffer_pos++] = event.Data2;
			l_running_status = event.Data0;
			break;
			
		// Poly - KeyPress
		case CIN_POLY_KEYPRESS:
			if (l_running_status != event.Data0)					// send status if needed
				in_midi_buffer[buffer_pos++] = event.Data0;
			in_midi_buffer[buffer_pos++] = event.Data1;
			in_midi_buffer[buffer_pos++] = event.Data2;
			l_running_status = event.Data0;
			break;
			
		// Control Change
		case CIN_CONTROL_CHANGE:
			if (l_running_status != event.Data0)					// send status if needed
				in_midi_buffer[buffer_pos++] = event.Data0;
			in_midi_buffer[buffer_pos++] = event.Data1;
			in_midi_buffer[buffer_pos++] = event.Data2;
			l_running_status = event.Data0;
			break;
			
		// Program Change
		case CIN_PROGRAM_CHANGE:
			if (l_running_status != event.Data0)					// send status if needed
				in_midi_buffer[buffer_pos++] = event.Data0;
			in_midi_buffer[buffer_pos++] = event.Data1;
			l_running_status = event.Data0;
			break;
			
		// Channel Pressure
		case CIN_CHANNEL_PRESSURE:
			if (l_running_status != event.Data0)					// send status if needed
				in_midi_buffer[buffer_pos++] = event.Data0;
			in_midi_buffer[buffer_pos++] = event.Data1;
			l_running_status = event.Data0;
			break;
			
		// PitchBend Change
		case CIN_PITCH_BEND:
			if (l_running_status != event.Data0)					// send status if needed
				in_midi_buffer[buffer_pos++] = event.Data0;
			in_midi_buffer[buffer_pos++] = event.Data1;
			in_midi_buffer[buffer_pos++] = event.Data2;
			l_running_status = event.Data0;
			break;
			
		// Single Byte
		case CIN_SINGLE_BYTE:
			in_midi_buffer[buffer_pos++] = event.Data0;
			break;
	}

	// update pointer
	*in_buffer_pos = buffer_pos;
	
	return true;
}