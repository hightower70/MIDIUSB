# MIDIUSB #
This is a replacement board for the cheap chineese USB to MIDI converter for fixing all issues with the cheap electronics. 

The original converter has some design flaws. It has usually no optocoupler on the MIDI input which makes it incompatible with some devices. The firmware has more problems: the midi running status is not handled on the input side, stange "note on" messages are inserted in the case of receiving running status for any controller. The sysex handling is completelly disaster, it handles the first few bytes correctly, after that it starts loosing data bytes, usually all bytes. The sysex input is the same, only the first few bytes are received, the other is discarded or only garbage is received.

This hardware fixes those issues and makes the cable suitable for daily usage.

The repository contains all information to build the cable, it has the schematics, PCB design and firmware source code. For downloading the firmware you will need an STLINK-V2 compatible programmer. The software was developed in CooCox development environment.

## Changing the USB cable name ##

By default the cable is displayed as "USB-MIDI Cable" in the control panel or any list of MIDI devices. If more than one cable is attached to the computer it is hard to identify each cable. Therefore the displyed name of cable can be changed. For changing the name a special System Exclusive message needs to be sent to the cable from the PC (via the USB port). Any program capable of sending sysex is suitable for this purpose e.g. MidiOX is one of the free software for sending sysex. The sysex message can be created by the provided command line tool 'SysexGen'. 

The usage is:
SysexGen filename.syx "Cable Name"

Where 'filename' is the name of the sysex message, the 'Cable Name' is the desired display name of the cable. The cable name can't be longer than 16 characters. Empty name will restore the original name.

Once the cabe name has been changed the cable needs to be removed and reconnected again. If the name is not changed it is recomended to uninstall the cable driver from the Control Panel of the windows and reconnect it again.

# Images #

<p align="center">
  <img width="434" height="218" src="https://user-images.githubusercontent.com/6670256/33966057-7545b242-e05e-11e7-905b-6156708f78c6.jpg">
  <br>
  The replacement board
  <br><br>
  <img width="504" height="378" src="https://user-images.githubusercontent.com/6670256/33966059-7800ac1c-e05e-11e7-9116-aeec1af2bc79.jpg">
  <br><br>
  The complete device  
</p>

# PCB #
The PCB can be ordered directly from the following link: [Dirty Cheap PCB](http://dirtypcbs.com/store/designer/details/8873/5868/midiusb-zip)
