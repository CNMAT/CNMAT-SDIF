/* Original author: Eric D. Scheirer, MIT Media Laboratory
 *
 * This source file has been placed in the public domain by its author(s).
 */

#ifndef _bitstream_h_
#define _bitstream_h

#include "port.h"

// Map code not included in this release
// #include "map.h"

// Bitstream Class

// bitstream type (input or output)
typedef enum {
    BS_INPUT,
        BS_OUTPUT
} Bitstream_t;


// the various exceptions/errors we can expect
typedef enum {
    E_NONE,
        E_END_OF_DATA,
        E_INVALID_ALIGNMENT,
        E_READ_FAILED,
        E_WRITE_FAILED
} Error_t;

// buffer size
const int BS_BUF_LEN = 1024;

class Bitstream 
{
private:
    // bitstream type (input/output)
    Bitstream_t type;
    
    // file descriptor (for file-based reading/writing)
    int fd;
    
    // file name (when given)
    char fn[MAX_LEN];
    
    // set to 1 by the constructor if the fd needs to be closed
    char close_fd;
    
    // input buffer
    unsigned char buf[BS_BUF_LEN];
    int buf_len;    // usable buffer size (for partially filled buffers)
    
    // current bit position in buf
    int cur_bit;
    
    // total bits read/written
    unsigned int tot_bits;
    
    // end of data flag
    unsigned char end;
    
    // error code (useful when exceptions are not supported)
    Error_t err_code;
    
    // functions            
    void fill_buf();    // fills buffer
    void flush_buf();   // flushes buffer
    
    // sets error code
    void seterror(Error_t err) { err_code = err; } 	
    
public:
    // convert error code to text message
    static char const *err2msg(Error_t code);
    
public:
    
    // bitstream as a file, given a file name
    Bitstream(const char *filename, Bitstream_t t);
    
    // bitstream as a file descriptor
    Bitstream(int fd, Bitstream_t t);
    
    ~Bitstream(void);
    
    // bitstream operations
    
    // get next 'n' bits, advance (input only)
    unsigned int getbits(int n);
    
    // probe next 'n' bits, do not advance (input only)
    unsigned nextbits(int length);
    
    // skip next 'n' bits (both input/output)
    void skipbits(int n);
    
    // put 'n' bits (output only)
    int putbits(unsigned int bits, int n);
    
    // align bitstream (n must be multiple of 8, both input/output)
    void align(int n);
    
    // float
    float getfloat(void);
    float nextfloat(void);
    float putfloat(float f);
    
    // double
    double getdouble(void);
    double nextdouble(void);
    double putdouble(double d);
    
    // long double 
    long double getldouble(void)     { return getdouble();  }
    long double nextldouble(void)    { return nextdouble(); }
    long double putldouble(double d) { return putdouble(d); }
    
    
    // flush buffer; left-over bits are also output with zero padding
    // (output only)
    void flushbits();
    
    // get current position (both input/output)
    unsigned int getpos(void)        { return tot_bits; }
    
    // returns 1 if reached end of data
    inline int atend() { return end; }
    
    // get last error
    inline int geterror(void) { return err_code; }
    
    // get last error in text form
    char const *getmsg(void) { return err2msg(err_code); }
    
#ifdef USE_EXCEPTION
    // Exception handling (when supported)
    
    // Our base exception
    class Error {
        Error_t code;    // exception code, from Error_t
        char const *txt; // text of the error
    public:
        Error(Error_t c) : code(c) {}
        inline int geterror(void)   { return code; }
        inline char const *getmsg(void)  { return Bitstream::err2msg(code); }
    };
    
    class EndOfData : public Error {
    public:
        EndOfData(void) : Error(E_END_OF_DATA) {}
    };
    
    class InvalidAlignment : public Error {
    public:
        InvalidAlignment(void) : Error(E_INVALID_ALIGNMENT) {}
    };
    
    class ReadFailed : public Error {
    public:
        ReadFailed(void) : Error(E_READ_FAILED) {}
    };
    
    class WriteFailed : public Error {
    public:
        WriteFailed(void) : Error(E_WRITE_FAILED) {}
    };
#endif /* USE_EXCEPTION */
    
};

#endif /* ! _bitstream_h_ */
