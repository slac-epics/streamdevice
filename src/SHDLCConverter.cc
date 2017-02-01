/***************************************************************
* Sensirion High-Level Data Link Control (SHDLC) Format Conv.  *
*  for use with EPICS streamdevice                             *
*                                                              *
* Author: Karl Gumerlock (klg@slac.stanford.edu)               *
*                                                              *
* SHDLC is spoken by various Sensirion devices, and is a       *
* packet-based binary communication protocol that does not     *
* lend itself well to the usual streamdevice protocol          *
* definition.                                                  *
*                                                              *
* Version 0.1 2014-07-09                                       *
*                                                              *
* Licensed under Creative Commons CC BY-SA 4.0                 *
* http://creativecommons.org/licenses/by-sa/4.0/               * 
***************************************************************/

#include <ctype.h>
#include "StreamFormatConverter.h"
#include "StreamError.h"

// SHDLC Converter %Z

bool appendStuffed ( StreamBuffer& output, unsigned char outByte )
{
    switch ( outByte )
    {
        case 0x7E:
            output.append(0x7D);
            output.append(0x5E);
            break;
        case 0x7D:
            output.append(0x7D);
            output.append(0x5D);
            break;
        case 0x11:
            output.append(0x7D);
            output.append(0x31);
            break;
        case 0x13:
            output.append(0x7D);
            output.append(0x33);
            break;
        default:
            output.append(outByte);
            break;
    }

    return true;
}

unsigned char unStuffAndInc ( const char* input, int& index, 
    int& bytesconsumed )
{
    unsigned char inByte;

    inByte = (unsigned char)(input[index]);

    if ( inByte == 0x7D )
    {
        inByte = (unsigned char)(input[++index]);
        bytesconsumed++;

        switch ( inByte )
        {
            case 0x5E:
                index++;
                bytesconsumed++;
                return 0x7E;
                break;
            case 0x5D:
                index++;
                bytesconsumed++; 
                return 0x7D;
                break;
            case 0x31:
                index++;
                bytesconsumed++;
                return 0x11;
                break;
            case 0x33:
                index++;
                bytesconsumed++;
                return 0x13;
                break;
        }
    }
    else
    {
        index++;
        bytesconsumed++;
        return inByte;
    }
}

class SHDLCConverter : public StreamFormatConverter
{
    int parse(const StreamFormat&, StreamBuffer&, const char*&, bool);
    bool printString(const StreamFormat&, StreamBuffer&, const char*);
    int scanString(const StreamFormat&, const char*, char*, size_t);
    bool printLong(const StreamFormat&, StreamBuffer&, long);
    int scanLong(const StreamFormat&, const char*, long&);
};

int SHDLCConverter::
parse(const StreamFormat& format, StreamBuffer& info,
    const char*& source, bool scanFormat)
{
    // Here we take advantage of the pre-parsed format flags
    // to decide exactly what to do.
    // Available flags are:
    //  * left_flag: '-' pre-pended signifies string data
    //  * sign_flag: '+' pre-pended signifies signed translation
    //                   --> not handled in parse
    //  * space_flag
    //  * alt_flag
    //  * zero_flag
    //  * skip_flag
    
    if ( format.flags & left_flag )
    {
        return string_format;
    } 
    else 
    {
        return unsigned_format; 
    }
}

bool SHDLCConverter::
printString(const StreamFormat& format, StreamBuffer& output, 
    const char* value)
{
    // OBSOLETE DO NOT USE
    error("SHDLC ERROR: printString unimplemented.\n");
    return true;
}

bool SHDLCConverter::
printLong(const StreamFormat& format, StreamBuffer& output,
    long value)
{
    unsigned char checksum=0, outByte, dataarray[8];
    int datalen, i;
    
    // BYTE 0: Start delimiter 0x7E
    output.append(0x7E);

    // BYTE 1: ADDRESS byte (0)
    appendStuffed(output, 0x00);
    checksum += 0x00;

    // BYTE 2: COMMAND byte
    outByte = (unsigned char)(format.width);
    appendStuffed(output, outByte);
    checksum += outByte;

    // BYTE 3: LENGTH byte
    outByte = (unsigned char)(format.prec);
    appendStuffed(output, outByte);
    checksum += outByte;

    // BYTE 4 to n-3: DATA bytes
    datalen = format.prec;
    
    // Only transmit data if there is any data (duh)
    if ( datalen > 0 )
    {
        for ( i=0; i<datalen; i++ )
        {
            // Decompose the long value by byte, LSB first
            dataarray[i] = (unsigned char)(value & 0xFF);
            value = value >> 8;
        }
        for ( i=(datalen-1); i>=0; i-- )
        {
            // Append it to the output MSB first
            appendStuffed(output, dataarray[i]);
            checksum += dataarray[i];
        }
    }
    
    // BYTE n-2: CHECKSUM byte
    appendStuffed(output, ~(checksum & 0xFF));

    // BYTE n-1: END byte (delimiter)
    output.append(0x7E);

    return true;
}

int SHDLCConverter::
scanString(const StreamFormat& format, const char* input,
    char* value, size_t maxlen)
{
    return true;
}

int SHDLCConverter::
scanLong(const StreamFormat& format, const char* input,
    long& value)
{
    // Data types to hold up to max. 64 bit (8 byte) longs
    int index=0, bytesconsumed=0, datalen, i;
    unsigned char inByte, checksum=0;
    unsigned long long valuetemp=0;

    // BYTE 0: Check for presence of delimiter
    inByte = (unsigned char)(input[0]);
    if ( inByte == 0x7E )
    {
        index++;
        bytesconsumed++;
        // BYTE 1: ADDRESS byte
        inByte = unStuffAndInc(input, index, bytesconsumed);
        checksum += inByte;
    } 
    else
    {
        // Not the start of a packet; fail?
        //return -1;

        // Let's assume we've got the usual framing error; we've lost
        // the first two bytes, which are the 0x7E delimiter and 
        // the address byte, which is 0 and we don't care about.
        // let's try to save this shit.
        // Even the checksum will be OK because it's not affected
        // by an address byte of 0x00.
    }

    // BYTE 2: COMMAND byte
    inByte = unStuffAndInc(input, index, bytesconsumed);
    checksum += inByte;

    // Check COMMAND byte against expected response (from protocol)
    if(inByte != (unsigned char)(format.width))
    {
        error( "SHDLC ERROR: COMMAND byte, expected %#02X, got %#02X\n",
            (unsigned char)(format.width), inByte);
        return -1;
    }
    
    // BYTE 3: STATE byte
    inByte = unStuffAndInc(input, index, bytesconsumed);
    checksum += inByte;

    // Check STATE byte - anything besides 0 is not a proper response
    if(inByte != 0)
    {
        error( "SHDLC ERROR: STATE byte, expected 0, got %#02X\n", 
            inByte);
        return -1;
    }

    // BYTE 4: LENGTH byte
    inByte = unStuffAndInc(input, index, bytesconsumed);
    checksum += inByte;
    datalen = inByte;

    // Check STATE byte against expected length (from protocol)
    if(datalen != format.prec)
    {
        error( "SHDLC ERROR: LENGTH byte, expected %#02X, got %#02X\n",
            (unsigned char)(format.prec), inByte);
        return -1;
    }

    // BYTES 5 to n-3: DATA (there might also be *no* data)
    for(i=0; i<datalen; i++)
    {
        // Read out all data bytes; reconstruct valuetemp, MSB first
        valuetemp = valuetemp << 8;
        inByte = unStuffAndInc(input, index, bytesconsumed);
        checksum += inByte;
        valuetemp += inByte;
    }
    
    // BYTE n-2: CHECKSUM byte
    inByte = unStuffAndInc(input, index, bytesconsumed);

    // Check CHECKSUM LSbyte against computed checksum
    if(inByte != (unsigned char)(~(checksum & 0xFF)))
    {
        error("SHDLC ERROR: CHECKSUM byte, calculated %#02X,\
                received %#02X\n", inByte, ~(checksum & 0xFF));
        return -1;
    }

    // BYTE n-1: END byte (delimiter)
    inByte = input[index];
    bytesconsumed++;

    // Check that we're at the end
    if(inByte != 0x7E)
    {
        error("SHDLC ERROR: END delimiter mismatch, expected 0x7E,\
            received %#02X\n", inByte);
        return -1;
    }

    // At this point, we have all of the data inside valuetemp
    // need to translate it and store it in value
    if(format.flags & sign_flag)
    {
        // sign_flag being set denotes signed output is desired
        switch(datalen)
        {
            case 1:
                value = (signed char)(valuetemp & 0xFF);
                break;
            case 2:
                value = (signed short)(valuetemp & 0xFFFF);
                break;
            case 3:
                value = (signed long)(valuetemp & 0xFFFFFFFF);
                break;
            case 4:
                value = (signed long long)(valuetemp);
                break;
            default:
                value = 0;
                break;
        }

    } 
    else 
    {
        // No sign_flat means output unsigned, although we are SOL
        // if we have a >2^63 bit integer.
        switch(datalen)
        {
            case 1:
                value = (valuetemp & 0xFF);
                break;
            case 2:
                value = (valuetemp & 0xFFFF);
                break;
            case 3:
                value = (valuetemp & 0xFFFFFFFF);
                break;
            case 4:
                value = (valuetemp);
                break;
            default:
                value = 0;
                break;
        }
    }

    // Number of bytes consumed is datalen + 7
    return bytesconsumed;
}

RegisterConverter (SHDLCConverter, "Z");
