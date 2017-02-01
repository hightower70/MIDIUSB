/*****************************************************************************/
/* MIDI Output buffer handling functions                                     */
/*                                                                           */
/* Copyright (C) 2017 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __midiUSB_h
#define __midiUSB_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <midiTypes.h>

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void midiUSBDeviceInitialize(void);
void midiUSBTask(void);
uint8_t midiUSBTransmit(uint8_t* Buf, uint16_t Len);
bool midiIsConnected(void);


#endif

