/* ISO_HEADER_START */

/* 
 * Copyright (C) 1997, ISO/IEC
 * 
 * The software module contained in this file was originally developed by
 * Prof. Alexandros Eleftheriadis and Yihan Fang (Columbia University) as part
 * of the Flavor run-time library (see http://www.ee.columbia.edu/flavor).
 * Copyright has been transferred to ISO/IEC for inclusion in the MPEG-4
 * Reference Software (ISO/IEC 14496-5) distribution. ISO/IEC gives users of
 * this code free license to this software or modifications thereof for use in
 * hardware or software products claiming conformance to one or more parts of
 * the MPEG-4 (ISO/IEC 14496) series of specifications. Those intending to use
 * this Reference Software in hardware or software products are advised that
 * its use may infringe existing patents. The original developer of this
 * software module and his company, the subsequent editors and their
 * companies, and ISO/IEC have no liability for use of this software or
 * modifications thereof in an implementation. Copyright is not released for
 * imlementations that do not comply to one or more parts of the MPEG-4
 * (ISO/IEC 14496) series of specifications. Columbia University retains full
 * right to use the code for its own purpose, including assignment or
 * donatation of this code to a third party.
 * 
 * This copyright notice must be included in all copies or derivative works.
 * 
 * For more information or to receive updated versions of this module, contact
 * Prof. Alexandros Eleftheriadis at eleft@ee.columbia.edu.
 */

/* ISO_HEADER_END */

/* bitstream.cpp: low-level routines for parsing bitstream data */

// bitstream IO implementation

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "port.h"
#include "bitstream.h"

// masks for bitstring manipulation
static const unsigned int mask[33] = {
    0x00000000, 0x00000001, 0x00000003, 0x00000007,
    0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
    0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
    0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
    0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,
    0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
    0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,
    0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff,
    0xffffffff
};

// file-based constructor
Bitstream::Bitstream(const char *filename, Bitstream_t t)
{
    type=t;
    close_fd=1;
    cur_bit=0;
    tot_bits=0;
    end=0;
    buf_len=BS_BUF_LEN;
    err_code=E_NONE;
    
    // copy file name -- only if it fits
    if (strlen(filename)<sizeof(fn))
        strcpy(fn, filename);
    
    memset(buf, 0, BS_BUF_LEN);
    
    switch (type) {
        
    case BS_INPUT:
        if ((fd=open(fn, O_RDONLY))<0) {
            fprintf(stderr, "Bitstream::Bitstream: cannot open file '%s'\n", fn);
            exit(1);
        }
#ifdef WIN32		
        // switch to binary mode
        setmode(fd, O_BINARY);
#endif
        cur_bit=BS_BUF_LEN<<3;	// fake we are at the end of buffer
        fill_buf();
        break;
        
    case BS_OUTPUT:
        if ((fd=open(fn, O_WRONLY|O_CREAT|O_TRUNC, 0644))<0) {
            fprintf(stderr, "Bitstream::Bitstream: cannot create file '%s'\n", fn);
            exit(1);
        }
#ifdef WIN32		
        // switch to binary mode
        setmode(fd, O_BINARY);
#endif
        break;
        
    default:
        fprintf(stderr, "Bitstream::Bitstream: unknown bitstream type %d\n", t);
        exit(1);
        break;
    }
}

// file descriptor-based constructor
Bitstream::Bitstream(int fd, Bitstream_t t)
{
    type=t;
    close_fd=0;
    cur_bit=0;
    tot_bits=0;
    buf_len=BS_BUF_LEN;
    memset(buf, 0, BS_BUF_LEN);
    this->fd=fd;
    end=0;
    err_code=E_NONE;
    
    switch (type){
        
    case BS_INPUT:
        cur_bit = BS_BUF_LEN<<3;	// fake we are at the end of buffer
        fill_buf();
        break;
        
    case BS_OUTPUT:
        break;
        
    default:
        fprintf(stderr, "Bitstream::Bitstream: unknown bitstream type %d\n", t);
        exit(1);
        break;
    }
#ifdef WIN32		
    // switch to binary mode
    setmode(fd, O_BINARY);
#endif
}

Bitstream::~Bitstream(void)
{  
    // make sure all data is out
    if (type==BS_OUTPUT)
        flushbits();
    if (close_fd)
        close(fd); 
}

// convert error code to text message
char const *
Bitstream::err2msg(Error_t code)
{
    switch (code) {
        
    case E_NONE:			return "<none>";
    case E_END_OF_DATA: 		return "End of data";
    case E_INVALID_ALIGNMENT:	return "Invalid alignment";
    case E_READ_FAILED: 		return "Read failed";
    case E_WRITE_FAILED: 		return "Write failed";
    default: 			return "Unknown error";
        
    }
}


// returns 'n' bits as unsigned int; advances bit pointer
unsigned int
Bitstream::getbits(int n)
{
    register unsigned int x;    // the value we will return
    unsigned char *v;           // the byte where cur_bit points to
    register int s;             // number of bits to shift 
    
    // make sure we have enough data
    if (cur_bit + n > (buf_len<<3))
        fill_buf();
    
	if (err_code != E_END_OF_DATA) {
		// starting byte in buffer
		v = buf + (cur_bit >> 3);
		
		// load 4 bytes - this way endianess is automatically taken care of
		x = (v[0]<<24) | (v[1]<<16) | (v[2]<<8) | v[3];
		
		// figure out how much shifting is required
		s = 32 - ((cur_bit % 8) + n);
		
		if (s >= 0) {         // need right adjust 
			x = (x >> s);
		} else {		// shift left and read an extra byte
			x = x << -s;
			x |= v[4] >> (8+s);
		}
		
		cur_bit += n;
		tot_bits += n;
		
	//	printf("Got %d bits: %x\n",n,x & mask[n]);
		return (x & mask[n]);
	}
	return 0;
}

// returns 'n' bits as unsigned int; does not advance bit pointer
unsigned int
Bitstream::nextbits(int n)
{
    register unsigned int x;    // the value we will return
    unsigned char *v;           // the byte where cur_bit points to
    register int s;             // number of bits to shift 
    
    // make sure we have enough data
    if (cur_bit + n > (buf_len<<3))
        fill_buf();
    
    // starting byte in buffer
    v = buf + (cur_bit >> 3);
    
    // load 4 bytes - this way endianess is automatically taken care of
    x = (v[0]<<24) | (v[1]<<16) | (v[2]<<8) | v[3];
    
    // figure out how much shifting is required
    s = 32 - ((cur_bit % 8) + n);
    
    if (s >= 0) {         // need right adjust 
        x = (x >> s);
    } else {		// shift left and read an extra byte
        x = x << -s;
        x |= v[4] >> (8+s);
    }
    
    // no change in cur_bit or tot_bits
    
    return (x & mask[n]);
}

// advance by some bits ignoring the value
void
Bitstream::skipbits(int n)
{
    // make sure we have enough data
    if (cur_bit + n > (buf_len<<3))
        if (type==BS_INPUT)
            fill_buf();
        else
            flush_buf();
        cur_bit += n;
        tot_bits += n;
        return;
}

// can only write at least one byte to a file at a time
// returns the output value
int 
Bitstream::putbits(unsigned int bits, int n)
{
    int delta;		    // required input shift amount
    unsigned char *v;	// current byte
    unsigned int tmp;	// temp value for shifted bits
    
    if (cur_bit + n > (buf_len<<3))
        flush_buf();
    
    delta=32-n-(cur_bit%8);
    v=buf+(cur_bit>>3);
    
    if (delta>=0) {
        tmp = bits<<delta;
        v[0] |= tmp>>24;
        v[1] |= tmp>>16;
        v[2] |= tmp>>8;
        v[3] |= tmp;
    } else {
        tmp = bits>>(-delta); // -delta<8
        v[0] |= tmp>>24;
        v[1] |= tmp>>16;
        v[2] |= tmp>>8;
        v[3] |= tmp;
        v[4] |= bits<<(8+delta);
    }
    
    cur_bit += n;
    tot_bits += n;
	// printf("Put %d bits: %x.\n",n,bits);
    return bits;
}

// get a float
float
Bitstream::getfloat(void)
{
    float f;
    
    *(int *)&f=getbits(32);
    return f;
}

// probe a float
float
Bitstream::nextfloat(void)
{
    float f;
    
    *(int *)&f=nextbits(32);
    return f;
}

// put a float
float
Bitstream::putfloat(float f)
{
    putbits(*((int *)&f), 32);
    return f;
}

// get a double
double
Bitstream::getdouble(void)
{
    double d;
    
    *(int *)&d=getbits(32);
    *(((int *)&d)+1)=getbits(32);
    return d;
}

// probe a double
double
Bitstream::nextdouble(void)
{
    double d;
    
    // make sure we have enough data (so that we can go back)
    if (cur_bit + 64 > (buf_len<<3))
        fill_buf();
    
    *(int *)&d=nextbits(32);
    cur_bit+=32;
    *(((int *)&d)+1)=nextbits(32);
    cur_bit-=32;
    return d;
}

// put a double 
double
Bitstream::putdouble(double d)
{
    putbits(*(int *)&d, 32);
    putbits(*(((int *)&d)+1), 32);
    return d;
}


// flush buffer; left-over bits are also output with zero padding
void
Bitstream::flushbits()
{
    flush_buf();
    
    if (cur_bit == 0)
        return;
    
    int l=write(fd, buf, 1);
    if (l!=1){
#ifdef USE_EXCEPTION
        throw WriteFailed();
#else
        seterror(E_WRITE_FAILED);
        return;
#endif
    }
    buf[0] = 0;
    cur_bit = 0;		// now only the left-over bits
}

// align bitstream
void
Bitstream::align(int n)
{
    // we only allow alignment on multiples of bytes
    if (n % 8) {
#ifdef USE_EXCEPTION
        throw InvalidAlignment();
#else
        seterror(E_INVALID_ALIGNMENT);
#endif
        return;
    }
    
    // align on next byte
    if (tot_bits%8)
        skipbits(8-(tot_bits%8));
    while (tot_bits % n)
        skipbits(8);
    return;
}

//
// get the next chunk of data from whatever the source is
//
void
Bitstream::fill_buf()
{
    int	n;	// how many bytes we must fetch (already read)
    int	l;	// how many bytes we will fetch (available)
    int	u;	// how many are still unread
    
    n = (cur_bit >> 3);
    u = buf_len-n;
    
    // move unread contents to the beginning of the buffer
    memmove(buf, buf+n, u);
    
    // clear the rest of buf
    memset(buf+u, 0, n);
    
    l = read(fd, buf+u, n);
    // check for end of data
    if (l==0) {
        end=1;
#ifdef USE_EXCEPTION
        throw EndOfData();
#else
        seterror(E_END_OF_DATA);
        return;
#endif
    }
    else if (l<0) {
        end=1;
#ifdef USE_EXCEPTION
        throw ReadFailed();
#else
        seterror(E_READ_FAILED);
        return;
#endif
    }
    else if (l<n)
        buf_len=u+l;
    
    // now we are at the first byte
    cur_bit &= 7;
}

// output the buffer excluding the left-over bits.
void
Bitstream::flush_buf()
{
    int l;	// number of bytes written already
    
    l = (cur_bit >> 3);
    
    // file output
    int n=write(fd, buf, l);
    if (n!=l) {
#ifdef USE_EXCEPTION
        throw WriteFailed();
#else
        seterror(E_WRITE_FAILED);
        return;
#endif
    }
    // are there any left-over bits?
    if (cur_bit & 0x7) {
        buf[0] = buf[l];	// copy the left-over bits
        // zero-out rest of buffer
        memset(buf+1, 0, BS_BUF_LEN-1);
    }
    else {
        // zero-out entire buffer
        memset(buf, 0, BS_BUF_LEN);
    }
    // keep left-over bits only
    cur_bit &= 7;
}

