/*****************************************************************************/
/* MIDI input handler                                                        */
/*                                                                           */
/* Copyright (C) 2017 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __midiInput_h
#define __midiInput_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <midiTypes.h>


/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void midiInputInitialize(void);
bool midiInputIsEmpty(void);
void midiInputDataReceived(uint8_t in_data);
uint16_t midiInputEventPopAndStore(uint8_t* in_buffer, uint16_t in_buffer_length);
bool midiInputIsReceiving(void);
void midiInputClear(void);

#endif
