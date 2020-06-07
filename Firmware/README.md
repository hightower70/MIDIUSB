# MIDIUSB - Firmware files
MIDI to USB converter cable using STM32F103 microcontroller. This folder contains the source code needed to compile the firmware.

The project files are included for CooCox IDE, STM32CubeIDE, EWARM and a makefile also included.

If you want to produce more than one unit, the USB serial number needs to be changed, so the host computer can distinguish between different instances. The serial number is located in the usbd_midi_desc.c file. Please see the USBD_SERIALNUMBER_STRING_FS define.
