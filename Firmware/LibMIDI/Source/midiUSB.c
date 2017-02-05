/*****************************************************************************/
/* USB MIDI interface                                                        */
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
#include <usbd_desc.h>
#include <usbd_midi.h>
#include <usbd_midi_if.h>
#include <midiOutput.h>
#include <midiInput.h>
#include <midiUSB.h>

/*****************************************************************************/
/* Module local functions                                                    */
/*****************************************************************************/
static int8_t midiUSBInitialize(void);
static int8_t midiUSBDeinitialize(void);
static int8_t midiUSBReceive(uint8_t* pbuf, uint32_t *Len);
static void midiUSBProcessUSBReceivedEvent(void);
static void midiUSBProcessUSBTransmitEvent(void);

/*****************************************************************************/
/* Module local variables                                                    */
/*****************************************************************************/

// Buffers and variables for MIDI->USB transmission
static uint8_t l_usb_transmit_buffer[USB_MIDI_DATA_HS_IN_SIZE];

// Buffers and variables for USB->MIDI transmission
static uint8_t l_usb_receive_buffer[USB_MIDI_DATA_HS_OUT_SIZE];
static USBMIDIEventPacket* l_pending_receive_event_buffer = NULL;
static uint16_t l_pending_receiver_event_buffer_count;
static uint16_t l_pending_receiver_event_buffer_index;

static USBD_MIDI_ItfTypeDef USBD_Interface_fops_FS = 
{
	midiUSBInitialize,
	midiUSBDeinitialize,
	midiUSBReceive
};

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

// USB Device Core handle declaration
USBD_HandleTypeDef hUsbDeviceFS;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/


//////////////////////////////////////////////////////////////////////////////
/// @brief Initializes USB MIDI device
void midiUSBDeviceInitialize(void)
{
  /* Init Device Library,Add Supported Class and Start the library*/
	USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);

	USBD_RegisterClass(&hUsbDeviceFS, &USBD_MIDI);

	USBD_MIDI_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);

	USBD_Start(&hUsbDeviceFS);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Check MIDI USB connection status
/// @return True if device is connected to the USB host and enumerated
bool midiIsConnected(void)
{
	return (hUsbDeviceFS.pClassData != NULL);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Initializes the MIDI USB low layer over the FS USB IP
/// @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
static int8_t midiUSBInitialize(void)
{
	// Set Application Buffers
	USBD_MIDI_SetTxBuffer(&hUsbDeviceFS, l_usb_transmit_buffer, 0);
	USBD_MIDI_SetRxBuffer(&hUsbDeviceFS, l_usb_receive_buffer);

	return (USBD_OK);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief DeInitializes the MIDI USB media low layer
/// @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
static int8_t midiUSBDeinitialize(void)
{
	return (USBD_OK);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Process data received over USB MIDI data OUT endpoint
/// @param  Buf: Buffer of data to be received
/// @param  Len: Number of data received (in bytes)
/// @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
static int8_t midiUSBReceive(uint8_t* Buf, uint32_t *Len)
{
	// prepare processing new received packet
	l_pending_receive_event_buffer = (USBMIDIEventPacket*)Buf;
	l_pending_receiver_event_buffer_count = (uint16_t)(*Len / sizeof(USBMIDIEventPacket));
	l_pending_receiver_event_buffer_index = 0;
	
	// process received packet
	midiUSBProcessUSBReceivedEvent();

	return (USBD_OK);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Handles task related to MIDI USB communication
void midiUSBTask(void)
{
	// process events received over USB
	midiUSBProcessUSBReceivedEvent();
	midiUSBProcessUSBTransmitEvent();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Transmits data over USB
static void midiUSBProcessUSBTransmitEvent(void)
{
	int16_t packet_length;
	
	// check for connection
	if(midiIsConnected())
	{
		// if connected transfer received MIDI event over the USB
		if (!USBD_MIDI_IsTransmitterBusy(&hUsbDeviceFS) && !midiInputIsEmpty())
		{
			packet_length = midiInputEventPopAndStore(l_usb_transmit_buffer, USB_MIDI_DATA_HS_IN_SIZE);

			if (packet_length > 0)
			{
				USBD_MIDI_SetTxBuffer(&hUsbDeviceFS, l_usb_transmit_buffer, packet_length);
				USBD_MIDI_TransmitPacket(&hUsbDeviceFS);
			}
		}
	}
	else
	{
		// if not connected clear MIDI input buffer
		midiInputClear();
	}
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Processes received data from MIDI USB 
static void midiUSBProcessUSBReceivedEvent(void)
{
	// do nothing if it is not connected
	if(!midiIsConnected())
		return;

	// if buffer is not empty -> process buffer
	if (l_pending_receive_event_buffer != NULL)
	{
		// copy events to the event buffer
		while (l_pending_receiver_event_buffer_index < l_pending_receiver_event_buffer_count)
		{
			if (!midiOutputEventPush(l_pending_receive_event_buffer[l_pending_receiver_event_buffer_index]))
				break;

			l_pending_receiver_event_buffer_index++;
		}
		
		// if all events could be copied into event buffer then reset buffer
		if (l_pending_receiver_event_buffer_index >= l_pending_receiver_event_buffer_count)
		{
			l_pending_receive_event_buffer = NULL;
			l_pending_receiver_event_buffer_count = 0;
			l_pending_receiver_event_buffer_index = 0;
		}
	}
	
	// if buffer is empty and receiver is not started -> start it
	if(!USBD_MIDI_IsReceiverBusy(&hUsbDeviceFS))
	{
		if(l_pending_receive_event_buffer == NULL)
		{
			// restart receiver
			USBD_MIDI_SetRxBuffer(&hUsbDeviceFS, l_usb_receive_buffer);
			USBD_MIDI_ReceivePacket(&hUsbDeviceFS);
		}
	}
}




