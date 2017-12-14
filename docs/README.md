Usage
=====

Wiring
------

The most basic connection is just to link the TX and RX of two Arduinos together. TX from one connects to RX of the other, and vice versa.

For connecting more boards together an [RS-485](RS485.md) network is recommended.

Library initialization
----------------------

You first need to create an ICSC object using one of the constructors:

```C++
ICSC(Stream *d, uint8_t station);
ICSC(Stream &d, uint8_t station);
ICSC(Stream *d, uint8_t station, int depin);
ICSC(Stream &d, uint8_t station, int depin);
```

The `Stream` is the device you wish to communicate over. `station` is the number (or single letter code) of this station, and `depin` is, if provided, the DE/RE pin pair of a MAX485 chip or similar.

Example:

```C++
#include <ICSC.h>

ICSC icsc(Serial, 'A', 3);

void setup()
{
  icsc.begin();
}
```

Register a reception command
----------------------------

```C++
ICSC::registerCommand(commandID, commandCallback);
```

Register a new command to be acted upon. Calls the function commandCallback when a packet with this commandID is received.

Example:

```C++
icsc.registerCommand('T', &tick);

void tick(unsigned char source, char command, unsigned char length, char *data)
{
  // ...
}
```

Unregister a reception command
------------------------------

```C++
ICSC::unregisterCommand(commandID);
```

Remove a registered callback command from the list of recognized commands at this station.

Example:

```C++
icsc.unregisterCommand('T');
```

Send a packet
-------------

```C++
ICSC::send(destination, commandID, length, data);
```

Send a packet to a remote station. The commandID should match a command registered in the remote station. Length is the number of bytes of data to include in the packet. Data is a pointer to the data to send (should be cast to a char * if it isn't already).

Example:

```C++
icsc.send(6, 'T', 5, (char *)&myData);
```

Send a broadcast
----------------

```C++
ICSC::broadcast(command, length, data);
```

Send a packet to all stations. The default address for broadcasting is 0 (can be edited in the ICSC.h file). Apart from that broadcast() works like send().

Example:

```C++
icsc.broadcast('T', 1, (char *)&myData);
```

Performing reception
--------------------

```C++
ICSC::process();
```

This function should be called during any long loops, and on every pass of the main loop() function.

Example:

```C++
void loop()
{
  icsc.process();
  // ...
}
```
