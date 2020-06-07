/*****************************************************************************/
/* USB-MIDI converter main File                                              */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <stdlib.h>
#include <sysMain.h>
#include <sysTimer.h>
#include <halHelpers.h>
#include <sysConfig.h>
#include <midiUSB.h>
#include <midiOutput.h>
#include <midiInput.h>
#include <halMIDIPort.h>
#include <usbd_def.h>
#include <usbd_ctlreq.h>
#include "stm32f1xx_hal.h"


/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define midiOUT_BUFFER_LENGTH 256
#define midiSYSEX_HEADER_LENGTH 4
#define midiSYSEX_PID_LENGTH 4
#define midiSYSEX_END 0xf7
#define midiMAX_CABLE_NAME_LENGTH 16
#define CABLE_NAME_FLASH_START_ADDRESS 0x0800FC00

/*****************************************************************************/
/* External variables                                                        */
/*****************************************************************************/
extern uint8_t USBD_FS_DeviceDesc[USB_LEN_DEV_DESC];

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static sysTick l_tx_timestamp = 0;
static sysTick l_rx_timestamp = 0;

static uint16_t l_peek_index = 0;

static uint8_t l_midi_out_buffer[midiOUT_BUFFER_LENGTH];

static uint8_t l_sysex_header[] = {0xf0, 0x7d, 0x1a, 0x55};
static int l_sysex_pos = 0;

static char l_cable_name_buffer[midiMAX_CABLE_NAME_LENGTH+1]; // +1 because of the terminator zero
static char l_new_cable_name_buffer[midiMAX_CABLE_NAME_LENGTH+1];
static uint16_t l_pid;

static char* l_default_cable_name = "USB-MIDI Cable";

/*****************************************************************************/
/* Module local functions                                                    */
/*****************************************************************************/
static void appSetLEDState(void);
static void appUSBToMIDITask(void);
static void CheckForUSBNameSettings();
static void CheckSysexByte(uint8_t in_byte);

static void FLASHEnableWrite();
static void FLASHDisableWrite();
static uint16_t FLASHReadHalfWord(uint32_t address);
static HAL_StatusTypeDef FLASHWriteHalfWord(uint32_t address, uint16_t data);

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/

void appInitialization(void)
{
	uint16_t name_char;
	uint32_t address;
	uint16_t flag;
	uint16_t pid;

	// load name valid flag
	flag = FLASHReadHalfWord(0);

	// load cable name from FLASH
	if(flag == 0)
	{
		// read PID from flash
		pid = FLASHReadHalfWord(1);

		USBD_FS_DeviceDesc[10] = LOBYTE(pid);
		USBD_FS_DeviceDesc[11] = HIBYTE(pid);

		// read cable name from FLASH
		address = 0;
		do
		{
			name_char = FLASHReadHalfWord(address+2);
			l_cable_name_buffer[address] = (uint8_t)name_char;
			address++;
		} while ((uint8_t)name_char != '\0');

		l_cable_name_buffer[address] = '\0';
	}
	else
	{
		// use default cable name
		address = 0;
		name_char = l_default_cable_name[0];
		while( (uint8_t)name_char != '\0')
		{
			name_char = l_default_cable_name[address];
			l_cable_name_buffer[address] = (uint8_t)name_char;
			address++;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Main task function
void appMainTask(void)
{
	// LED blinking routine
	appSetLEDState();

	// task to handle MIDI communication (except USB->MIDI)
	midiUSBTask();

	// USB -> MIDI out task
	appUSBToMIDITask();
}
/*****************************************************************************/
/* Module local function implementation                                      */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Heart beat LED blinking routine
static void appSetLEDState(void)
{
	// Connection LED state
	if (midiIsConnected())
	{
		drvHAL_SetPinLow(CONNECT_LED_GPIO_Port, CONNECT_LED_Pin);
	}
	else
	{
		drvHAL_SetPinHigh(CONNECT_LED_GPIO_Port, CONNECT_LED_Pin);
	}

	// TX LED State
	if (midiOutputIsTransmitting())
	{
		drvHAL_SetPinLow(TX_LED_GPIO_Port, TX_LED_Pin);
		l_tx_timestamp = sysTimerGetTimestamp();
	}
	else
	{
		if (sysTimerGetTimeSince(l_tx_timestamp) > 100)
		{
			drvHAL_SetPinHigh(TX_LED_GPIO_Port, TX_LED_Pin);
		}
	}

	// RX LED State
	if (midiInputIsReceiving())
	{
		drvHAL_SetPinLow(RX_LED_GPIO_Port, RX_LED_Pin);
		l_rx_timestamp = sysTimerGetTimestamp();
	}
	else
	{
		if (sysTimerGetTimeSince(l_rx_timestamp) > 100)
		{
			drvHAL_SetPinHigh(RX_LED_GPIO_Port, RX_LED_Pin);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Send MIDI events from output queue to the MIDI out port
static void appUSBToMIDITask(void)
{
	uint16_t pos;
	bool repeat;

	if (!midiOutputIsEmpty() && halMIDIIsTransmitterEmpty())
	{
		pos = 0;
		do
		{
			CheckForUSBNameSettings();
			repeat = midiOutputEventPopAndStore(l_midi_out_buffer, midiOUT_BUFFER_LENGTH, &pos);
		} while (repeat);

		halMIDIPortSendBuffer(l_midi_out_buffer, pos);
	}
}

static void CheckForUSBNameSettings()
{
	USBMIDIEventPacket packet;

	// peek packet
	packet = midiEventQueuePeek(&g_midi_output_queue, &l_peek_index);

	switch (packet.PacketHeader)
	{
		// SysEx starts or continues
		case CIN_SYSEX:
			CheckSysexByte(packet.Data0);
			CheckSysexByte(packet.Data1);
			CheckSysexByte(packet.Data2);
			break;

		// Single byte System Common Message or SysEx ends with following single byte.
		case CIN_SYSEX_END_1:
			CheckSysexByte(packet.Data0);
			break;

		// SysEx ends with following two bytes.
		case CIN_SYSEX_END_2:
			CheckSysexByte(packet.Data0);
			CheckSysexByte(packet.Data1);
			break;

		// SysEx ends with following three bytes.
		case CIN_SYSEX_END_3:
			CheckSysexByte(packet.Data0);
			CheckSysexByte(packet.Data1);
			CheckSysexByte(packet.Data2);
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Analyzes incoming (over USB) SysEx bytes
static void CheckSysexByte(uint8_t in_byte)
{
	int i;

	// check for the first byte
	if(l_sysex_pos == 0)
	{
		if(in_byte == l_sysex_header[0])
		{
			l_sysex_pos++;
		}
	}
	else
	{
		// check for header
		if(l_sysex_pos < midiSYSEX_HEADER_LENGTH)
		{
			if(in_byte == l_sysex_header[l_sysex_pos])
			{
				l_sysex_pos++;
			}
			else
			{
				l_sysex_pos = 0;
			}
		}
		else
		{
			if(in_byte == midiSYSEX_END)
			{
				// finish sysex
				l_new_cable_name_buffer[l_sysex_pos - midiSYSEX_HEADER_LENGTH - midiSYSEX_PID_LENGTH] = '\0';
				l_sysex_pos = 0;

				// write new cable name to the FLASH
				FLASHEnableWrite();

				// write name only if it is not empty
				if(l_new_cable_name_buffer[0] != '\0')
				{
					// Flag name is valid
					FLASHWriteHalfWord(0,0);

					// Write PID
					FLASHWriteHalfWord(1,l_pid);

					// write name
					i = 0;
					while(l_new_cable_name_buffer[i] != '\0')
					{
						FLASHWriteHalfWord(i+2, l_new_cable_name_buffer[i]);

						i++;
					}

					// write terminator zero
					FLASHWriteHalfWord(i+2, '\0');
				}

				// protect FLASH
				FLASHDisableWrite();
			}
			else
			{
				if(l_sysex_pos < midiSYSEX_HEADER_LENGTH + midiSYSEX_PID_LENGTH)
				{
					in_byte &= 0x0f;

					switch(l_sysex_pos - midiSYSEX_HEADER_LENGTH)
					{
						case 0:
							l_pid = (uint16_t)in_byte;
							break;

						case 1:
							l_pid |= (uint16_t)in_byte << 4;
							break;

						case 2:
							l_pid |= (uint16_t)in_byte << 8;
							break;

						case 3:
							l_pid |= (uint16_t)in_byte << 12;
							break;
					}
					l_sysex_pos++;
				}
				else
				{
					// store cable name
					if((l_sysex_pos - midiSYSEX_HEADER_LENGTH - midiSYSEX_PID_LENGTH) < midiMAX_CABLE_NAME_LENGTH)
					{
						l_new_cable_name_buffer[l_sysex_pos - midiSYSEX_HEADER_LENGTH - midiSYSEX_PID_LENGTH] = in_byte;
						l_sysex_pos++;
					}
				}
			}
		}
	}
}

/*****************************************************************************/
/* USB Descriptor handling                                                   */
/*****************************************************************************/

extern uint8_t USBD_StrDesc[];

/**
* @brief  USBD_FS_ProductStrDescriptor
*         return the product string descriptor
* @param  speed : current device speed
* @param  length : pointer to data length variable
* @retval pointer to descriptor buffer
*/
uint8_t *  USBD_FS_ProductStrDescriptor( USBD_SpeedTypeDef speed , uint16_t *length)
{
	USBD_GetString ((uint8_t*)l_cable_name_buffer, USBD_StrDesc, length);

	return USBD_StrDesc;
}

/*****************************************************************************/
/* FLASH storage of cable name                                               */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Unprotects FLASH. Must be called before any write operation
void FLASHEnableWrite()
{
	uint32_t PAGEError = 0;
	FLASH_EraseInitTypeDef EraseInitStruct;

	HAL_FLASH_Unlock();

	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks = FLASH_BANK_1;
	EraseInitStruct.PageAddress = CABLE_NAME_FLASH_START_ADDRESS;
	EraseInitStruct.NbPages = 1;
	if(HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	{
	    //Erase error!
	}

	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_OPTVERR | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Turn on flash write protection
static void FLASHDisableWrite()
{
  HAL_FLASH_Lock();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief FLASH read (16 bit)
static uint16_t FLASHReadHalfWord(uint32_t address)
{
	uint16_t val = 0;

  address = address * 2 + CABLE_NAME_FLASH_START_ADDRESS;

  val = *(__IO uint16_t*)address;

  return val;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Writes FLASH half word
static HAL_StatusTypeDef FLASHWriteHalfWord(uint32_t address, uint16_t data)
{
	HAL_StatusTypeDef status;

  address = address * 2 + CABLE_NAME_FLASH_START_ADDRESS;

  status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, data);

  while(!__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) == RESET);

  return status;
}

