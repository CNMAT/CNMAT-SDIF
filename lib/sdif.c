/*
 * Copyright (c) 1997,1998 Regents of the University of California.
 * All rights reserved.
 * The name of the University may not be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE
 * IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.

 sdif.c

 Utilities for formatting data into SDIF

 SDIF spec: http://www.cnmat.berkeley.edu/SDIF

 Matt Wright, 1/24/97
 Version 1.1 5/12/97 by Matt + Amar (little endian)
 Version 1.2 1/12/98 by Amar (revised spec)

*/

#include <stdio.h>
#include <string.h>
#include "sdif.h"

void SizeofSanityCheck(void) {
  int OK = 1;
  
  if (sizeof(int32) != 4) {
    fprintf(stderr, "sizeof(int32) is %d!!!\n", sizeof(int32));
    OK = 0;
  }
  
  if (sizeof(float32) != 4) {
    fprintf(stderr, "sizeof(float32) is %d!!!\n", sizeof(float32));
    OK = 0;
  }
  
  if (sizeof(float64) != 8) {
    fprintf(stderr, "sizeof(float64) is %d!!!\n", sizeof(float64));
    OK = 0;
  }
  
  if (!OK) {
    exit(-234);
  }
}


void FillSDIFGlobalHeader(struct SDIFGlobalHeader *h) {
  Copy4Bytes(h->SDIF, "SDIF");
  h->size = 8;
  memset (h->reserved,0,8);
}

void Copy4Bytes(char *target, const char *string) {
  target[0] = string[0];
  target[1] = string[1];
  target[2] = string[2];
  target[3] = string[3];
}

int str4eq(const char *this, const char *that) {
  return this[0] == that[0] && this[1] == that[1] &&
    this[2] == that[2] && this[3] == that[3];
}

FILE *OpenSDIFWrite(const char *filename) {
  struct SDIFGlobalHeader h;
  FILE *result;

  SizeofSanityCheck();

  if ((result = fopen(filename, "wb")) == NULL) return NULL;
  FillSDIFGlobalHeader(&h);
  if (WriteSDIFGlobalHeader(&h, result) < 1) {
    fclose(result);
    return NULL;
  }

  return result;
}

int CloseSDIFWrite(FILE *f) {
  fflush(f);
  return fclose(f);
}
int WriteSDIFGlobalHeader(struct SDIFGlobalHeader *h, FILE *f) {
#ifdef LITTLE_ENDIAN
  if (write1(&(h->SDIF),4,f) != 4)
    return -1;
  if (write4(&(h->size),1,f) != 1)
    return -1;
  if (write1(&(h->reserved),8,f) != 8)
    return -1;
  return 1;
#else
  return fwrite(h, sizeof(*h), 1, f) == 1;
#endif
}


FILE *OpenSDIFRead(const char *filename) {
  FILE *result = NULL;
  char buf[8];
  int size;

  SizeofSanityCheck();

  if ((result = fopen(filename, "rb")) == NULL) return NULL;

  /* Now make sure the header is OK. */
  if (read1(buf,4, result) != 4) goto lose;
  if (!str4eq(buf, "SDIF")) goto lose;
  if (read4(&size, 1, result) != 1) goto lose;
  
  if (size % 8 != 0) {
    goto lose;
  }

  /* skip size bytes */
  fseek (result,size,SEEK_CUR);

  return result;

lose:
  fprintf(stderr, "Bad SDIF header in file %s\n", filename);
  fclose(result);
  return NULL;
}

int CloseSDIFRead(FILE *f) {
  return fclose(f);
}

#include <time.h>

int32 GenUniqueSDIFFrameID(void) {
  return (int32) time(NULL);
}



int SkipSDIFFrame(struct SDIFFrameHeader *head, FILE *f) {
  /* The header's size count includes the 8-byte time tag, 4-byte
     stream ID and 4-byte matrix count that we already read. */
  int bytesToSkip = head->size - 16;

  if (bytesToSkip < 0) {
    fprintf(stderr, "Badly formed SDIF file: frame's size too low for time tag and ID.\n");
    return -1;
  }

  if (fseek(f, bytesToSkip, SEEK_CUR) != 0) {
    fprintf(stderr, "fseek failed while skipping unrecognized frame type\n");
    return -2;
  }

  return 0;
}


/***********************/

int WriteSDIFFrameHeader(struct SDIFFrameHeader *fh, FILE *f) {
#ifdef LITTLE_ENDIAN
  if (write1(&(fh->frameType),4,f) != 4)
    return -1;
  if (write4(&(fh->size),1,f) != 1)
    return -1;
  if (write8(&(fh->time),1,f) != 1)
    return -1;
  if (write4(&(fh->streamID),1,f) != 1)
    return -1;
  if (write4(&(fh->matrixCount),1,f) != 1)
    return -1;
#ifdef __WIN32__
  fflush(f);
#endif
  return 1;
#else
  return fwrite(fh, sizeof(*fh), 1, f) == 1;
#endif
}

int ReadSDIFFrameHeader(struct SDIFFrameHeader *fh, FILE *f) {
#ifdef LITTLE_ENDIAN
  if (read1(&(fh->frameType),4,f) != 4)
    return -1;
  if (read4(&(fh->size),1,f) != 1)
    return -1;
  if (read8(&(fh->time),1,f) != 1)
    return -1;
  if (read4(&(fh->streamID),1,f) != 1)
    return -1;
  if (read4(&(fh->matrixCount),1,f) != 1)
    return -1;
  return 1;
#else
  return fread(fh, sizeof(*fh), 1, f) == 1;
#endif
}

/***********/

int WriteRowOf1TRC(RowOf1TRC *row, FILE *f) {
  return (write4(row,4,f) == 4);
}

int ReadRowOf1TRC(RowOf1TRC *row, FILE *f) {
  return read4(row,4,f) == 4;
}

int Read1TRCVals(FILE *f, float *indexp, float *freqp, float *phasep, float *ampp) {
  RowOf1TRC data;

#ifdef LITTLE_ENDIAN
  if (read4(&data, 4, f) != 1) {
    return -1;
  }
#else
  if (fread (&data, sizeof(data), 1, f) != 1) {
    return -1;
  }
#endif

  *indexp = data.index;
  *freqp = data.freq;
  *phasep = data.phase;
  *ampp = data.amp;
  return 0;
}

int Write1TRCVals(FILE *f, float index, float freq, float phase, float amp) {
  RowOf1TRC data;

  data.index = index;
  data.freq = freq;
  data.phase = phase;
  data.amp = amp;

#ifdef LITTLE_ENDIAN
  if (write4(&data, 4, f) != 1) {
    return -1;
  }
#else
  if (fwrite (&data, sizeof(data), 1, f) != 1) {
    return -1;
  }
#endif

  return 0;

}

int32 SizeOf1TRCFrame (int numTracks) {
  /* 16 bytes for the time stamp, ID and matrix count, plus 16 bytes for
     the # rows, # columns, matrix type and matrix data type,
     plus four 4-byte floating point numbers (index, amp, freq, phase)
     for each track appearing in this frame. Note that this is always a
     multiple of 8, so no padding is necessary*/

  return  16 + 16 + (4 * 4 * numTracks);
}


/************* 1RES ****************/

int WriteRowOf1RES (RowOf1RES *row, FILE *f) {
  return (write4(row,4,f) == 4);
}

int ReadRowOf1RES (RowOf1RES *row, FILE *f) {
  return read4(row,4,f) == 4;
}

int32 SizeOf1RESFrame (int numResonances) {
  /* 16 bytes for the time stamp, ID and matrix count, plus 16 bytes for
     the # rows, # columns, matrix type and matrix data type,
     plus four 4-byte floating point numbers (freq, gain, bw phase)
     for each track appearing in this frame. Note that this is always a
     multiple of 8, so no padding is necessary*/

  return  16 + 16 + (4 * 4 * numResonances);
}


/***********/

int WriteMatrixHeader(MatrixHeader *m, FILE *f) {
#ifdef LITTLE_ENDIAN
  if (write1(&(m->matrixType),4,f) != 4) return -1;
  if (write4(&(m->matrixDataType),1,f) != 1) return -1;
  if (write4(&(m->rowCount),1,f) != 1) return -1;
  if (write4(&(m->columnCount),1,f) != 1) return -1;
  return 1;
#else
  return fwrite(m, sizeof(*m), 1, f) == 1;
#endif

}
int ReadMatrixHeader(MatrixHeader *m, FILE *f) {
#ifdef LITTLE_ENDIAN
  if (read1(&(m->matrixType),4,f) != 4) return -1;
  if (read4(&(m->matrixDataType),1,f) != 1) return -1;
  if (read4(&(m->rowCount),1,f) != 1) return -1;
  if (read4(&(m->columnCount),1,f) != 1) return -1;
  return 1;
#else
  return fread(m, sizeof(*m), 1, f) == 1;
#endif

}



/***********************************************************/
/********* Abstract away big endian/little endian in *******/
/********* reading/writing 1, 2, 4, and 8 byte words. ******/
/***********************************************************/

#ifdef LITTLE_ENDIAN
#define BUFSIZE 4096
static	char	p[BUFSIZE];
#endif


int write1(void *block, size_t n, FILE *f)
{
  return fwrite (block,1,n,f);
}

int write2(void *block, size_t n, FILE *f)
{
#ifdef LITTLE_ENDIAN

  short temp;
  char  *q = block;
  int	  i,m=2*n;

  if ((n << 1) > BUFSIZE) {
    int num = BUFSIZE >> 1;
    int numWritten;
    numWritten = write2(block, num, f);
    numWritten += write2(((char *) block) + num, n-num, f);
    return numWritten;
  } 

  for (i = 0; i < m; i += 2) {
    p[i] = q[i+1];
    p[i+1] = q[i];
  }
  return fwrite(p,2,n,f);
#else
  return fwrite (block,2,n,f);
#endif
}

int write4(void *block, size_t n, FILE *f)
{
#ifdef LITTLE_ENDIAN
  char  *q = block;
  int	  i,m = 4*n;

  if ((n << 2) > BUFSIZE) {
    int num = BUFSIZE >> 2;
    int numWritten;
    numWritten = write4(block, num, f);
    numWritten += write4(((char *) block) + num, n-num, f);
    return numWritten;
  } 

  for (i = 0; i < m; i += 4) {
    p[i] = q[i+3];
    p[i+3] = q[i];
    p[i+1] = q[i+2];
    p[i+2] = q[i+1];
  }
  return fwrite(p,4,n,f);
#else
  return fwrite (block,4,n,f);
#endif

}

int write8(void *block, size_t n, FILE *f)
{
#ifdef LITTLE_ENDIAN
  char  *q = block;
  int	  i,m = 8*n;

  if ((n << 3) > BUFSIZE) {
    int num = BUFSIZE >> 3;
    int numWritten;
    numWritten = write8(block, num, f);
    numWritten += write8(((char *) block) + num, n-num, f);
    return numWritten;
  } 


  for (i = 0; i < m; i += 8) {
    p[i] = q[i+7];
    p[i+7] = q[i];
    p[i+1] = q[i+6];
    p[i+6] = q[i+1];
    p[i+2] = q[i+5];
    p[i+5] = q[i+2];
    p[i+3] = q[i+4];
    p[i+4] = q[i+3];
  }
  return fwrite(p,8,n,f);
#else
  return fwrite (block,8,n,f);
#endif
}


int read1(void *block, size_t n, FILE *f)
{
  return fread (block,1,n,f);
}

int read2(void *block, size_t n, FILE *f)
{

#ifdef LITTLE_ENDIAN
  short temp;
  char	*q = block;
  int	  i,m = 2*n;
  int result;

  if ((n << 1) > BUFSIZE) {
    int num = BUFSIZE >> 1;
    int numread;
    numread = read2(block, num, f);
    numread += read2(((char *) block) + num, n-num, f);
    return numread;
  } 

  result = fread(p,2,n,f);

  for (i = 0; i < m; i += 2) {
    q[i] = p[i+1];
    q[i+1] = p[i];
  }

  return result;
#else
  return fread(block,2,n,f);
#endif
}

int read4(void *block, size_t n, FILE *f)
{
#ifdef LITTLE_ENDIAN
  long temp;
  char	*q = block;
  int	  i,m = 4 * n;
  int result;

  if ((n << 2) > BUFSIZE) {
    int num = BUFSIZE >> 2;
    int numread;
    numread = read4(block, num, f);
    numread += read4(((char *) block) + num, n-num, f);
    return numread;
  } 

  result = fread(p,4,n,f);

  for (i = 0; i < m; i += 4) {
    q[i] = p[i+3];
    q[i+3] = p[i];
    q[i+1] = p[i+2];
    q[i+2] = p[i+1];
  }
  return result;

#else
  return fread(block,4,n,f);
#endif

}

int read8(void *block, size_t n, FILE *f)
{
#ifdef LITTLE_ENDIAN
  long temp;
  char	*q = block;
  int	  i,m = 8 * n;
  int result;

  if ((n << 3) > BUFSIZE) {
    int num = BUFSIZE >> 3;
    int numread;
    numread = read8(block, num, f);
    numread += read8(((char *) block) + num, n-num, f);
    return numread;
  } 


  result = fread(p,8,n,f);
  for (i = 0; i < m; i += 8) {
      q[i] = p[i+7];
      q[i+7] = p[i];
      q[i+1] = p[i+6];
      q[i+6] = p[i+1];
      q[i+2] = p[i+5];
      q[i+5] = p[i+2];
      q[i+3] = p[i+4];
      q[i+4] = p[i+3];
  }
  return result;

#else
  return fread(block,8,n,f);
#endif
}




