#ifndef __halMIDIPort_h
#define __halMIDIPort_h

#include <midiTypes.h>

void halMIDIPortInitialize(void);
void halMIDIPortSendBuffer(uint8_t* in_buffer, uint16_t in_buffer_length);
bool halMIDIIsTransmitterEmpty(void);

#endif
