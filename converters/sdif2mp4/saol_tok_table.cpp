/* Original author: Eric D. Scheirer, MIT Media Laboratory
 *
 * This source file has been placed in the public domain by its author(s).
 */

/* saol_tok_table.cpp: the token tables for bitstream encoding and
   decoding */

#include "string.h"
#include "saol.tab.h"
#include "saol_tok_table.h"

#define N_TOKEN 211

/* This table matches up each lexeme to a bitstream token.  It's used by
   the encoder only. */

struct lexel_map_entry_struct {
  long lexel;
  int token;
} lexel_token[] = {
  {AOPCODE, TOK_AOPCODE},
  {ASIG, TOK_ASIG},
  {ELSE, TOK_ELSE},
  {EXPORTS, TOK_EXPORTS},
  {EXTEND, TOK_EXTEND},
  {GLOBAL, TOK_GLOBAL},
  {IF, TOK_IF},
  {IMPORTS, TOK_IMPORTS},
  {INCHANNELS, TOK_INCHANNELS},
  {INSTR, TOK_INSTR},
  {INTERP, TOK_INTERP},
  {IOPCODE, TOK_IOPCODE},
  {IVAR, TOK_IVAR},
  {KOPCODE, TOK_KOPCODE},
  {KRATE, TOK_KRATE},
  {KSIG, TOK_KSIG},
  {MAP, TOK_MAP},
  {OPARRAY, TOK_OPARRAY},
  {OPCODE, TOK_OPCODE},
  {OUTBUS, TOK_OUTBUS},
/* CORRIGENDUM eds 12oct99 */
  {OUTCHANNELS, TOK_OUTCHANNELS},
/* END CORR */
  {OUTPUT, TOK_OUTPUT},
  {PRESET, TOK_PRESET},
  {RETURN, TOK_RETURN},
  {ROUTE, TOK_ROUTE},
  {SEND, TOK_SEND},
  {SEQUENCE, TOK_SEQUENCE},
  {SASBF, TOK_SASBF},
  {SPATIALIZE, TOK_SPATIALIZE},
  {SRATE, TOK_SRATE},
  {TABLE, TOK_TABLE},
  {TABLEMAP, TOK_TABLEMAP},
  {TEMPLATE, TOK_TEMPLATE},
  {TURNOFF, TOK_TURNOFF},
  {WHILE, TOK_WHILE},
  {WITH, TOK_WITH},
  {XSIG, TOK_XSIG},
  {AND, TOK_AND},
  {OR, TOK_OR},
  {GEQ, TOK_GEQ},
  {LEQ, TOK_LEQ},
  {NEQ, TOK_NEQ},
  {EQEQ, TOK_EQEQ},
  {MINUS, TOK_MINUS},
  {STAR, TOK_STAR},
  {SLASH, TOK_SLASH},
  {PLUS, TOK_PLUS},
  {GT, TOK_GT},
  {LT, TOK_LT},
  {Q, TOK_Q},
  {COL, TOK_COL},
  {LP, TOK_LP},
  {RP, TOK_RP},
  {LC, TOK_LC},
  {RC, TOK_RC},
  {LB, TOK_LB},
  {RB, TOK_RB},
  {SEM, TOK_SEM},
  {COM, TOK_COM},
  {EQ, TOK_EQ},
  {NOT, TOK_NOT} };

  /* this table matches up each bitstream token with its character string --
     it's the table in Annex A.  It's used by the decoder during detokenization
	 and by the encoder during tokenization. */

struct str_table_entry_struct {
  char *str;
  int token;
} str_table[] = {

  {"aopcode", TOK_AOPCODE},
  {"asig", TOK_ASIG},
  {"else", TOK_ELSE},
  {"exports", TOK_EXPORTS},
  {"extend", TOK_EXTEND},
  {"global", TOK_GLOBAL},
  {"if", TOK_IF},
  {"imports", TOK_IMPORTS},
  {"inchannels", TOK_INCHANNELS},
  {"instr", TOK_INSTR},
  {"interp", TOK_INTERP},
  {"iopcode", TOK_IOPCODE},
  {"ivar", TOK_IVAR},
  {"kopcode", TOK_KOPCODE},
  {"krate", TOK_KRATE},
  {"ksig", TOK_KSIG},
  {"map", TOK_MAP},
  {"oparray", TOK_OPARRAY},
  {"opcode", TOK_OPCODE},
  {"outbus", TOK_OUTBUS},
  {"outchannels", TOK_OUTCHANNELS},
  {"output", TOK_OUTPUT},
  {"preset", TOK_PRESET},
  {"return", TOK_RETURN},
  {"route", TOK_ROUTE},
  {"send", TOK_SEND},
  {"sequence", TOK_SEQUENCE},
  {"sasbf", TOK_SASBF},
  {"spatialize", TOK_SPATIALIZE},
  {"srate", TOK_SRATE},
  {"table", TOK_TABLE},
  {"tablemap", TOK_TABLEMAP},
  {"template", TOK_TEMPLATE},
  {"turnoff", TOK_TURNOFF},
  {"while", TOK_WHILE},
  {"with", TOK_WITH},
  {"xsig", TOK_XSIG},
  {"k_rate", TOK_K_RATE},
  {"s_rate", TOK_S_RATE},
  {"inchan", TOK_INCHAN},
  {"outchan", TOK_OUTCHAN},
  {"time", TOK_TIME},
  {"itime", TOK_ITIME},
  {"dur", TOK_DUR},
  {"MIDIctrl", TOK_MIDICTRL},
  {"MIDItouch", TOK_MIDITOUCH},
  {"MIDIbend", TOK_MIDIBEND},
  {"input", TOK_INPUT},
  {"ingroup", TOK_INGROUP},
  {"released", TOK_RELEASED},
  {"cpuload", TOK_CPULOAD},
  {"position", TOK_POSITION},
  {"direction", TOK_DIRECTION},
  {"listenerPosition", TOK_LISTENERPOSITION},
  {"listenerDirection", TOK_LISTENERDIRECTION},
  {"minFront", TOK_MINFRONT},
  {"minBack", TOK_MINBACK},
  {"maxFront", TOK_MAXFRONT},
  {"maxBack", TOK_MAXBACK},
  {"channel", TOK_CHANNEL},
  {"input_bus", TOK_INPUT_BUS},
  {"output_bus", TOK_OUTPUT_BUS},
  {"startup", TOK_STARTUP},
  {"&&", TOK_AND},
  {"||", TOK_OR},
  {">=", TOK_GEQ},
  {"<=", TOK_LEQ},
  {"!=", TOK_NEQ},
  {"==", TOK_EQEQ},
  {"-", TOK_MINUS},
  {"*", TOK_STAR},
  {"/", TOK_SLASH},
  {"+", TOK_PLUS},
  {">", TOK_GT},
  {"<", TOK_LT},
  {"?", TOK_Q},
  {":", TOK_COL},
  {"(", TOK_LP},
  {")", TOK_RP},
  {"{", TOK_LC},
  {"}", TOK_RC},
  {"[", TOK_LB},
  {"]", TOK_RB},
  {";", TOK_SEM},
  {",", TOK_COM},
  {"=", TOK_EQ},
  {"!", TOK_NOT},
  {"sample", TOK_SAMPLE},
  {"data", TOK_DATA},
  {"random", TOK_RANDOM},
  {"step", TOK_STEP},
  {"lineseg", TOK_LINESEG},
  {"expseg", TOK_EXPSEG},
  {"cubicseg", TOK_CUBICSEG},
  {"polynomial", TOK_POLYNOMIAL},
  {"window", TOK_WINDOW},
  {"harm", TOK_HARM},
  {"harm_phase", TOK_HARM_PHASE},
  {"periodic", TOK_PERIODIC},
  {"buzz", TOK_BUZZ},
  {"concat", TOK_CONCAT},
  {"empty", TOK_EMPTY},
  {"int", TOK_CO_INT},
  {"frac", TOK_FRAC},
  {"dbamp", TOK_DBAMP},
  {"ampdb", TOK_AMPDB},
  {"abs", TOK_ABS},
  {"sgn", TOK_SGN},
  {"exp", TOK_EXP},
  {"log", TOK_LOG},
  {"sqrt", TOK_SQRT},
  {"sin", TOK_SIN},
  {"cos", TOK_COS},
  {"atan", TOK_ATAN},
  {"pow", TOK_POW},
  {"log10", TOK_LOG10},
  {"asin", TOK_ASIN},
  {"acos", TOK_ACOS},
  {"floor", TOK_FLOOR},
  {"ceil", TOK_CEIL},
  {"min", TOK_MIN},
  {"max", TOK_MAX},
  {"gettune", TOK_GETTUNE},
  {"settune", TOK_SETTUNE},
  {"pchoct", TOK_PCHOCT},
  {"octpch", TOK_OCTPCH},
  {"cpspch", TOK_CPSPCH},
  {"pchcps", TOK_PCHCPS},
  {"cpsoct", TOK_CPSOCT},
  {"octcps", TOK_OCTCPS},
  {"pchmidi", TOK_PCHMIDI},
  {"midipch", TOK_MIDIPCH},
  {"octmidi", TOK_OCTMIDI},
  {"midioct", TOK_MIDIOCT},
  {"cpsmidi", TOK_CPSMIDI},
  {"midicps", TOK_MIDICPS},
  {"ftlen", TOK_FTLEN},
  {"ftloop", TOK_FTLOOP},
  {"ftloopend", TOK_FTLOOPEND},
  {"ftsetloop", TOK_FTSETLOOP},
  {"ftsetend", TOK_FTSETEND},
  {"ftbasecps", TOK_FTBASECPS},
  {"ftsetbase", TOK_FTSETBASE},
  {"ftsetsr", TOK_FTSETSR},
  {"tableread", TOK_TABLEREAD},
  {"tablewrite", TOK_TABLEWRITE},
  {"oscil", TOK_OSCIL},
  {"loscil", TOK_LOSCIL},
  {"doscil", TOK_DOSCIL},
  {"koscil", TOK_KOSCIL},
  {"kline", TOK_KLINE},
  {"aline", TOK_ALINE},
  {"kexpon", TOK_KEXPON},
  {"aexpon", TOK_AEXPON},
  {"kphasor", TOK_KPHASOR},
  {"aphasor", TOK_APHASOR},
  {"pluck", TOK_PLUCK},
  {"buzz", TOK_CO_BUZZ},
  {"grain", TOK_GRAIN},
  {"irand", TOK_IRAND},
  {"krand", TOK_KRAND},
  {"arand", TOK_ARAND},
  {"ilinrand", TOK_ILINRAND},
  {"klinrand", TOK_KLINRAND},
  {"alinrand", TOK_ALINRAND},
  {"iexprand", TOK_IEXPRAND},
  {"kexprand", TOK_KEXPRAND},
  {"aexprand", TOK_AEXPRAND},
  {"kpoissonrand", TOK_KPOISSONRAND},
  {"apoissonrand", TOK_APOISSONRAND},
  {"igaussrand", TOK_IGAUSSRAND},
  {"kgaussrand", TOK_KGAUSSRAND},
  {"agaussrand", TOK_AGAUSSRAND},
  {"port", TOK_PORT},
  {"hipass", TOK_HIPASS},
  {"lopass", TOK_LOPASS},
  {"bandpass", TOK_BANDPASS},
  {"bandstop", TOK_BANDSTOP},
  {"fir", TOK_FIR},
  {"iir", TOK_IIR},
  {"firt", TOK_FIRT},
  {"iirt", TOK_IIRT},
  {"biquad", TOK_BIQUAD},
  {"fft", TOK_FFT},
  {"ifft", TOK_IFFT},
  {"rms", TOK_RMS},
  {"gain", TOK_GAIN},
  {"balance", TOK_BALANCE},
  {"decimate", TOK_DECIMATE},
  {"upsamp", TOK_UPSAMP},
  {"downsamp", TOK_DOWNSAMP},
  {"samphold", TOK_SAMPHOLD},
  {"delay", TOK_DELAY},
  {"delay1", TOK_DELAY1},
  {"fracdelay", TOK_FDELAY},
  {"comb", TOK_COMB},
  {"allpass", TOK_ALLPASS},
  {"chorus", TOK_CHORUS},
  {"flange", TOK_FLANGE},
  {"reverb", TOK_REVERB},
  {"compressor", TOK_COMPRESSOR},
  {"gettune", TOK_GETTUNE},
  {"settune", TOK_SETTUNE},
  {"ftsr", TOK_FTSR},
  {"ftsetsr", TOK_FTSETSR},
  {"gettempo", TOK_GETTEMPO},
  {"settempo", TOK_SETTEMPO},
  {"fx_speedc", TOK_FX_SPEEDC},
  {"speedt", TOK_SPEEDC_T},
  {"kdump",TOK_KDUMP},
  {"adump",TOK_ADUMP},
  {"idump",TOK_IDUMP}
};

int is_builtin(char *x) {
	/* if a character string is one of the ones in the token table, 
	   return the token that corresponds */
  int i;

  for (i=0; strcmp(str_table[i].str,x) && i < N_TOKEN;
       i++) ;
  if (i < N_TOKEN)
    return str_table[i].token;
  return -1;
}

int lexel_map(long lexel) {
	/* return the bitstream token that corresponds to a lexeme. */
  int i;

  for (i=0;i!=200;i++)
    if (lexel_token[i].lexel == lexel) return(lexel_token[i].token);

  return -1;
}

char *tok_str(int tok) {
	/* return the string that corresponds to a particular bitstream
	   token */
  int i;
  for (i=0;i!=300;i++)
    if (str_table[i].token == tok) return str_table[i].str;

  return 0;
}
