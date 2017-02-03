/*****************************************************************************/
/* USB MIDI converter main File                                              */
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

#define midiOUT_BUFFER_LENGTH 256

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static sysTick l_tx_timestamp = 0;
static sysTick l_rx_timestamp = 0;

static uint8_t l_midi_out_buffer[midiOUT_BUFFER_LENGTH];

/*****************************************************************************/
/* Module local functions                                                    */
/*****************************************************************************/
static void appSetLEDState(void);
static void appUSBToMIDITask(void);

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/

void appInitialization(void)
{
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
			repeat = midiOutputEventPopAndStore(l_midi_out_buffer, midiOUT_BUFFER_LENGTH, &pos);
		} while (repeat);
		
		halMIDIPortSendBuffer(l_midi_out_buffer, pos);
	}
}

