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
