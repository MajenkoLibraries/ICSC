/********************************************************************************
 * Copyright (c) 2013, Majenko Technologies and S.J.Hoeksma
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of Majenko Technologies.
 ********************************************************************************/

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <ICSC.h>

//Destructor of the class
ICSC::~ICSC(){
 #ifdef ICSC_DYNAMIC
   free(_data);
   free(_commands);
 #endif
}

// Initialize the system.  Set up the serial port to the right baud rate
// and initialize my variables.
void ICSC::begin()
{
#ifndef ICSC_NO_STATS
    _stats.oob_bytes = 0;
    _stats.tx_packets = 0;
    _stats.tx_bytes = 0;
    _stats.rx_packets = 0;
    _stats.rx_bytes = 0;
    _stats.cs_errors = 0;
    _stats.cb_run = 0;
    _stats.cb_bad = 0;
#endif

    // Reset the state machine
    reset();

    if (_dePin != -1) {
        pinMode(_dePin, OUTPUT);
        digitalWrite(_dePin, LOW);
    }
}

// Send a message to a remote station. Origin is the originating station
//  "station" is the ID assigned to the remote station, "command" is a
// command code that has been programmed into the remote station.
//  "len" is the number of bytes to send in the message (if a
// string is being sent, remember to include the terminating 0).  "data" is
// a pointer (cast to a char *) to the data to send.

// The checksum is calculated as the sum of all the variable content of the packet
// modulus 256.  I.e., everything except the framing characters SOH/STX/ETX/EOT and
// the checksum itself.
boolean ICSC::send(unsigned char origin,unsigned char station, char command, unsigned char len, char *data)
{
    unsigned char i;
    unsigned char cs = 0;
    unsigned char del;

    #ifdef ICSC_COLLISION_DETECTION
      // This is how many times to try and transmit before failing.
      unsigned char timeout = 10;

      // Let's start out by looking for a colision.  If there has been anything seen in
      // the last millisecond, then wait for a random time and check again.

      while (process()) {
        del = rand() % 20;
        for (i=0; i<del; i++) {
            delay(1);
          #ifndef ICSC_NO_STATS
            _stats.collision++;
          #endif
            process();
        }
        timeout--;
        if (timeout == 0) {
            // Failed to transmit!!!
          #ifndef ICSC_NO_STATS
            _stats.tx_fail++;
          #endif
            return false;
        }
      }
    #endif

    assertDE();
     // Start of header by writing multiple SOH
    for(byte w=0;w<ICSC_SOH_START_COUNT;w++)  _dev->write(SOH);
    _dev->write(station);  // Destination address
    cs += station;
    _dev->write(origin); // Source address
    cs += _station;
    _dev->write(command);  // Command code
    cs += command;
    _dev->write(len);      // Length of text
    cs += len;
    _dev->write(STX);      // Start of text
    for(i=0; i<len; i++) {
        _dev->write(data[i]);      // Text bytes
        cs += data[i];
    }
    _dev->write(ETX);      // End of text
    _dev->write(cs);
    _dev->write(EOT);
    waitForTransmitToComplete();
    deassertDE();
#ifndef ICSC_NO_STATS
    _stats.tx_packets++;
    _stats.tx_bytes += len;
#endif
    return true;
}

//Mostly used send, we use our internal station as origin
boolean ICSC::send(unsigned char station, char command, unsigned char len, char *data)
{
  return send(_station,station,command,len,data);
}

// Send a string of data to a remote station.
boolean ICSC::send(unsigned char station, char command,char *str)
{
  return send(station,command,(unsigned char)strlen(str),str);
}

//Wrapper to send a long
boolean ICSC::send(unsigned char station, char command, long data)
{
  return send(station,command,sizeof(data),(char *)&data);
}

//Wrapper to send a int
boolean ICSC::send(unsigned char station, char command, int data)
{
  return send(station,command,sizeof(data),(char *)&data);
}

//Wrapper to send a char
boolean ICSC::send(unsigned char station, char command, char data)
{
  return send(station,command,sizeof(data),(char *)&data);
}

//Broadcast data to all stations
boolean ICSC::broadcast(char command, unsigned char len, char *data)
{
  return send(ICSC_BROADCAST,command,len,data);
}

//BroadCast a string to all Stations
boolean ICSC::broadcast(char command, char *str)
{
  return send(ICSC_BROADCAST,command,(unsigned char)strlen(str),str);
}

//Wrapper for broadcasting a long
boolean ICSC::broadcast(char command, long data)
{
  return send(ICSC_BROADCAST,command,sizeof(data),(char *)&data);
}

//Wrapper for broadcasting a int
boolean ICSC::broadcast(char command, int data)
{
  return send(ICSC_BROADCAST,command,sizeof(data),(char *)&data);
}

//Wrapper for broadcasting a single char
boolean ICSC::broadcast(char command, char data)
{
  return send(ICSC_BROADCAST,command,sizeof(data),(char *)&data);
}

//Reset the state machine and release the data pointer
void ICSC::reset(){
  _recPhase = 0;
  _recPos = 0;
  _recLen = 0;
  _recCommand = 0;
  _recCS = 0;
  _recCalcCS = 0;
#ifdef ICSC_DYNAMIC
  free(_data);
  _data=NULL;
#endif
}

// This is the main reception state machine.  Progress through the states
// is keyed on either special control characters, or counted number of bytes
// received.  If all the data is in the right format, and the calculated
// checksum matches the received checksum, AND the destination station is
// our station ID, then look for a registered command that matches the
// command code.  If all the above is true, execute the command's
// function.
boolean ICSC::process()
{
    char inch;
    unsigned char i;
    unsigned char cbok = 0;
    if (!_dev->available()) return false;

    while(_dev->available()) {
        inch = _dev->read();

        // Record the timestamp of this byte, so that we know how long since we last
        // saw any activity on the line
        _lastByteSeen = millis();

        switch(_recPhase) {

            // Case 0 looks for the header.  Bytes arrive in the serial interface and get
            // shifted through a header buffer.  When the start and end characters in
            // the buffer match the SOH/STX pair, and the destination station ID matches
            // our ID, save the header information and progress to the next state.
            case 0:
                memcpy(&_header[0],&_header[1],5);
                _header[5] = inch;
                if ((_header[0] == SOH) && (_header[5] == STX) && (_header[1] != _header[2])) {
                    _recCalcCS = 0;
                    _recStation = _header[1];
                    _recSender = _header[2];
                    _recCommand = _header[3];
                    _recLen = _header[4];

                    for (i=1; i<=4; i++) {
                        _recCalcCS += _header[i];
                    }
                    _recPhase = 1;
                    _recPos = 0;

                    //Check if we should process this message
                    //We reject the message if we are the sender
                    //We reject if we are not the receiver and message is not a broadcast
                    if ((_recSender == _station) ||
                        (_recStation != _station &&
                         _recStation != ICSC_BROADCAST &&
                         _recStation != ICSC_SYS_RELAY )) {
                        reset();
                        break;
                    }

                    if (_recLen == 0) {
                        _recPhase = 2;
                    }

                #ifdef ICSC_DYNAMIC
                    else {
                       _data = (char *)malloc(_recLen);
                    }
                 #else
                    if (_recLen > MAX_MESSAGE) {
                       _recPhase = 0;
                    }
                #endif

              #ifndef ICSC_NO_STATS
                } else {

                    _stats.oob_bytes++;
              #endif
                }
                break;

            // Case 1 receives the data portion of the packet.  Read in "_recLen" number
            // of bytes and store them in the _data array.
            case 1:
                _data[_recPos++] = inch;
                _recCalcCS += inch;
                if (_recPos == _recLen) {
                    _recPhase = 2;
                }
                break;

            // After the data comes a single ETX character.  Do we have it?  If not,
            // reset the state machine to default and start looking for a new header.
            case 2:
                // Packet properly terminated?
                if (inch == ETX) {
                    _recPhase = 3;
                } else {
                    reset();
                }
                break;

            // Next comes the checksum.  We have already calculated it from the incoming
            // data, so just store the incoming checksum byte for later.
            case 3:
                _recCS = inch;
                _recPhase = 4;
                break;

            // The final state - check the last character is EOT and that the checksum matches.
            // If that test passes, then look for a valid command callback to execute.
            // Execute it if found.
            case 4:
                cbok = 0;
                if (inch == EOT) {
                    if (_recCS == _recCalcCS) {


                        // First, check for system level commands.  It is possible
                        // to register your own callback as well for system level
                        // commands which will be called after the system default
                        // hook.

                        switch (_recCommand) {
                            case ICSC_SYS_PING:
                                respondToPing(_recSender, _recCommand, _recLen, _data);
                                break;
                          #ifndef ICSC_NO_STATS
                            case ICSC_SYS_QSTAT:
                                respondToQSTAT(_recSender, _recCommand, _recLen, _data);
                                break;
                          #endif
                        }


                      #ifdef ICSC_DYNAMIC
                        for (i=0; i<_commandCount; i++) {
                      #else
                        for (i=0; i<MAX_COMMANDS; i++) {
                      #endif
                            if ((_commands[i].commandCode == _recCommand ||
                                 _commands[i].commandCode == ICSC_CATCH_ALL )
                               && _commands[i].callback)
                            {
                                _commands[i].callback(_recSender, _recCommand, _recLen, _data);
                                #ifndef ICSC_NO_STATS
                                _stats.cb_run++;
                                #endif
                                cbok = 1;
                            }
                        }
               #ifndef ICSC_NO_STATS
                        if (cbok == 0) {
                            _stats.cb_bad++;
                        }
                        _stats.rx_packets++;
                        _stats.rx_bytes += _recLen;
                    } else {
                        _stats.cs_errors++;
                    }
                #else
                    }
                #endif
                }
                //Clear the data
                reset();
                //Return true, we have processed one command
                return true;
                break;
        }
    }
    return true;
}

// Add a new command code / function pair into the list of
// registered commands.  If there is no room, fail silently.
void ICSC::registerCommand(char command, callbackFunction func)
{
    unsigned char i;

#ifdef ICSC_DYNAMIC

    // Check if we should update the function
    for (i=0; i<_commandCount; i++) {
        if (_commands[i].commandCode == command) {
            _commands[i].callback = func;
            return;
        }
    }

    // Not Found add a new command
    _commandCount++;
    _commands = (command_ptr)realloc(_commands,_commandCount * sizeof(command_t));
    _commands[i].commandCode=command;
    _commands[i].callback = func;

#else

    for (i=0; i<MAX_COMMANDS; i++) {
        if (_commands[i].commandCode == 0) {
            _commands[i].commandCode = command;
            _commands[i].callback = func;
            return;
        }
    }

#endif
}

// Look for a registered command and delete it from the
// list if found.  If not found, silently fail.
void ICSC::unregisterCommand(char command)
{
    unsigned char i;

#ifdef ICSC_DYNAMIC

    for (i=0; i<_commandCount; i++) {
        if (_commands[i].commandCode == command) {
            _commandCount--;
            memcpy(&_commands[i],&_commands[i+1],(_commandCount-i) * sizeof(command_t));
            _commands=(command_ptr) realloc(_commands,_commandCount * sizeof(command_t));
            return;
        }
    }

#else

    for (i=0; i<MAX_COMMANDS; i++) {
        if (_commands[i].commandCode == command) {
            _commands[i].commandCode = 0;
            _commands[i].callback = NULL;
            return;
        }
    }

#endif
}

// This is a bit of a fudge at the moment as many different versions
// of the different core code from the different chips provides
// different facilities for knowing if the transmission has
// completed.
void ICSC::waitForTransmitToComplete()
{
  if (_dePin == -1)  //Skip flush
    return;

#ifdef __PIC32MX__
    // MPIDE has nothing yet for this.  It uses the hardware buffer, which
    // could be up to 8 levels deep.  For now, let's just delay for 8
    // characters worth.
    delayMicroseconds((F_CPU/9600)+1);
#else
#if defined(ARDUINO) && ARDUINO >= 100
#if ARDUINO >= 104
    // Arduino 1.0.4 and upwards does it right
    _dev->flush();
#else
    // Between 1.0.0 and 1.0.3 it almost does it - need to compensate
    // for the hardware buffer. Delay for 2 bytes worth of transmission.
    _dev->flush();
    delayMicroseconds((20000000UL/9600)+1);
#endif
#endif
#endif
}


// Control the DE line, if a DEPin has been specified.  This will
// always put the DEPin into output mode in case something else has
// tampered with the pin setting in the mean time.
void ICSC::assertDE()
{
    if (_dePin != -1) {
        digitalWrite(_dePin, HIGH);
        delayMicroseconds(5);
    }
}

void ICSC::deassertDE()
{
    if (_dePin != -1) {
        digitalWrite(_dePin, LOW);
    }
}

// Statistics.  Just return a pointer to the internal stats structure.
#ifndef ICSC_NO_STATS
stats_ptr ICSC::stats()
{
    return &_stats;
}
#endif

// System level command responders

// Respond to a PING packet with a PONG packet.  Send back whatever data was received.
void ICSC::respondToPing(unsigned char station, char command, unsigned char len, char *data)
{
    send(station, ICSC_SYS_PONG, len, data);
}

// Stat query request - respond with the current stats structure
#ifndef ICSC_NO_STATS
void ICSC::respondToQSTAT(unsigned char station, char command, unsigned char len, char *data)
{
    send(station, ICSC_SYS_RSTAT, sizeof(stats_t), (char *)&_stats);
}
#endif

//function which can be used to validate during a callback
//that the message comes from a BroadCast function
boolean ICSC::isBroadCast()
{
  return _recStation==ICSC_BROADCAST;
}


//A function which can be used to validate during a callback
//that the message comes from a relay function
boolean ICSC::isRelay()
{
  return _recStation==ICSC_SYS_RELAY;
}

