ICSC
====

Inter-Chip Serial Communications
--------------------------------

A simple to use yet powerful system for communicating
using RS232, RS485, etc between Arduinos, chipKITs, PCs, etc.


Packet format
-------------

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
    
