/***************************************************************
* StreamDevice Support                                         *
*                                                              *
* (C) 1999 Dirk Zimoch (zimoch@delta.uni-dortmund.de)          *
* (C) 2007 Dirk Zimoch (dirk.zimoch@psi.ch)                    *
*                                                              *
* This is a custom exponential format converter for            * 
* StreamDevice.                                                *
* The number is represented as two signed integers, mantissa   *
* and exponent, like in +00011-01                              *
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
#include <math.h>

// Exponential Converter %m
// Eric Berryman requested a double format that reads
// +00011-01 as 11e-01
// I.e integer mantissa and exponent without 'e' or '.'
// But why not +11000-04 ?
// For writing, I chose the following convention:
// Format precision defines number of digits in mantissa
// Number of digits in exponent is at least 2
// No leading '0' in mantissa (except for 0.0 of course)
// Format flags +, -, and space are supported in the usual way
// Flags #, 0 are not supported

class ExponentialConverter : public StreamFormatConverter
{
    virtual int parse(const StreamFormat&, StreamBuffer&, const char*&, bool);
    virtual int scanDouble(const StreamFormat&, const char*, double&);
    virtual bool printDouble(const StreamFormat&, StreamBuffer&, double);
};

int ExponentialConverter::
parse(const StreamFormat& fmt, StreamBuffer& info,
    const char*& source, bool scanFormat)
{
    return double_format;
}

int ExponentialConverter::
scanDouble(const StreamFormat& fmt, const char* input, double& value)
{
    int mantissa;
    int exponent;
    int length = -1;
    
    sscanf(input, "%d%d%n", &mantissa, &exponent, &length);
    if (fmt.flags & skip_flag) return length;
    if (length == -1) return -1;
    value = (double)(mantissa) * pow(10, exponent);
    return length;
}

bool ExponentialConverter::
printDouble(const StreamFormat& fmt, StreamBuffer& output, double value)
{
    // Have to divide value into mantissa and exponent
    // precision field is number of characters in mantissa
    // number of characters in exponent is at least 2
    int spaces;
    StreamBuffer buf;
    
    buf.printf("%.*e", fmt.prec-1, fabs(value));
    buf.remove(1,1);
    buf.remove(buf.find('e'),1);
    
    spaces = fmt.width-buf.length();
    if (fmt.flags & (space_flag|sign_flag) || value < 0) spaces--;
    if (spaces < 0) spaces = 0;
    if (!(fmt.flags & left_flag))
        output.append(' ', spaces);
    if (fmt.flags & (space_flag|sign_flag) == space_flag && value >= 0)
        output.append(' ');
    if (fmt.flags & sign_flag && value >= 0)
        output.append('+');
    if (value <= 0)
        output.append('-');
    output.append(buf);
    if (fmt.flags & left_flag)
        output.append(' ', spaces);
    return true;
}

RegisterConverter (ExponentialConverter, "m");

