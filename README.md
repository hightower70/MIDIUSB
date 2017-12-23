# MIDIUSB
This is a replacement board for the cheap chineese USB to MIDI converter for fixing all issues with the cheap electronics. 

The original converter has some design flaws. It has usually no optocoupler on the MIDI input which makes it incompatible with some devices. The firmware has more problems: the midi running status is not handled on the input side, stange "note on" messages are inserted in the case of receiving running status for any controller. The sysex handling is completelly disaster, it handles the first few bytes correctly, after that it starts loosing data bytes, usually all bytes. The sysex input is the same, only the first few bytes are received, the other is discarded or only garbage is received.

This hardware fixes those issues and makes the cable suitable for daily usage.

The repository contains all information to build the cable, it has the schematics, PCB design and firmware source code. For downloading the firmware you will need an STLINK-V2 compatible programmer. The software was developed in CooCox development environment.

# Images

<p align="center">
  <img width="434" height="218" src="https://user-images.githubusercontent.com/6670256/33966057-7545b242-e05e-11e7-905b-6156708f78c6.jpg">
  <br>
  The replacement board
  <br><br>
  <img width="504" height="378" src="https://user-images.githubusercontent.com/6670256/33966059-7800ac1c-e05e-11e7-9116-aeec1af2bc79.jpg">
  <br><br>
  The complete device  
</p>
