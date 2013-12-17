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

#ifndef _ICSC_H
#define _ICSC_H

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

// Uncomment the definition of ICSC_DYNAMIC if you want to use
// dynamic memory allocation
//#define ICSC_DYNAMIC

// Uncomment the definition of ICSC_NO_STATS if you don't need stats
// and need to save space
#define ICSC_NO_STATS

// Uncomment the ICSC_COLLISION_DETECTION if collision detection is needed
// On Arduino the transmit and receive are isolated and therefor not needed
//#define ICSC_COLLISION_DETECTION

#ifndef ICSC_DYNAMIC
// This defines the maximum size of any message that can be sent
// between chips.  Reduce this to conserve memory at the cost
// of smaller packet sizes.  Note that this only affects the
// size of packets that can be received - you can always send
// up to 255 bytes.  If the remote end can't receive them all
// the packet will be silently discarded.
#define MAX_MESSAGE 25

// The maximum number of registered commands.  If you're not
// going to have many commands reducing this can save memory.
// If you want lots of commands you may need to increase this
// value.  Note that this ony affects the commands registerable
// at the receiving end
#define MAX_COMMANDS 10
#endif

//If this StationId is used during send(broadcast)
//the message will be picked up by all devices
#define ICSC_BROADCAST  0x00


#define ICSC_CMD_SYS    0x1F

// Shouldn't use anything that would conflict with the header characters.
#define ICSC_SYS_PING   0x05
#define ICSC_SYS_PONG   0x06
#define ICSC_SYS_QSTAT  0x07
#define ICSC_SYS_RSTAT  0x08
//Used when message is relayed to other station via a other station
#define ICSC_SYS_RELAY  0x09


//When this is used during registerCommand all message will pushed
//to the callback function
#define ICSC_CATCH_ALL    0xFF

//The number of SOH to start a message
//some device like Raspberry was missing the first SOH
//Increase or decrease the number to your needs
#define ICSC_SOH_START_COUNT 1

// Format of command callback functions
typedef void(*callbackFunction)(unsigned char, char, unsigned char, char *);

// Structure to store command code / function pairs
typedef struct {
    char commandCode;
    callbackFunction callback;
} command_t, *command_ptr;

#ifndef ICSC_NO_STATS
typedef struct {
    unsigned long oob_bytes;
    unsigned long rx_packets;
    unsigned long rx_bytes;
    unsigned long tx_packets;
    unsigned long tx_bytes;
    unsigned long tx_fail;
    unsigned long cs_errors;
    unsigned long cb_run;
    unsigned long cb_bad;
    unsigned long collision;

} stats_t, *stats_ptr;
#endif

class _ICSC {
    private:
      #ifdef ICSC_DYNAMIC

        // The number of commands registered
        uint8_t _commandCount;

        // The registered commands
        command_ptr _commands;

        // Receiving data buffer
        char* _data;

      #else

        // The registered commands
        command_t _commands[MAX_COMMANDS];

        // Receiving data buffer
        char _data[MAX_MESSAGE];

      #endif

        // Receiving header information
        char _header[6];

        // Reception state machine control and storage variables
        unsigned char _recPhase;
        unsigned char _recPos;
        unsigned char _recCommand;
        unsigned char _recLen;
        unsigned char _recStation;
        unsigned char _recSender;
        unsigned char _recCS;
        unsigned char _recCalcCS;

        // Timestamp of last byte to pass through the
        // fsm.  Used to help avoid colisions.
        unsigned long _lastByteSeen;

        // My station ID
        unsigned char _station;

        // Serial device in use
        HardwareSerial *_hserial;
#if defined(_USE_USB_FOR_SERIAL_)
        USBSerial *_userial;
#else
        void *_userial;
#endif
        int _dePin;
        unsigned long _baud;

     #ifndef ICSC_NO_STATS
        // Statistics gathering
        stats_t _stats;
     #endif

        void assertDE();
        void waitForTransmitToComplete();
        void deassertDE();

    protected:
        void reset();
        void respondToPing(unsigned char station, char command, unsigned char len, char *data);
      #ifndef ICSC_NO_STATS
        void respondToQSTAT(unsigned char station, char command, unsigned char len, char *data);
      #endif

    public:
        _ICSC();
        ~_ICSC();
        void begin(unsigned char station, unsigned long baud=9600);
        void begin(unsigned char station, unsigned long baud, int dePin);
#if defined(_USE_USB_FOR_SERIAL_)
        void begin(unsigned char station, unsigned long baud, USBSerial *sdev);
        void begin(unsigned char station, unsigned long baud, USBSerial *sdev, int dePin);
#endif
        void begin(unsigned char station, unsigned long baud, HardwareSerial *sdev);
        void begin(unsigned char station, unsigned long baud, HardwareSerial *sdev, int dePin);
        boolean send(unsigned char origin,unsigned char station, char command, unsigned char len=0, char *data=NULL);
        boolean send(unsigned char station, char command, unsigned char len=0, char *data=NULL);
        boolean send(unsigned char station, char command,char *str);
        boolean send(unsigned char station, char command, long data);
        boolean send(unsigned char station, char command, int data);
        boolean send(unsigned char station, char command, char data);
        boolean broadcast(char command, unsigned char len=0, char *data=NULL);
        boolean broadcast(char command, char *str);
        boolean broadcast(char command, long data);
        boolean broadcast(char command, int data);
        boolean broadcast(char command, char data);
        boolean process();
        void registerCommand(char command, callbackFunction func);
        void unregisterCommand(char command);
      #ifndef ICSC_NO_STATS
        stats_ptr stats();
      #endif
        boolean isBroadCast();
        boolean isRelay();

    private:
        void serialWrite(unsigned char b);
        int serialRead();
        int serialAvailable();
        void serialFlush();

};

// Packet wrapping characters, defined in standard ASCII table
#define SOH 1
#define STX 2
#define ETX 3
#define EOT 4

// Global object for interacting with the class
extern _ICSC ICSC;

#endif
