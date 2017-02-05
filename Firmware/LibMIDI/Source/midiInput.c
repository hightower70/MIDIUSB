/*****************************************************************************/
/* MIDI input handler                                                        */
/*                                                                           */
/* Copyright (C) 2017 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <midiInput.h>
#include <midiEventQueue.h>
#include <sysConfig.h>

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static USBMIDIEventPacket l_midi_input_queue_buffer[midiIN_BUFFER_MAX_EVENT_COUNT];
static midiEventQueueInfo l_midi_input_queue;
static MidiMessageType l_running_status;
static USBMIDIEventPacket l_current_input_packet;
static uint8_t l_current_received_byte_count;
static uint8_t l_expected_byte_count;
static bool l_sysex_receiving;
static volatile bool l_is_receiving;

/*****************************************************************************/
/* Module local functions                                                    */
/*****************************************************************************/
static uint8_t midiGetMessageLength(MidiMessageType in_message_status);
static USBMIDICodeIndexNumber midiGetUSBMIDICodeIndexNumber(MidiMessageType in_message_status, uint16_t in_message_length);


/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes MIDI input
void midiInputInitialize(void)
{
	midiEventQueueInitialize(&l_midi_input_queue,	l_midi_input_queue_buffer, midiIN_BUFFER_MAX_EVENT_COUNT);
	l_running_status = MIDI_INVALID_TYPE;
	l_current_received_byte_count = 0;
	l_expected_byte_count = 0;
	l_sysex_receiving = false;
	l_is_receiving = false;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Clears MIDI input queue
void midiInputClear(void)
{
	midiEventQueueClear(&l_midi_input_queue);
}

///////////////////////////////////////////////////////////////////////////////
/// @Brief Returns the state of the MIDI In buffer
/// @return true if MIDI In queue is empty
bool midiInputIsEmpty(void)
{
	return midiEventQueueIsEmpty(&l_midi_input_queue);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Returns true when valid midi message is received since the last call of this function
/// @return true when message is received since the last call of the function
bool midiInputIsReceiving(void)
{
	bool retval = l_is_receiving;
	
	l_is_receiving = false;
	
	return retval;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Pops MIDI event from the queue and copies to the given buffer
/// @param in_buffer A buffer to receive MIDI events
/// @param in_buffer_length Size of the MIDI event buffer in bytes
/// @retval Number of bytes copied to the buffer
uint16_t midiInputEventPopAndStore(uint8_t* in_buffer, uint16_t in_buffer_length)
{
	USBMIDIEventPacket* buffer = (USBMIDIEventPacket*)in_buffer;
	uint16_t buffer_event_size = in_buffer_length / sizeof(USBMIDIEventPacket);
	uint16_t event_index;
	USBMIDIEventPacket event;
	
	// pop and copy events from the queue 
	event_index = 0;
	do
	{
		event = midiEventQueuePop(&l_midi_input_queue);
		
		if (event.EventData != 0)
		{
			buffer[event_index++].EventData = event.EventData;
		}
	} while ( event_index < buffer_event_size && event.EventData != 0);
	
	// return used buffer size
	return event_index * sizeof(USBMIDIEventPacket);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Stores received data into the MIDI in queue
void midiInputDataReceived(uint8_t in_data)
{
	USBMIDIEventPacket realtime_message;
	
	// check if incoming data is a status code
	if (midiIS_STATUS_CODE(in_data))
	{
		// if it's real time message -> store it
		if (midiIS_REAL_TIME_MESSAGE(in_data))
		{
			realtime_message.EventData = 0;
			realtime_message.PacketHeader = CIN_SINGLE_BYTE;
			realtime_message.Data0 = in_data;
			
			midiEventQueuePush(&l_midi_input_queue, realtime_message);
			l_is_receiving = true;
		}
		else
		{
			if (l_sysex_receiving)
			{
				// if sysex end -> store it
				if (in_data == MIDI_SYSTEM_EXLUSIVE_END)
					l_current_input_packet.Data[l_current_received_byte_count++] = in_data;

				// store sysex message
				l_current_input_packet.PacketHeader = midiGetUSBMIDICodeIndexNumber(MIDI_SYSTEM_EXCLUSIVE, l_current_received_byte_count);
				midiEventQueuePush(&l_midi_input_queue, l_current_input_packet);
				l_is_receiving = true;

				// if message was interrupted by non-realtime status code -> store the broken sysex and start storing the new message
				if(in_data != MIDI_SYSTEM_EXLUSIVE_END)
				{
					l_current_input_packet.Status = in_data;
					l_current_received_byte_count = 1;

					l_expected_byte_count = midiGetMessageLength(l_current_input_packet.Status);
				}

				// close sysex receiving
				l_sysex_receiving = false;
			}
			else
			{
				if (in_data == MIDI_SYSTEM_EXCLUSIVE)
				{
				
					// store received byte
					l_current_input_packet.EventData = 0;
					l_current_input_packet.Data0 = in_data;
					l_current_received_byte_count = 1;
					l_expected_byte_count = 3; // max. 3 bytes per event package

					// flag sysex receiving
					l_sysex_receiving = true;

					// invalidate running status
					l_running_status = MIDI_INVALID_TYPE;
				}
				else
				{
					// store status if channel message is received
					if (midiIS_CHANNEL_MESSAGE(in_data))
						l_running_status = (MidiMessageType)in_data;	// channel message -> store status
					else
						l_running_status = MIDI_INVALID_TYPE; // system common message -> invalidate running status
					
					l_current_input_packet.Status = in_data;
					l_current_received_byte_count = 1;
			
					l_expected_byte_count = midiGetMessageLength(l_current_input_packet.Status);
				}
			}
		}
	}
	else
	{
		// store non status codes
		if(l_sysex_receiving)
		{
			// sysex receiving
			if (l_current_received_byte_count == 0)
			{
				// init new message packet for sysex data
				l_current_input_packet.EventData = 0;
				l_current_input_packet.Data0 = in_data;
				l_current_received_byte_count = 1;
				l_expected_byte_count = 3; // max. 3 bytes per event package
			}
			else
			{
				// store data
				if (l_current_received_byte_count <= 3)
				{
					l_current_input_packet.Data[l_current_received_byte_count] = in_data;
				}

				// increment pointer
				if (l_current_received_byte_count <= l_expected_byte_count)
					l_current_received_byte_count++;
			}
		}
		else
		{
			// check for data value at the first position (running status needs to be applied)
			if (l_current_received_byte_count == 0)
			{
				// running status
				l_current_input_packet.Status = (uint8_t)l_running_status;
				l_current_input_packet.Data1 = in_data;
				l_current_received_byte_count = 2;

				l_expected_byte_count = midiGetMessageLength(l_current_input_packet.Status);
			}
			else
			{
				// store data
				if (l_current_received_byte_count > 0 && l_current_received_byte_count <= 3)
				{
					l_current_input_packet.Data[l_current_received_byte_count] = in_data;
				}

				// increment pointer
				if (l_current_received_byte_count <= l_expected_byte_count)
					l_current_received_byte_count++;
			}
		}
	}
	
	// store message if all bytes received
	if (l_expected_byte_count > 0 && l_current_received_byte_count == l_expected_byte_count)
	{
		if(l_sysex_receiving)
			l_current_input_packet.PacketHeader = CIN_SYSEX;
		else
			l_current_input_packet.PacketHeader = midiGetUSBMIDICodeIndexNumber(l_current_input_packet.Status, l_current_received_byte_count);

		midiEventQueuePush(&l_midi_input_queue, l_current_input_packet);
		l_current_received_byte_count = 0;
		l_expected_byte_count = 0;
		l_is_receiving = true;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets MIDI message length based on status code
/// @param in_message_type Status code of the current message
static uint8_t midiGetMessageLength(MidiMessageType in_message_status)
{
	static uint8_t channel_voice_message_length[] =
	{ 
		3, // Note Off
		3, // Note On
		3, // Polyphonic AfterTouch
		3, // Control Change / Channel Mode	
		2, // Program Change
		2, // Channel (monophonic) AfterTouch
		3, // Pitch Bend	
		1, // Real time
	};
	
	if (in_message_status < midiIS_STATUS_CODE(in_message_status))
	{
		// invalid status
		return 0;
	}
	else
	{
		if (midiIS_REAL_TIME_MESSAGE(in_message_status))
		{
			// real time message
			return 1;
		}
		else
		{
			if (midiIS_CHANNEL_MESSAGE(in_message_status))
			{
				// channel message
				return channel_voice_message_length[(midiGET_STATUS_CODE(in_message_status) >> 4) & 0x07];
			}
			else
			{
				// handle system common messages
				switch(in_message_status)
				{
					// System Common - MIDI Time Code Quarter Frame
					case 	MIDI_TIME_CODE_QUARTER_FRAME:
						return 2;

					// System Common - Song Position Pointer
					case MIDI_SONG_POSITION:
						return 3;

					// System Common - Song Select
					case MIDI_SONG_SELECT:
						return 2;

					// System Common - Tune Request
					case MIDI_TUNE_REQUEST:
						return 1;

					// invalid
					default:
						return 0;
				}
			}
		}
	}
}
	
///////////////////////////////////////////////////////////////////////////////
/// @brief 
static USBMIDICodeIndexNumber midiGetUSBMIDICodeIndexNumber(MidiMessageType in_message_status, uint16_t in_message_length)
{
	static USBMIDICodeIndexNumber channel_voice_message_header[] =
	{ 
		CIN_NOTE_OFF,					// Note Off
		CIN_NOTE_ON,					// Note On
		CIN_POLY_KEYPRESS,		// Polyphonic AfterTouch
		CIN_CONTROL_CHANGE,		// Control Change / Channel Mode	
		CIN_PROGRAM_CHANGE,		// Program Change
		CIN_CHANNEL_PRESSURE, // Channel (monophonic) AfterTouch
		CIN_PITCH_BEND,				// Pitch Bend	
		0,										// Not used
	};	
	
	if (midiIS_REAL_TIME_MESSAGE(in_message_status))
	{
		// handle real time messages
		return CIN_SINGLE_BYTE;
	}
	else
	{
		if (midiIS_CHANNEL_MESSAGE(in_message_status))
		{
			// voice channel messages
			return channel_voice_message_header[(midiGET_STATUS_CODE(in_message_status) >> 4) & 0x07];
		}
		else
		{
			if(in_message_status == MIDI_SYSTEM_EXCLUSIVE)
			{
				// handle system exclusive messages
				switch (in_message_length)
				{
					case 1:
						return CIN_SYSEX_END_1;

					case 2:
						return CIN_SYSEX_END_2;

					case 3:
						return CIN_SYSEX_END_3;
				}
			}
			else
			{
				// handle system common messages
				switch(in_message_status)
				{
					// System Common - MIDI Time Code Quarter Frame
					case 	MIDI_TIME_CODE_QUARTER_FRAME:
						return CIN_TWO_BYTE_SYSTEM_COMMON_MESSAGE;

					// System Common - Song Position Pointer
					case MIDI_SONG_POSITION:
						return CIN_THREE_BYTE_SYSTEM_COMMON_MESSAGE;

					// System Common - Song Select
					case MIDI_SONG_SELECT:
						return CIN_TWO_BYTE_SYSTEM_COMMON_MESSAGE;

					// System Common - Tune Request
					case MIDI_TUNE_REQUEST:
						return CIN_SYSEX_END_1;

					// invalid
					default:
						return CIN_MISC;
				}
			}
		}
	}
	
	return CIN_MISC;
}
