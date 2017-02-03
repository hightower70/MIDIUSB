/*****************************************************************************/
/* MIDI related type definitions                                             */
/*                                                                           */
/* Copyright (C) 2017 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef _midiTypes_h
#define _midiTypes_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/
#define midiGET_MIDI_EVENT_CABLE_NUMBER(x) (x & 0x0f)
#define midiGET_MIDI_EVENT_CODE_INDEX_NUMBER(x) ((x >> 4) & 0x0f)
#define midiIS_STATUS_CODE(x) ((x & 0x80) != 0)
#define midiGET_STATUS_CODE(x) (x & 0xf0)
#define midiGET_CHANNEL(x) ((x & 0x0f)+1)
#define midiIS_CHANNEL_MESSAGE(x)  (x >= MIDI_NOTE_OFF && x < MIDI_SYSTEM_EXCLUSIVE)
#define midiIS_REAL_TIME_MESSAGE(x) (x >= MIDI_CLOCK)
#define midiIS_SYSTEM_COMMON_MESSAGE(x) (x >= MIDI_SYSTEM_EXCLUSIVE && x < MIDI_CLOCK)

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

/// MIDI Message types
typedef enum
{
	MIDI_INVALID_TYPE = 0x00,							///< For notifying errors
	MIDI_NOTE_OFF = 0x80,									///< Note Off
	MIDI_NOTE_ON = 0x90,									///< Note On
	MIDI_AFTERTOUCH_POLY = 0xA0,					///< Polyphonic AfterTouch
	MIDI_CONTROL_CHANGE = 0xB0,						///< Control Change / Channel Mode
	MIDI_PROGRAM_CHANGE = 0xC0,						///< Program Change
	MIDI_AFTERTOUCH_CHANNEL = 0xD0,				///< Channel (monophonic) AfterTouch
	MIDI_PITCH_BEND = 0xE0,								///< Pitch Bend
	MIDI_SYSTEM_EXCLUSIVE = 0xF0,					///< System Exclusive
	MIDI_TIME_CODE_QUARTER_FRAME = 0xF1,  ///< System Common - MIDI Time Code Quarter Frame
	MIDI_SONG_POSITION = 0xF2,						///< System Common - Song Position Pointer
	MIDI_SONG_SELECT = 0xF3,							///< System Common - Song Select
	MIDI_TUNE_REQUEST = 0xF6,							///< System Common - Tune Request
	MIDI_SYSTEM_EXLUSIVE_END = 0xf7,			///< System exlusive end
	MIDI_CLOCK = 0xF8,									  ///< System Real Time - Timing Clock
	MIDI_START = 0xFA,										///< System Real Time - Start
	MIDI_CONTINUE = 0xFB,									///< System Real Time - Continue
	MIDI_STOP = 0xFC,											///< System Real Time - Stop
	MIDI_ACTIVE_SENSING = 0xFE,						///< System Real Time - Active Sensing
	MIDI_SYSTEM_RESET = 0xFF,							///< System Real Time - System Reset
} MidiMessageType; 

/// USB MIDI code index
typedef enum
{
	CIN_MISC = 0x0,																///< 1, 2 or 3 Miscellaneous function codes.Reserved for future extensions.
	CIN_CABLE_EVENT = 0x1,												///> 1, 2 or 3 Cable events.Reserved for future expansion.
	CIN_TWO_BYTE_SYSTEM_COMMON_MESSAGE = 0x2,			///< 2 Two byte System Common messages like MTC, SongSelect, etc.
	CIN_THREE_BYTE_SYSTEM_COMMON_MESSAGE = 0x03,	///< 3 Three byte System Common messages like SPP, etc.
	CIN_SYSEX = 0x4,															///< 3 SysEx starts or continues
	CIN_SYSEX_END_1 = 0x5,												///< 1 Single byte System Common Message or SysEx ends with following single byte.
	CIN_SYSEX_END_2 = 0x6,												///< 2 SysEx ends with following two bytes.
	CIN_SYSEX_END_3 = 0x7,												///< 3 SysEx ends with following three bytes.
	CIN_NOTE_OFF = 0x8,														///< 3 Note - off
	CIN_NOTE_ON = 0x9,														///< 3 Note - on
	CIN_POLY_KEYPRESS = 0xA,											///< 3 Poly - KeyPress
	CIN_CONTROL_CHANGE = 0xB,											///< 3 Control Change
	CIN_PROGRAM_CHANGE = 0xC,											///< 2 Program Change
	CIN_CHANNEL_PRESSURE = 0xD,										///< 2 Channel Pressure
	CIN_PITCH_BEND = 0xE,													///< 3 PitchBend Change
	CIN_SINGLE_BYTE = 0xF,												///< 1 Single Byte
} USBMIDICodeIndexNumber;

#pragma pack(1)

/// MIDI event struct
typedef union
{
	uint32_t EventData;

	struct
	{
		uint8_t PacketHeader;

		union
		{
			uint8_t Data[3];
			struct
			{
				uint8_t Status;
				uint8_t Param1;
				uint8_t Param2;
			};
			
			struct
			{
				uint8_t Data0;
				uint8_t Data1;
				uint8_t Data2;
			};
		};
	};
} USBMIDIEventPacket;

#pragma pack()



#endif
