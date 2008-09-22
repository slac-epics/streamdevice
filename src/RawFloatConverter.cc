/***************************************************************
* StreamDevice Support                                         *
*                                                              *
* (C) 1999 Dirk Zimoch (zimoch@delta.uni-dortmund.de)          *
* (C) 2005 Dirk Zimoch (dirk.zimoch@psi.ch)                    *
*                                                              *
* This is the raw format converter of StreamDevice.            *
* Please refer to the HTML files in ../doc/ for a detailed     *
* documentation.                                               *
*                                                              *
* If you do any changes in this file, you are not allowed to   *
* redistribute it any more. If there is a bug or a missing     *
* feature, send me an email and/or your patch. If I accept     *
* your changes, they will go to the next release.              *
*                                                              *
* DISCLAIMER: If this software breaks something or harms       *
* someone, it's your problem.                                  *
*                                                              *
***************************************************************/

#include "StreamFormatConverter.h"
#include "StreamError.h"


// Let's use epicsEndian.h
#include "epicsEndian.h"


#ifndef EPICS_FLOAT_WORD_ORDER
#error define EPICS_FLOAT_WORD_ORDER as EPICS_ENDIAN_LITTLE or EPICS_ENDIAN_BIG 
#endif

#if (EPICS_FLOAT_WORD_ORDER == EPICS_ENDIAN_LITTLE || EPICS_FLOAT_WORD_ORDER == EPICS_ENDIAN_BIG)




// Raw Float Converter %R

class RawFloatConverter : public StreamFormatConverter
{
    int parse(const StreamFormat&, StreamBuffer&, const char*&, bool);
    bool printDouble(const StreamFormat&, StreamBuffer&, double);
    int scanDouble(const StreamFormat&, const char*, double&);
};

int RawFloatConverter::
parse(const StreamFormat& format, StreamBuffer&,
    const char*&, bool)
{
    // Assume IEEE formats with 4 or 8 bytes (default: 4)
    if (format.width==0 || format.width==4 || format.width==8)
        return double_format;
    error ("Only width 4 or 8 allowed for %%R format.\n");
    return false;
}

bool RawFloatConverter::
printDouble(const StreamFormat& format, StreamBuffer& output, double value)
{
    int nbOfBytes;
    int n;
    union {
        double dval;
        float  fval;
        char   bytes[8];
    } buffer;
    
    nbOfBytes = format.width;
    if (nbOfBytes == 0)
        nbOfBytes = 4;

    if (nbOfBytes == 4)
        buffer.fval = value;
    else 
        buffer.dval = value;

#if (EPICS_FLOAT_WORD_ORDER == EPICS_ENDIAN_BIG) 
    bool swap = format.flags & alt_flag;
#else
    bool swap = !(format.flags & alt_flag);
#endif
    
    if (swap)
    {
    	for (n = nbOfBytes-1; n >= 0; n--)
    	{
            output.append(buffer.bytes[n]);
    	}
    } else {
    	for (n = 0; n < nbOfBytes; n++)
    	{
            output.append(buffer.bytes[n]);
    	}
    } 
    return true;
}

int RawFloatConverter::
scanDouble(const StreamFormat& format, const char* input, double& value)
{
    int nbOfBytes;
    int i, n;

    union {
        double dval;
        float  fval;
        char   bytes[8];
    } buffer;
    
    nbOfBytes = format.width;
    if (nbOfBytes == 0)
        nbOfBytes = 4;

    if (format.flags & skip_flag)
    {
        return(nbOfBytes); // just skip input
    }
    
#if (EPICS_FLOAT_WORD_ORDER == EPICS_ENDIAN_BIG) 
    bool swap = format.flags & alt_flag;
#else
    bool swap = !(format.flags & alt_flag);
#endif

    if (swap)
    {
    	for (n = nbOfBytes-1, i = 0; n >= 0; n--, i++)
    	{
            buffer.bytes[n] = input[i];
    	}
    } else {
    	for (n = 0; n < nbOfBytes; n++)
    	{
            buffer.bytes[n] = input[n];
    	}
    }

    if (nbOfBytes == 4)
        value = buffer.fval;
    else 
        value = buffer.dval;

    return nbOfBytes;
}

RegisterConverter (RawFloatConverter, "R");

#endif /* known byte order */
