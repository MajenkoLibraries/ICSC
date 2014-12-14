ICSC
====

Inter-Chip Serial Communications
--------------------------------

A simple to use yet powerful system for communicating
using RS232, RS485, etc between Arduinos, chipKITs, PCs, etc.


Packet format
-------------

All values are 8-bit and symbolic names are ASCII standard values.

Preamble:

    SOH
    SOH
    ...
    SOH

Packet:

    SOH
    Dest ID
    Origin ID
    Command
    Data Len
    STX
    [Data Len bytes]
    ETX
    Checksum
    EOT

Checksum is sum of all bytes between (but not including) SOH and ETX, modulus 256.
    
Special destination address 0x00 is the broadcast. All stations will receive and
act upon messages sent to this address.

Addresses that equate to the ASCII symbols SOH etc should be avoided.

Packet Reception
----------------

Packet reception and identification is through a 6-byte sliding window identifying
the header of a packet.  As bytes arrive they enter the top of the window and
they work their way down to the bottom as more bytes arrive.  When the first
byte of the window is SOH, the last byte is STX, and the Dest ID is either 0x00
or the ID of the receiving station, it is assumed a valid packet is arriving.
From this point on Data Len bytes are then read into the data buffer.
If the next three bytes do not equal ETX, the valid checksum, and EOT, then the
packet is rejected.  Otherwise the function associated with Command is executed.
