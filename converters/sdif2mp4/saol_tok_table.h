/* ISO_HEADER_START */

/*********************************************************************

  This software module was originally developed by
  
	Eric D. Scheirer (MIT Media Laboratory)
	
  in the course of development of the MPEG-4 standard.
  This software module is an implementation of a part of one or more
  MPEG-4 tools as specified by the MPEG-4 standard.  ISO/IEC gives users
  of the MPEG-4 standard free license to this software module or
  modifications thereof for use in hardware or software products
  claiming conformance to MPEG-4.  Those intending to use this software
  module in hardware or software products are advised that its use may
  infringe existing patents.  The original developer of this software
  module and his/her company, the subsequent editors and their
  companies, and ISO/IEC have no liability for use of this software
  module or modifications thereof in an implementation.  Copyright is
  not released for non MPEG-4 conforming products. The MIT Media
  Laboratory retains full right to use the code for its own purpose,
  assign or donate the code to a third party and to inhibit third
  parties from using the code for non MPEG-4 conforming products.  This
  copyright notice must be included in all copies or derivative
  works. Copyright (c) 1998.
	 
***********************************************************************/

/* ISO_HEADER_END */

int is_builtin(char *);
int lexel_map(long);
char *tok_str(int);

#define TOK_AOPCODE 0x01
#define TOK_ASIG 0x02
#define TOK_ELSE 0x03
#define TOK_EXPORTS 0x04
#define TOK_EXTEND 0x05
#define TOK_GLOBAL 0x06
#define TOK_IF 0x07
#define TOK_IMPORTS 0x08
#define TOK_INCHANNELS 0x09
#define TOK_INSTR 0x0A
#define TOK_IOPCODE 0x0B
#define TOK_IVAR 0x0C
#define TOK_KOPCODE 0x0D
#define TOK_KRATE 0x0E
#define TOK_KSIG 0x0F
#define TOK_MAP 0x10
#define TOK_OPARRAY 0x11
#define TOK_OPCODE 0x12
#define TOK_OUTBUS 0x13
#define TOK_OUTCHANNELS 0x14
#define TOK_OUTPUT 0x15
#define TOK_RETURN 0x16
#define TOK_ROUTE 0x17
#define TOK_SEND 0x18
#define TOK_SEQUENCE 0x19
#define TOK_SASBF 0x1A
#define TOK_SPATIALIZE 0x1B
#define TOK_SRATE 0x1C
#define TOK_TABLE 0x1D
#define TOK_TABLEMAP 0x1E
#define TOK_TEMPLATE 0x1F
#define TOK_TURNOFF 0x20
#define TOK_WHILE 0x21
#define TOK_WITH 0x22
#define TOK_XSIG 0x23
#define TOK_INTERP 0x24
#define TOK_PRESET 0x25

#define TOK_K_RATE 0x30
#define TOK_S_RATE 0x31
#define TOK_INCHAN 0x32
#define TOK_OUTCHAN 0x33
#define TOK_TIME 0x34
#define TOK_DUR 0x35
#define TOK_MIDICTRL 0x36
#define TOK_MIDITOUCH 0x37
#define TOK_MIDIBEND 0x38
#define TOK_INPUT 0x39
#define TOK_INGROUP 0x3A
#define TOK_RELEASED 0x3B
#define TOK_CPULOAD 0x3C
#define TOK_POSITION 0x3D
#define TOK_DIRECTION 0x3E
#define TOK_LISTENERPOSITION 0x3F
#define TOK_LISTENERDIRECTION 0x40
#define TOK_MINFRONT 0x41
#define TOK_MINBACK 0x42
#define TOK_MAXFRONT 0x43
#define TOK_MAXBACK 0x44
#define TOK_PARAMS 0x45
#define TOK_ITIME 0x46
#define TOK_TEMPO 0x47
#define TOK_CHANNEL 0x48
#define TOK_INPUT_BUS 0x49
#define TOK_OUTPUT_BUS 0x4A
#define TOK_STARTUP 0x4B

#define TOK_AND 0x50
#define TOK_OR 0x51
#define TOK_GEQ 0x52
#define TOK_LEQ 0x53
#define TOK_NEQ 0x54
#define TOK_EQEQ 0x55
#define TOK_MINUS 0x56
#define TOK_STAR 0x57
#define TOK_SLASH 0x58
#define TOK_PLUS 0x59
#define TOK_GT 0x5A
#define TOK_LT 0x5B
#define TOK_Q 0x5C
#define TOK_COL 0x5D
#define TOK_LP 0x5E
#define TOK_RP 0x5F
#define TOK_LC 0x60
#define TOK_RC 0x61
#define TOK_LB 0x62
#define TOK_RB 0x63
#define TOK_SEM 0x64
#define TOK_COM 0x65
#define TOK_EQ 0x66
#define TOK_NOT 0x67

#define TOK_SAMPLE 0x6F
#define TOK_DATA 0x70
#define TOK_RANDOM 0x71
#define TOK_STEP 0x72
#define TOK_LINESEG 0x73
#define TOK_EXPSEG 0x74
#define TOK_CUBICSEG 0x75
#define TOK_POLYNOMIAL 0x76
#define TOK_SPLINE 0x77
#define TOK_WINDOW 0x78
#define TOK_HARM 0x79
#define TOK_HARM_PHASE 0x7A
#define TOK_PERIODIC 0x7B
#define TOK_BUZZ 0x7C
#define TOK_CONCAT 0x7D
#define TOK_EMPTY 0x7E
#define TOK_DESTROY 0x7F

#define TOK_CO_INT 0x80 
#define TOK_FRAC 0x81
#define TOK_DBAMP 0x82
#define TOK_AMPDB 0x83
#define TOK_ABS 0x84
#define TOK_EXP 0x85
#define TOK_LOG 0x86
#define TOK_SQRT 0x87
#define TOK_SIN 0x88
#define TOK_COS 0x89
#define TOK_ATAN 0x8A
#define TOK_POW 0x8B
#define TOK_LOG10 0x8C
#define TOK_ASIN 0x8D
#define TOK_ACOS 0x8E
#define TOK_FLOOR 0x8F
#define TOK_CEIL 0x90
#define TOK_MIN 0x91
#define TOK_MAX 0x92
#define TOK_PCHOCT 0x93
#define TOK_OCTPCH 0x94
#define TOK_CPSPCH 0x95
#define TOK_PCHCPS 0x96
#define TOK_CPSOCT 0x97
#define TOK_OCTCPS 0x98
#define TOK_PCHMIDI 0x99
#define TOK_MIDIPCH 0x9A
#define TOK_OCTMIDI 0x9B
#define TOK_MIDIOCT 0x9C
#define TOK_CPSMIDI 0x9D
#define TOK_MIDICPS 0x9E
#define TOK_SGN 0x9F

#define TOK_FTLEN 0xA0
#define TOK_FTLOOP 0xA1
#define TOK_FTLOOPEND 0xA2
#define TOK_FTSETLOOP 0xA3
#define TOK_FTSETEND 0xA4
#define TOK_FTBASECPS 0xA5
#define TOK_FTSETBASE 0xA6
#define TOK_TABLEREAD 0xA7
#define TOK_TABLEWRITE 0xA8
#define TOK_OSCIL 0xA9
#define TOK_LOSCIL 0xAA
#define TOK_DOSCIL 0xAB
#define TOK_KOSCIL 0xAC
#define TOK_KLINE 0xAD
#define TOK_ALINE 0xAE
#define TOK_SBLOCK 0xAF
#define TOK_KEXPON 0xB0
#define TOK_AEXPON 0xB1
#define TOK_KPHASOR 0xB2
#define TOK_APHASOR 0xB3
#define TOK_PLUCK 0xB4
#define TOK_CO_BUZZ 0xB5
#define TOK_GRAIN 0xB6
#define TOK_IRAND 0xB7
#define TOK_KRAND 0xB8
#define TOK_ARAND 0xB9
#define TOK_ILINRAND 0xBA
#define TOK_KLINRAND 0xBB
#define TOK_ALINRAND 0xBC
#define TOK_IEXPRAND 0xBD
#define TOK_KEXPRAND 0xBE
#define TOK_AEXPRAND 0xBF
#define TOK_KPOISSONRAND 0xC0
#define TOK_APOISSONRAND 0xC1
#define TOK_IGAUSSRAND 0xC2
#define TOK_KGAUSSRAND 0xC3
#define TOK_AGAUSSRAND 0xC4
#define TOK_PORT 0xC5
#define TOK_HIPASS 0xC6
#define TOK_LOPASS 0xC7
#define TOK_BANDPASS 0xC8
#define TOK_BANDSTOP 0xC9
#define TOK_FIR 0xCA
#define TOK_IIR 0xCB
#define TOK_FIRT 0xCC
#define TOK_IIRT 0xCD
#define TOK_BIQUAD 0xCE
#define TOK_FFT 0xCF
#define TOK_IFFT 0xD0
#define TOK_RMS 0xD1
#define TOK_GAIN 0xD2
#define TOK_BALANCE 0xD3
#define TOK_DECIMATE 0xD4
#define TOK_UPSAMP 0xD5
#define TOK_DOWNSAMP 0xD6
#define TOK_SAMPHOLD 0xD7
#define TOK_DELAY 0xD8
#define TOK_DELAY1 0xD9
#define TOK_FDELAY 0xDA
#define TOK_COMB 0xDB
#define TOK_ALLPASS 0xDC
#define TOK_CHORUS 0xDD
#define TOK_FLANGE 0xDE
#define TOK_REVERB 0xDF
#define TOK_COMPRESSOR 0xE0
#define TOK_GETTUNE 0xE1
#define TOK_SETTUNE 0xE2
#define TOK_FTSR 0xE3
#define TOK_FTSETSR 0xE4
#define TOK_GETTEMPO 0xE5
#define TOK_SETTEMPO 0xE6
#define TOK_FX_SPEEDC 0xE7
#define TOK_SPEEDC_T 0xE8

#define TOK_SYMBOL 0xF0
#define TOK_NUMBER 0xF1
#define TOK_INT 0xF2
#define TOK_STRING 0xF3
#define TOK_BYTE 0xF4
#define TOK_EOO 0xFF

#define TOK_IDUMP 0xF6
#define TOK_KDUMP 0xF7
#define TOK_ADUMP 0xF8
