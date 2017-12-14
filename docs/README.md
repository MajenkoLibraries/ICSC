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

The `Stream` is the device you wish to communicate over. `station` is the number (or single letter code) of this station, and `depin` is, if provided, the DE/RE pin pair of a MAX485 chip or similar.  The stream device you choose must be manually configured before starting ICSC:

Example:

```C++
#include <ICSC.h>

ICSC icsc(Serial, 'A', 3);

void setup()
{
  Serial.begin(9600);
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
void setup() {
  // init code here
  icsc.registerCommand('T', &tick);
}

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
ICSC::send(unsigned char station, char command, unsigned char len=0, char *data=NULL);
ICSC::send(unsigned char station, char command, char *str);
ICSC::send(unsigned char station, char command, long data);
ICSC::send(unsigned char station, char command, int data);
ICSC::send(unsigned char station, char command, char data);

```

Send a packet to a remote station. The commandID should match a command registered in the remote station. 

The different overloaded methods provide, in turn:

* Sending of a raw data packet of length `len`.
* Sending a NULL-terminated C string.
* Sending a `long` value (32 bits).
* Sending an `int` value (**Caution: the size of this depends on the architecture. It may be 16 or 32 bits.**).
* Sending a single char value (8 bit signed value).

Example:

```C++
icsc.send(6, 'T', 5, (char *)&myData);
```

Send a broadcast
----------------

```C++
ICSC::broadcast(char command, unsigned char len=0, char *data=NULL);
ICSC::broadcast(char command, char *str);
ICSC::broadcast(char command, long data);
ICSC::broadcast(char command, int data);
ICSC::broadcast(char command, char data);
```

This is the same as the `send` method above, except the data is sent to all
stations on the network.  Care should be taken to ensure that the destination
stations do not respond to a broadcast message since that would cause collisions
on the network.

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
