Usage
=====

Wiring
------

The most basic connection is just to link the TX and RX of two Arduinos together. TX from one connects to RX of the other, and vice versa.

For connecting more boards together an [RS-485](RS485.md) network is recommended.

Library initialization
----------------------

```C++
ICSC.begin(stationId, baudRate);
ICSC.begin(stationId, baudRate, &SerialPort);
ICSC.begin(stationId, baudRate, &SerialPort, DEPin);
ICSC.begin(stationId, baudRate, DEPin);
```

Initialize the serial port and internal structures. stationId is the ID of the local station. Set the baud rate to be the same on all stations. &SerialPort is a pointer to a HardwareSerial device, such as &Serial1. If not specified, defaults to &Serial. DEPin is the pin to use to control the RE#/DE pins of an RS-485 transceiver chip if needed. If not specified RE#/DE operation is disabled.

Example:

```C++
#include <ICSC.h>

void setup()
{
  ICSC.begin(3, 115200, &Serial1);
}
```

Register a reception command
----------------------------

```C++
ICSC.registerCommand(commandID, commandCallback);
```

Register a new command to be acted upon. Calls the function commandCallback when a packet with this commandID is received.

Example:

```C++
ICSC.registerCommand('T', &tick);

void tick(unsigned char source, char command, unsigned char length, char *data)
{
  // ...
}
```

Unregister a reception command
------------------------------

```C++
ICSC.unregisterCommand(commandID);
```

Remove a registered callback command from the list of recognized commands at this station.

Example:

```C++
ICSC.unregisterCommand('T');
```

Send a packet
-------------

```C++
ICSC.send(destination, commandID, length, data);
```

Send a packet to a remote station. The commandID should match a command registered in the remote station. Length is the number of bytes of data to include in the packet. Data is a pointer to the data to send (should be cast to a char * if it isn't already).

Example:

```C++
ICSC.send(6, 'T', 5, (char *)&myData);
```

Send a broadcast
----------------

```C++
ICSC.broadcast(command, length, data);
```

Send a packet to all stations. The default address for broadcasting is 0 (can be edited in the ICSC.h file). Apart from that broadcast() works like send().

Example:

```C++
ICSC.broadcast('T', 1, (char *)&myData);
```

Performing reception
--------------------

```C++
ICSC.process();
```

This function should be called during any long loops, and on every pass of the main loop() function.

Example:

```C++
void loop()
{
  ICSC.process();
  // ...
}
```
