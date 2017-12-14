# MIDIUSB
This is a replacement board for the cheap chineese USB to MIDI converter for fixing all issues with the cheap electronics. 

The original converter has some design flaws. It has usually no optocoupler at the MIDI input which makes less incompatible with some devices. The firmware has some more problems: the minid running status is not handled, stange notes inserted instead in the case of using running status for any controller. The sysex handling is completelly disaster, it handles the first few bytes correctly, after that it starts loosing data bytes. 

This hardware fixes those issues and makes the cable suitable for daily usage.

The repository contains all information to build the converter, it has the schematics, PCB design and firmware source code.

![The board](https://user-images.githubusercontent.com/6670256/33966057-7545b242-e05e-11e7-905b-6156708f78c6.jpg)
The replacement board 

![The device](https://user-images.githubusercontent.com/6670256/33966059-7800ac1c-e05e-11e7-9116-aeec1af2bc79.jpg)
The complete device
