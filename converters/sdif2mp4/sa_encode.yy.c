# include "stdio.h"
# define U(x) ((unsigned char)(x))
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX 2048
# define output(c) (void)putc(c,yyout)
#if defined(__cplusplus) || defined(__STDC__)

#ifdef __cplusplus
extern "C" {
#endif
	int yylex(void);
	int yyback(int *, int);
	int yyinput(void);
	int yylook(void);
	void yyoutput(int);
	int yyracc(int);
	int yyreject(void);
	void yyunput(int);
#ifndef yyless
	void yyless(long int);
#endif
#ifndef yywrap
	int yywrap(void);
#endif
#ifdef __cplusplus
}
#endif

#endif

# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO (void)fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;

# line 3 "sa_encoder.lex"
/* $Id: sa_encode.yy.c,v 1.2 2001/11/27 21:57:37 matt Exp $ */

# line 4 "sa_encoder.lex"
/* $Log: sa_encode.yy.c,v $
 * Revision 1.2  2001/11/27 21:57:37  matt
 * From /disk5/disk6/people/matt/sdif/saol/sdif2mp4.oldsdif
 *
# Revision 1.2  1998/05/06  21:44:26  eds
# FCD version.
#
# Revision 1.1  1997/11/10  22:58:10  eds
# Initial revision
# */

# line 11 "sa_encoder.lex"
/*********************************************************************

ISO_HEADER_START

This software module was originally developed by

  Eric Scheirer (MIT Media Laboratory)

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

ISO_HEADER_END

***********************************************************************/

# line 46 "sa_encoder.lex"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "saol.tab.h"
  
void count(void),comment(void),dumpline(int i);
int yyline=1,yycol=0;
char thisline[1000],**all_lines = NULL;
int yywrap(void) { return(1); }

# define YYNEWLINE 10
yylex(void){
int nstr; extern int yyprevious;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:

# line 61 "sa_encoder.lex"
	{ comment(); }
break;
case 2:

# line 62 "sa_encoder.lex"
{ count(); return(AOPCODE) ; }
break;
case 3:

# line 63 "sa_encoder.lex"
	{ count(); return(ASIG) ; }
break;
case 4:

# line 64 "sa_encoder.lex"
         { count(); return(ELSE) ; }
break;
case 5:

# line 65 "sa_encoder.lex"
      { count(); return(EXPORTS) ; }
break;
case 6:

# line 66 "sa_encoder.lex"
       { count(); return(EXTEND) ; }
break;
case 7:

# line 67 "sa_encoder.lex"
       { count(); return(GLOBAL) ; }
break;
case 8:

# line 68 "sa_encoder.lex"
           { count(); return(IF) ; }
break;
case 9:

# line 69 "sa_encoder.lex"
      { count(); return(IMPORTS); }
break;
case 10:

# line 70 "sa_encoder.lex"
   { count(); return(INCHANNELS) ; }
break;
case 11:

# line 71 "sa_encoder.lex"
        { count(); return(INSTR); }
break;
case 12:

# line 72 "sa_encoder.lex"
{ count(); return(IOPCODE); }
break;
case 13:

# line 73 "sa_encoder.lex"
	{ count(); return(IVAR) ; }
break;
case 14:

# line 74 "sa_encoder.lex"
{ count(); return(KOPCODE); }
break;
case 15:

# line 75 "sa_encoder.lex"
        { count(); return(KRATE) ; }
break;
case 16:

# line 76 "sa_encoder.lex"
	{ count(); return(KSIG) ; }
break;
case 17:

# line 77 "sa_encoder.lex"
        { count(); return(MAP) ; }
break;
case 18:

# line 78 "sa_encoder.lex"
      { count(); return(OPARRAY) ; }
break;
case 19:

# line 79 "sa_encoder.lex"
{ count(); return(OPCODE) ; }
break;
case 20:

# line 80 "sa_encoder.lex"
{ count(); return(OUTBUS) ; }
break;
case 21:

# line 81 "sa_encoder.lex"
  { count(); return(OUTCHANNELS) ; }
break;
case 22:

# line 82 "sa_encoder.lex"
{ count(); return(OUTPUT) ; }
break;
case 23:

# line 83 "sa_encoder.lex"
{ count(); return(RETURN) ; }
break;
case 24:

# line 84 "sa_encoder.lex"
        { count(); return(ROUTE) ; }
break;
case 25:

# line 85 "sa_encoder.lex"
         { count(); return(SEND) ; }
break;
case 26:

# line 86 "sa_encoder.lex"
     { count(); return(SEQUENCE) ; }
break;
case 27:

# line 87 "sa_encoder.lex"
        { count(); return(SASBF) ; }
break;
case 28:

# line 88 "sa_encoder.lex"
   { count(); return(SPATIALIZE) ; }
break;
case 29:

# line 89 "sa_encoder.lex"
	{ count(); return(SRATE); }
break;
case 30:

# line 90 "sa_encoder.lex"
	{ count(); return(TABLE); }
break;
case 31:

# line 91 "sa_encoder.lex"
     { count(); return(TABLEMAP); }
break;
case 32:

# line 92 "sa_encoder.lex"
{ count(); return(TEMPLATE); }
break;
case 33:

# line 93 "sa_encoder.lex"
      { count(); return(TURNOFF); }
break;
case 34:

# line 94 "sa_encoder.lex"
	{ count(); return(WHILE); }
break;
case 35:

# line 95 "sa_encoder.lex"
         { count(); return(WITH); }
break;
case 36:

# line 96 "sa_encoder.lex"
	{ count(); return(XSIG) ; }
break;
case 37:

# line 97 "sa_encoder.lex"
	{ count(); return(AND); }
break;
case 38:

# line 98 "sa_encoder.lex"
	{ count(); return(OR); }
break;
case 39:

# line 99 "sa_encoder.lex"
	{ count(); return(GEQ); }
break;
case 40:

# line 100 "sa_encoder.lex"
	{ count(); return(LEQ); }
break;
case 41:

# line 101 "sa_encoder.lex"
	{ count(); return(NEQ); }
break;
case 42:

# line 102 "sa_encoder.lex"
	{ count(); return(EQEQ); }
break;
case 43:

# line 103 "sa_encoder.lex"
	{ count(); return(MINUS); }
break;
case 44:

# line 104 "sa_encoder.lex"
	{ count(); return(STAR); }
break;
case 45:

# line 105 "sa_encoder.lex"
	{ count(); return(SLASH); }
break;
case 46:

# line 106 "sa_encoder.lex"
	{ count(); return(PLUS); }
break;
case 47:

# line 107 "sa_encoder.lex"
	{ count(); return(GT); }
break;
case 48:

# line 108 "sa_encoder.lex"
	{ count(); return(LT); }
break;
case 49:

# line 109 "sa_encoder.lex"
	{ count(); return(Q); }
break;
case 50:

# line 110 "sa_encoder.lex"
	{ count(); return(COL); }
break;
case 51:

# line 111 "sa_encoder.lex"
	{ count(); return(LP); }
break;
case 52:

# line 112 "sa_encoder.lex"
	{ count(); return(RP); }
break;
case 53:

# line 113 "sa_encoder.lex"
	{ count(); return(LC); }
break;
case 54:

# line 114 "sa_encoder.lex"
	{ count(); return(RC); }
break;
case 55:

# line 115 "sa_encoder.lex"
	{ count(); return(LB); }
break;
case 56:

# line 116 "sa_encoder.lex"
	{ count(); return(RB); }
break;
case 57:

# line 117 "sa_encoder.lex"
	{ count(); return(SEM); }
break;
case 58:

# line 118 "sa_encoder.lex"
	{ count(); return(COM); }
break;
case 59:

# line 119 "sa_encoder.lex"
	{ count(); return(EQ); }
break;
case 60:

# line 120 "sa_encoder.lex"
            { count(); return(NOT); }
break;
case 61:

# line 122 "sa_encoder.lex"
{ count(); yytext[strlen(yytext)-1] = 0; yytext; /* strip quotes */
                           return(STRCONST); }
break;
case 62:

# line 124 "sa_encoder.lex"
	{ count(); return(IDENT) ; }
break;
case 63:

# line 125 "sa_encoder.lex"
	{ count(); return(INTGR) ; }
break;
case 64:

# line 126 "sa_encoder.lex"
{ count(); return(NUMBER) ; }
break;
case 65:

# line 127 "sa_encoder.lex"
	{ count(); }
break;
case 66:

# line 128 "sa_encoder.lex"
	{ count();
                  printf("Line %d: Unknown character: '%s'\n",yyline,yytext); }
break;
case -1:
break;
default:
(void)fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */

# line 132 "sa_encoder.lex"

void comment() {
  char c;

  while ((c = input()) != '\n'); /* skip */
  yyline++;
  thisline[0] = 0;
  yycol = 0;
  }
              

void count() {
  char *nl;
  
  if (strcmp(yytext,"\n")) {
    yycol += strlen(yytext);
    strcat(thisline,yytext);
    }
  else {
    int i=0;

    while (thisline[i] == ' ' || thisline[i] == '\t') i++;
    nl = strdup(&thisline[i]);
   
    yycol = 0;
    if (all_lines)
      all_lines = (char **)realloc(all_lines,(yyline +2) * sizeof(char *));
    else
      all_lines = (char **)malloc(2 * sizeof(char *));
    all_lines[yyline-1] = nl; /* since yyline starts from 1 */
    yyline++;
    thisline[0] = 0;
    all_lines[yyline-1] = thisline;
    }
  }

void dumpline(int i) {
 
  printf("%s\n",all_lines[i]);
}
int yyvstop[] = {
0,

66,
0,

65,
66,
0,

65,
0,

60,
66,
0,

66,
0,

66,
0,

51,
66,
0,

52,
66,
0,

44,
66,
0,

46,
66,
0,

58,
66,
0,

43,
66,
0,

64,
66,
0,

45,
66,
0,

63,
64,
66,
0,

50,
66,
0,

57,
66,
0,

48,
66,
0,

59,
66,
0,

47,
66,
0,

49,
66,
0,

62,
66,
0,

55,
66,
0,

56,
66,
0,

62,
66,
0,

62,
66,
0,

62,
66,
0,

62,
66,
0,

62,
66,
0,

62,
66,
0,

62,
66,
0,

62,
66,
0,

62,
66,
0,

62,
66,
0,

62,
66,
0,

62,
66,
0,

53,
66,
0,

66,
0,

54,
66,
0,

41,
0,

61,
0,

37,
0,

64,
0,

1,
0,

64,
0,

63,
64,
0,

40,
0,

42,
0,

39,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

8,
62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

38,
0,

64,
0,

64,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

17,
62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

3,
62,
0,

4,
62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

13,
62,
0,

62,
0,

62,
0,

16,
62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

25,
62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

35,
62,
0,

36,
62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

11,
62,
0,

62,
0,

62,
0,

15,
62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

24,
62,
0,

27,
62,
0,

62,
0,

62,
0,

29,
62,
0,

30,
62,
0,

62,
0,

62,
0,

34,
62,
0,

62,
0,

62,
0,

6,
62,
0,

7,
62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

19,
62,
0,

20,
62,
0,

62,
0,

22,
62,
0,

23,
62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

2,
62,
0,

5,
62,
0,

9,
62,
0,

62,
0,

12,
62,
0,

14,
62,
0,

18,
62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

62,
0,

33,
62,
0,

62,
0,

62,
0,

26,
62,
0,

62,
0,

31,
62,
0,

32,
62,
0,

62,
0,

62,
0,

62,
0,

10,
62,
0,

62,
0,

28,
62,
0,

21,
62,
0,
0};
# define YYTYPE unsigned char
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	1,3,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,4,	1,5,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,6,	1,7,	
0,0,	0,0,	0,0,	1,8,	
8,46,	1,9,	1,10,	1,11,	
1,12,	1,13,	1,14,	1,15,	
1,16,	1,17,	14,47,	16,49,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	1,18,	
1,19,	1,20,	1,21,	1,22,	
1,23,	6,42,	1,24,	20,53,	
21,54,	22,55,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
7,43,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
7,43,	7,43,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
1,25,	1,3,	1,26,	0,0,	
0,0,	0,0,	1,27,	0,0,	
0,0,	0,0,	1,28,	43,45,	
1,29,	0,0,	1,30,	32,70,	
1,31,	7,44,	1,32,	66,99,	
1,33,	68,101,	77,112,	1,34,	
1,35,	1,36,	7,43,	29,61,	
1,37,	1,38,	2,6,	7,43,	
1,39,	1,40,	1,41,	2,8,	
27,57,	2,9,	2,10,	2,11,	
27,58,	2,13,	2,14,	2,15,	
2,16,	38,84,	31,67,	33,71,	
7,43,	31,68,	31,69,	40,85,	
33,72,	37,82,	37,83,	2,18,	
2,19,	2,20,	2,21,	2,22,	
2,23,	15,47,	15,47,	15,47,	
15,47,	15,47,	15,47,	15,47,	
15,47,	15,47,	15,47,	57,89,	
58,90,	59,91,	17,50,	7,45,	
17,51,	17,51,	17,51,	17,51,	
17,51,	17,51,	17,51,	17,51,	
17,51,	17,51,	34,73,	28,59,	
2,25,	2,3,	2,26,	60,92,	
61,94,	63,95,	2,27,	60,93,	
34,74,	65,98,	2,28,	28,60,	
2,29,	67,100,	2,30,	36,79,	
2,31,	69,102,	2,32,	36,80,	
2,33,	64,96,	70,103,	2,34,	
2,35,	2,36,	15,48,	72,106,	
2,37,	2,38,	73,107,	74,108,	
2,39,	2,40,	2,41,	36,81,	
71,104,	64,97,	71,105,	75,109,	
78,113,	17,52,	24,56,	24,56,	
24,56,	24,56,	24,56,	24,56,	
24,56,	24,56,	24,56,	24,56,	
76,110,	79,114,	80,115,	76,111,	
81,116,	82,117,	83,118,	24,56,	
24,56,	24,56,	24,56,	24,56,	
24,56,	24,56,	24,56,	24,56,	
24,56,	24,56,	24,56,	24,56,	
24,56,	24,56,	24,56,	24,56,	
24,56,	24,56,	24,56,	24,56,	
24,56,	24,56,	24,56,	24,56,	
24,56,	84,119,	89,120,	90,121,	
91,122,	24,56,	92,123,	24,56,	
24,56,	24,56,	24,56,	24,56,	
24,56,	24,56,	24,56,	24,56,	
24,56,	24,56,	24,56,	24,56,	
24,56,	24,56,	24,56,	24,56,	
24,56,	24,56,	24,56,	24,56,	
24,56,	24,56,	24,56,	24,56,	
24,56,	30,62,	93,124,	35,75,	
45,43,	94,125,	95,126,	35,76,	
30,63,	30,64,	30,65,	96,127,	
45,43,	45,0,	97,128,	98,129,	
99,130,	30,66,	35,77,	48,48,	
35,78,	100,131,	48,86,	48,86,	
48,86,	48,86,	48,86,	48,86,	
48,86,	48,86,	48,86,	48,86,	
101,132,	102,133,	104,134,	105,135,	
107,139,	45,43,	108,140,	109,141,	
110,142,	111,143,	112,144,	113,145,	
114,146,	115,147,	45,43,	116,148,	
106,136,	106,137,	117,149,	45,43,	
50,50,	50,50,	50,50,	50,50,	
50,50,	50,50,	50,50,	50,50,	
50,50,	50,50,	106,138,	118,150,	
52,87,	119,151,	52,87,	120,152,	
45,43,	52,88,	52,88,	52,88,	
52,88,	52,88,	52,88,	52,88,	
52,88,	52,88,	52,88,	86,86,	
86,86,	86,86,	86,86,	86,86,	
86,86,	86,86,	86,86,	86,86,	
86,86,	87,88,	87,88,	87,88,	
87,88,	87,88,	87,88,	87,88,	
87,88,	87,88,	87,88,	123,153,	
124,154,	125,155,	126,156,	127,157,	
128,158,	50,52,	129,159,	131,160,	
132,161,	134,162,	135,163,	136,164,	
137,165,	138,166,	139,167,	140,168,	
141,169,	143,170,	144,171,	145,172,	
146,173,	147,174,	148,175,	149,176,	
152,177,	153,178,	154,179,	155,180,	
156,181,	157,182,	159,183,	160,184,	
162,185,	163,186,	164,187,	165,188,	
166,189,	167,190,	170,191,	171,192,	
173,193,	174,194,	175,195,	177,196,	
178,197,	181,198,	182,199,	183,200,	
184,201,	185,202,	188,203,	191,204,	
192,205,	193,206,	194,207,	195,208,	
199,209,	203,210,	204,211,	205,212,	
206,213,	207,214,	209,215,	210,216,	
212,217,	215,218,	216,219,	217,220,	
219,221,	0,0,	0,0,	0,0,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+-1,	0,		0,	
yycrank+-89,	yysvec+1,	0,	
yycrank+0,	0,		yyvstop+1,
yycrank+0,	0,		yyvstop+3,
yycrank+0,	0,		yyvstop+6,
yycrank+4,	0,		yyvstop+8,
yycrank+-75,	0,		yyvstop+11,
yycrank+2,	0,		yyvstop+13,
yycrank+0,	0,		yyvstop+15,
yycrank+0,	0,		yyvstop+18,
yycrank+0,	0,		yyvstop+21,
yycrank+0,	0,		yyvstop+24,
yycrank+0,	0,		yyvstop+27,
yycrank+4,	0,		yyvstop+30,
yycrank+105,	0,		yyvstop+33,
yycrank+4,	0,		yyvstop+36,
yycrank+120,	0,		yyvstop+39,
yycrank+0,	0,		yyvstop+43,
yycrank+0,	0,		yyvstop+46,
yycrank+6,	0,		yyvstop+49,
yycrank+7,	0,		yyvstop+52,
yycrank+8,	0,		yyvstop+55,
yycrank+0,	0,		yyvstop+58,
yycrank+174,	0,		yyvstop+61,
yycrank+0,	0,		yyvstop+64,
yycrank+0,	0,		yyvstop+67,
yycrank+17,	yysvec+24,	yyvstop+70,
yycrank+71,	yysvec+24,	yyvstop+73,
yycrank+11,	yysvec+24,	yyvstop+76,
yycrank+195,	yysvec+24,	yyvstop+79,
yycrank+27,	yysvec+24,	yyvstop+82,
yycrank+10,	yysvec+24,	yyvstop+85,
yycrank+27,	yysvec+24,	yyvstop+88,
yycrank+77,	yysvec+24,	yyvstop+91,
yycrank+202,	yysvec+24,	yyvstop+94,
yycrank+98,	yysvec+24,	yyvstop+97,
yycrank+41,	yysvec+24,	yyvstop+100,
yycrank+22,	yysvec+24,	yyvstop+103,
yycrank+0,	0,		yyvstop+106,
yycrank+19,	0,		yyvstop+109,
yycrank+0,	0,		yyvstop+111,
yycrank+0,	0,		yyvstop+114,
yycrank+-11,	yysvec+7,	0,	
yycrank+0,	0,		yyvstop+116,
yycrank+-299,	0,		0,	
yycrank+0,	0,		yyvstop+118,
yycrank+0,	yysvec+15,	yyvstop+120,
yycrank+270,	0,		0,	
yycrank+0,	0,		yyvstop+122,
yycrank+300,	0,		yyvstop+124,
yycrank+0,	yysvec+17,	yyvstop+126,
yycrank+317,	0,		0,	
yycrank+0,	0,		yyvstop+129,
yycrank+0,	0,		yyvstop+131,
yycrank+0,	0,		yyvstop+133,
yycrank+0,	yysvec+24,	yyvstop+135,
yycrank+51,	yysvec+24,	yyvstop+137,
yycrank+59,	yysvec+24,	yyvstop+139,
yycrank+50,	yysvec+24,	yyvstop+141,
yycrank+71,	yysvec+24,	yyvstop+143,
yycrank+73,	yysvec+24,	yyvstop+145,
yycrank+0,	yysvec+24,	yyvstop+147,
yycrank+73,	yysvec+24,	yyvstop+150,
yycrank+102,	yysvec+24,	yyvstop+152,
yycrank+77,	yysvec+24,	yyvstop+154,
yycrank+14,	yysvec+24,	yyvstop+156,
yycrank+81,	yysvec+24,	yyvstop+158,
yycrank+16,	yysvec+24,	yyvstop+160,
yycrank+92,	yysvec+24,	yyvstop+162,
yycrank+90,	yysvec+24,	yyvstop+164,
yycrank+119,	yysvec+24,	yyvstop+166,
yycrank+91,	yysvec+24,	yyvstop+168,
yycrank+94,	yysvec+24,	yyvstop+170,
yycrank+94,	yysvec+24,	yyvstop+172,
yycrank+104,	yysvec+24,	yyvstop+174,
yycrank+122,	yysvec+24,	yyvstop+176,
yycrank+17,	yysvec+24,	yyvstop+178,
yycrank+123,	yysvec+24,	yyvstop+180,
yycrank+135,	yysvec+24,	yyvstop+182,
yycrank+125,	yysvec+24,	yyvstop+184,
yycrank+122,	yysvec+24,	yyvstop+186,
yycrank+132,	yysvec+24,	yyvstop+188,
yycrank+122,	yysvec+24,	yyvstop+190,
yycrank+160,	yysvec+24,	yyvstop+192,
yycrank+0,	0,		yyvstop+194,
yycrank+327,	0,		yyvstop+196,
yycrank+337,	0,		0,	
yycrank+0,	yysvec+87,	yyvstop+198,
yycrank+167,	yysvec+24,	yyvstop+200,
yycrank+164,	yysvec+24,	yyvstop+202,
yycrank+167,	yysvec+24,	yyvstop+204,
yycrank+159,	yysvec+24,	yyvstop+206,
yycrank+197,	yysvec+24,	yyvstop+208,
yycrank+203,	yysvec+24,	yyvstop+210,
yycrank+191,	yysvec+24,	yyvstop+212,
yycrank+203,	yysvec+24,	yyvstop+214,
yycrank+194,	yysvec+24,	yyvstop+216,
yycrank+212,	yysvec+24,	yyvstop+218,
yycrank+198,	yysvec+24,	yyvstop+220,
yycrank+218,	yysvec+24,	yyvstop+222,
yycrank+212,	yysvec+24,	yyvstop+224,
yycrank+226,	yysvec+24,	yyvstop+226,
yycrank+0,	yysvec+24,	yyvstop+228,
yycrank+216,	yysvec+24,	yyvstop+231,
yycrank+220,	yysvec+24,	yyvstop+233,
yycrank+246,	yysvec+24,	yyvstop+235,
yycrank+215,	yysvec+24,	yyvstop+237,
yycrank+218,	yysvec+24,	yyvstop+239,
yycrank+237,	yysvec+24,	yyvstop+241,
yycrank+236,	yysvec+24,	yyvstop+243,
yycrank+220,	yysvec+24,	yyvstop+245,
yycrank+222,	yysvec+24,	yyvstop+247,
yycrank+223,	yysvec+24,	yyvstop+249,
yycrank+232,	yysvec+24,	yyvstop+251,
yycrank+229,	yysvec+24,	yyvstop+253,
yycrank+233,	yysvec+24,	yyvstop+255,
yycrank+238,	yysvec+24,	yyvstop+257,
yycrank+255,	yysvec+24,	yyvstop+259,
yycrank+258,	yysvec+24,	yyvstop+261,
yycrank+252,	yysvec+24,	yyvstop+263,
yycrank+0,	yysvec+24,	yyvstop+265,
yycrank+0,	yysvec+24,	yyvstop+268,
yycrank+281,	yysvec+24,	yyvstop+271,
yycrank+286,	yysvec+24,	yyvstop+273,
yycrank+300,	yysvec+24,	yyvstop+275,
yycrank+284,	yysvec+24,	yyvstop+277,
yycrank+302,	yysvec+24,	yyvstop+279,
yycrank+286,	yysvec+24,	yyvstop+281,
yycrank+291,	yysvec+24,	yyvstop+283,
yycrank+0,	yysvec+24,	yyvstop+285,
yycrank+292,	yysvec+24,	yyvstop+288,
yycrank+303,	yysvec+24,	yyvstop+290,
yycrank+0,	yysvec+24,	yyvstop+292,
yycrank+291,	yysvec+24,	yyvstop+295,
yycrank+306,	yysvec+24,	yyvstop+297,
yycrank+290,	yysvec+24,	yyvstop+299,
yycrank+304,	yysvec+24,	yyvstop+301,
yycrank+292,	yysvec+24,	yyvstop+303,
yycrank+296,	yysvec+24,	yyvstop+305,
yycrank+310,	yysvec+24,	yyvstop+307,
yycrank+310,	yysvec+24,	yyvstop+309,
yycrank+0,	yysvec+24,	yyvstop+311,
yycrank+312,	yysvec+24,	yyvstop+314,
yycrank+309,	yysvec+24,	yyvstop+316,
yycrank+314,	yysvec+24,	yyvstop+318,
yycrank+315,	yysvec+24,	yyvstop+320,
yycrank+309,	yysvec+24,	yyvstop+322,
yycrank+307,	yysvec+24,	yyvstop+324,
yycrank+318,	yysvec+24,	yyvstop+326,
yycrank+0,	yysvec+24,	yyvstop+328,
yycrank+0,	yysvec+24,	yyvstop+331,
yycrank+320,	yysvec+24,	yyvstop+334,
yycrank+305,	yysvec+24,	yyvstop+336,
yycrank+322,	yysvec+24,	yyvstop+338,
yycrank+315,	yysvec+24,	yyvstop+340,
yycrank+308,	yysvec+24,	yyvstop+342,
yycrank+315,	yysvec+24,	yyvstop+344,
yycrank+0,	yysvec+24,	yyvstop+346,
yycrank+326,	yysvec+24,	yyvstop+349,
yycrank+327,	yysvec+24,	yyvstop+351,
yycrank+0,	yysvec+24,	yyvstop+353,
yycrank+331,	yysvec+24,	yyvstop+356,
yycrank+328,	yysvec+24,	yyvstop+358,
yycrank+315,	yysvec+24,	yyvstop+360,
yycrank+334,	yysvec+24,	yyvstop+362,
yycrank+316,	yysvec+24,	yyvstop+364,
yycrank+323,	yysvec+24,	yyvstop+366,
yycrank+0,	yysvec+24,	yyvstop+368,
yycrank+0,	yysvec+24,	yyvstop+371,
yycrank+324,	yysvec+24,	yyvstop+374,
yycrank+338,	yysvec+24,	yyvstop+376,
yycrank+0,	yysvec+24,	yyvstop+378,
yycrank+327,	yysvec+24,	yyvstop+381,
yycrank+340,	yysvec+24,	yyvstop+384,
yycrank+336,	yysvec+24,	yyvstop+386,
yycrank+0,	yysvec+24,	yyvstop+388,
yycrank+338,	yysvec+24,	yyvstop+391,
yycrank+325,	yysvec+24,	yyvstop+393,
yycrank+0,	yysvec+24,	yyvstop+395,
yycrank+0,	yysvec+24,	yyvstop+398,
yycrank+326,	yysvec+24,	yyvstop+401,
yycrank+332,	yysvec+24,	yyvstop+403,
yycrank+342,	yysvec+24,	yyvstop+405,
yycrank+343,	yysvec+24,	yyvstop+407,
yycrank+324,	yysvec+24,	yyvstop+409,
yycrank+0,	yysvec+24,	yyvstop+411,
yycrank+0,	yysvec+24,	yyvstop+414,
yycrank+336,	yysvec+24,	yyvstop+417,
yycrank+0,	yysvec+24,	yyvstop+419,
yycrank+0,	yysvec+24,	yyvstop+422,
yycrank+348,	yysvec+24,	yyvstop+425,
yycrank+340,	yysvec+24,	yyvstop+427,
yycrank+352,	yysvec+24,	yyvstop+429,
yycrank+334,	yysvec+24,	yyvstop+431,
yycrank+349,	yysvec+24,	yyvstop+433,
yycrank+0,	yysvec+24,	yyvstop+435,
yycrank+0,	yysvec+24,	yyvstop+438,
yycrank+0,	yysvec+24,	yyvstop+441,
yycrank+351,	yysvec+24,	yyvstop+444,
yycrank+0,	yysvec+24,	yyvstop+446,
yycrank+0,	yysvec+24,	yyvstop+449,
yycrank+0,	yysvec+24,	yyvstop+452,
yycrank+343,	yysvec+24,	yyvstop+455,
yycrank+353,	yysvec+24,	yyvstop+457,
yycrank+350,	yysvec+24,	yyvstop+459,
yycrank+344,	yysvec+24,	yyvstop+461,
yycrank+356,	yysvec+24,	yyvstop+463,
yycrank+0,	yysvec+24,	yyvstop+465,
yycrank+350,	yysvec+24,	yyvstop+468,
yycrank+358,	yysvec+24,	yyvstop+470,
yycrank+0,	yysvec+24,	yyvstop+472,
yycrank+338,	yysvec+24,	yyvstop+475,
yycrank+0,	yysvec+24,	yyvstop+477,
yycrank+0,	yysvec+24,	yyvstop+480,
yycrank+346,	yysvec+24,	yyvstop+483,
yycrank+354,	yysvec+24,	yyvstop+485,
yycrank+362,	yysvec+24,	yyvstop+487,
yycrank+0,	yysvec+24,	yyvstop+489,
yycrank+349,	yysvec+24,	yyvstop+492,
yycrank+0,	yysvec+24,	yyvstop+494,
yycrank+0,	yysvec+24,	yyvstop+497,
0,	0,	0};
struct yywork *yytop = yycrank+464;
struct yysvf *yybgin = yysvec+1;
unsigned char yymatch[] = {
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,011 ,012 ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
011 ,01  ,'"' ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,'+' ,01  ,'+' ,01  ,01  ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,01  ,'"' ,01  ,01  ,'A' ,
01  ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
0};
char yyextra[] = {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
#ident	"$Revision: 1.2 $"

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
#if defined(__cplusplus) || defined(__STDC__)
int yylook(void)
#else
yylook()
#endif
{
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych, yyfirst;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	yyfirst=1;
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank && !yyfirst){  /* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
#ifndef LONGLINES
			if(yylastch > &yytext[YYLMAX]) {
				fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
				exit(1);
			}
#endif
			yyfirst=0;
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (void *)yyt > (void *)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
#ifndef LONGLINES
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
#endif
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((void *)yyt < (void *)yycrank) {	/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
#ifndef LONGLINES
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
#endif
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
#ifndef LONGLINES
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
#endif
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = (int)(yylastch-yytext+1);
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
#if defined(__cplusplus) || defined(__STDC__)
int yyback(int *p, int m)
#else
yyback(p, m)
	int *p;
#endif
{
	if (p==0) return(0);
	while (*p) {
		if (*p++ == m)
			return(1);
	}
	return(0);
}
	/* the following are only used in the lex library */
#if defined(__cplusplus) || defined(__STDC__)
int yyinput(void)
#else
yyinput()
#endif
{
	return(input());
	}
#if defined(__cplusplus) || defined(__STDC__)
void yyoutput(int c)
#else
yyoutput(c)
  int c; 
#endif
{
	output(c);
	}
#if defined(__cplusplus) || defined(__STDC__)
void yyunput(int c)
#else
yyunput(c)
   int c; 
#endif
{
	unput(c);
	}
