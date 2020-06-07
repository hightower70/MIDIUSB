# SysexGen
The SysexGen is a small command line utility to create binary content of the SysEx message for changing cable name. By default the cable is displayed as "USB-MIDI Cable" in the control panel or any list of MIDI devices. If more than one cable is attached to the computer it is hard to identify each cable. Therefore the displayed name of cable can be changed. For changing the name a special System Exclusive message needs to be sent to the cable from the PC (via the USB port over MIDI protocol). Any program capable of sending sysex is suitable for this purpose e.g. MidiOX is one of the free software for sending sysex. The sysex message can be created by the provided command line tool 'SysexGen'.

The usage is: SysexGen filename.syx "Cable Name" pid

Where 'filename' is the name of the file to receive the binary sysex message, the 'Cable Name' is the desired display name of the cable, pid is the new USB PID for the cable. The cable name can't be longer than 16 characters. The 'pid' is the USB PID (Product ID). This also can be changed in order to differentiate cables. Please note, changing the USB PID can cause conflict to existing USB device. The recommended USB PID range is [2290..2299]. The default is 2290. The default settings can be restored by generating this sysex message:

SysexGen default.syx "USB-MIDI Cable" 2290

Once the cabe name has been changed the cable needs to be removed and reconnected again. If the name is not changed it is recomended to uninstall the cable driver from the Control Panel of the windows and reconnect it again.


