/************************************************************
 * 
 * sdif2mp4.cpp : Converts SDIF files to MP4 Structured Audio 
 *                bitstreams
 *
 * For external documentation, see
 *
 *  M. Wright & E.D. Scheirer (1999).  "Cross-coding SDIF into 
 *  MPEG-4 Structured Audio".  Proc. 1999 International 
 *  Computer Music Conference, Beijing.
 *
 * Original author: Eric D. Scheirer, MIT Media Laboratory
 * 
 * This source file is copyright (c) 1999 The Media Laboratory
 * and the Massachusetts Institute of Technology.  Permission is
 * granted for free use in research and non-commercial projects;
 * written permission is required for any use in commercial projects.
 *
 * History: 
 *
 *  15 Jun 1999: Version 1.0 by Eric 
 *
 **************************************************************/

#include <stdio.h>

/* global variables that change the operation of the cross coder */

/* default place to look for the synthesis code */
#define DEFAULT_SDIF_ORC_NAME "sdif.saol"
/* default control (frame) rate of the synthesizer, in Hz */
#define DEFAULT_KRATE 200.0
/* maximum number of streams in a file */
#define MAX_STREAMS 20
/* maximum number of simultaneous frames (has to match up with
   the number of 'imports' tables in the synthesizer) */
#define MAX_TABLE_INDEX 5

/* this is the name of the controller in each SAOL instrument that
   receives the message that the instrument got a new table */

#define CHANGE_CONTROLLER_NAME "changed"

/* this is the base of the names of the wavetables in the SAOL
   orchestra (table1, table2, table3, ...) */

#define TABLE_NAME_BASE "sdif_table"

/* ******* frame-type definitions ******* */

/* update this section to support new frame types */

#define NUM_FRAME_TYPES 3
#define MAX_MATRIX_TYPES 2
typedef enum { FT_1PCH, FT_1TRC, FT_1STF, UNK_FT = -1 } ftypes;

struct frame_type_table {
  char *SDIFname; /* frame ID */
  ftypes ftype;   /* frame type */
  char *instr;    /* instrument for synthesis */
  int num_mtypes; /* number of matrix types it supports */
  char *matname[MAX_MATRIX_TYPES]; /* names of all the matrix types it supports */
  int mtag[MAX_MATRIX_TYPES]; /* tags for all the matrix types it supports */
} frame_def[] = {
	{"1FQ0", FT_1PCH, "pitch_est", 1, {"1FQ0"}, {1}},
	{"1TRC", FT_1TRC, "track",     1, {"1TRC"}, {1}},
	{"1STF", FT_1STF, "stft",      2, {"1STI", "1STF"}, {1, 2}}
};

/* *************************************** */

/* These files are part of the public SA distribution, see
     http://sound.media.mit.edu/mpeg4 */

#include "bitstream.h"
#include "sa_bitstream.h"
#include "saol_tok_table.h"

extern "C" {
#include "sdif.h"   /* SDIF-reading library */
#include "saol.tab.h" /* lexical analysis tokens */
extern FILE *yyin; /* 'yy' keywords are in sa_encode.yy.c */
int yylex(void); 
extern char yytext[];
}

/* all floats in SA are 32-bit */
typedef sdif_float32 sa_real;

/* this structure holds all the information about the command-
   line options */
struct cmdinfo {
	char *sdif;  /* name of the SDIF file */
	char *orc;   /* name of the SAOL orchestra for synthesis */
	char *mp4;   /* name of the MP4 (output) file */
	sa_real kpd; /* control (frame) rate of the synthesizer */
} ;

/* a simple class to manage the wavetables that are dynamically
   built-up from the SDIF frames */

class wavetable {
private:
	vector <sa_real> data; /* all the data points in this table */
	int instr; /* which note the wavetable goes to */
	int index; /* the number of this wavetable */
	sa_real time; /* the delivery time of this table */
public:
	wavetable(int, int, sa_real); 
    /* CTR options are note number, table index, and time */
	void push(int);
	/* put an int  in the table */
	void push(sdif_float32);
	/* put a float in the table */
	void push(void *, int, int, int); 
	/* put a whole matrix in the table */
	void make_events(SA_access_unit *);
	/* make all the events (two of them) that the table needs */
} ;

cmdinfo *process_cmdline(int argc, char *argv[]);
void help();
void deal_with_orc(cmdinfo *cmd, Bitstream *out);
orc_file *parse_saol(Bitstream *out);
void deal_with_SDIF(FILE *,cmdinfo *,Bitstream *);
void convert_frame_to_table(SDIF_FrameHeader fh, FILE *SDIF, Bitstream *out);
int useful_frame_type(char *type);
char *get_streamType_instr(char *streamType);
int matrix_type_tag(char *ft, char *mt);
int lookup_stream(unsigned int streamID);
int new_stream(SDIF_FrameHeader fh, SA_access_unit *au);
void make_end_event(Bitstream *out, sa_real time);
int stream_type(int id);
ftypes ft_to_tag(char *type);
void reset_table_index();
int next_table_index();
char *get_instr_id_label(int id);
char *get_table_name(int index);
void fatal_SDIF_error(char *loc);
void fatal_error(char *s);
void *malloc_wrapper(int size);
void free_wrapper(void *p, int size);

void main(int argc, char *argv[]) {
	Bitstream *out;
	cmdinfo *cmd;
	FILE *SDIFfp;
	char s[80];

	/* get the command line arguments */
	cmd = process_cmdline(argc, argv);

	/* initialize and open SDIF processing */

	if (SDIF_Init(malloc_wrapper,free_wrapper)) { /* nonzero is error */
		fatal_SDIF_error("initializing SDIF");
	}

	if (!(SDIFfp = SDIF_OpenRead(cmd->sdif))) {
		sprintf(s,"opening '%s'",cmd->sdif);
		fatal_SDIF_error(s);
	}

	/* open the bitstream file */

	out = new Bitstream(cmd->mp4,BS_OUTPUT);

	/* deal with the orchestra */

	deal_with_orc(cmd,out);

	/* deal with the SDIF file */

	deal_with_SDIF(SDIFfp,cmd,out);

	/* close the bitstream file */

	out->flushbits();
	delete out;
	exit(0);
}

cmdinfo *process_cmdline(int argc, char *argv[]) {
	// suck in all the command line arguments and fill up the command structure
	cmdinfo *cmd = new cmdinfo;

	// first initialize
	cmd->sdif = NULL;
	cmd->orc = NULL;
	cmd->mp4 = NULL;
	cmd->kpd = (sa_real)(1./DEFAULT_KRATE);

	// go through all the command line arguments
	while (++argv,--argc) {
		/* name of the input file */
		if (!strncmp(*argv,"-sdif",2)) { cmd->sdif = *(++argv); argc--; }
		/* name of the SAOL orchestra */
		else if (!strncmp(*argv,"-orc",2)) { cmd->orc = *(++argv); argc--; }
		/* name of the output file */
		else if (!strncmp(*argv,"-mp4",2)) { cmd->mp4 = *(++argv); argc--; }
		/* orchestra k-rate */
		else if (!strncmp(*argv,"-krate",2)) { 
			cmd->kpd = (sa_real)1.0 / atoi(*(++argv)); argc--;
		}
		/* dump out help message */
		else if (!strncmp(*argv,"-help",2)) help();
		/* print version number */
		else if (!strcmp(*argv,"-v")) { 
			printf("sdif2mp4 version 1.0 (15 June 1999)\n");
		}
		else { printf("Unknown option '%s'.\n",*argv); help(); exit(1); }
	}

	if (!cmd->sdif) { /* input file name is required */
		help();
		exit(1);
	}
	/* use default orc name if none given */
	if (!cmd->orc) cmd->orc = DEFAULT_SDIF_ORC_NAME;

	/* make default output name if none given:
	    remove '.sdif' if its there
	    then add '.mp4' regardless */
	if (!cmd->mp4) {
		char *end,t[100];

		end = strrchr(cmd->sdif,'.'); /* find final extension */
		if (!strcmp(end,".sdif")) {
			strncpy(t,cmd->sdif,end - cmd->sdif);
			t[end-cmd->sdif] = 0;
		}
		else
			strcpy(t,cmd->sdif);
		strcat(t,".mp4");
		cmd->mp4 = strdup(t);
	
		printf("Using '%s' as output file.\n",cmd->mp4);
	}
	return cmd;
}

void help() {
	/* print out help for user. */
  printf("Usage: sdif2mp4 -sdif [sdif_file] [options]\n");
  printf("\n");
  printf("Options:\n");
  printf("   -orc         : SAOL file for SDIF synthesis\n");
  printf("   -mp4         : MP4 output file\n");
  printf("   -v           : print version number.\n");
  printf("\n");
  exit(1);
}


/***************************************************************/
// functions for dealing with the orchestra 
/***************************************************************/

sa_symtable symtab; /* orchestra symbol table */

void deal_with_orc(cmdinfo *cmd, Bitstream *out) {
	/* get the orchestra and put it in the bitstream
	   note that there's no error-checking on the orchestra 
	   here -- you should make sure the orchestra is 
	   clean before you run sdif2mp4*/
    orc_file *orc;
	FILE *fp;
	char s[100];
	
	/* open orchestra file */

	if (!(fp = fopen(cmd->orc,"r"))) {
		sprintf(s,"Couldn't open orchestra file '%s'.",cmd->orc);
		fatal_error(s);
	}

	yyin = fp; /* pass FILE to lexer */
	orc = parse_saol(out);

	out->putbits(1,1); /* more data */
	out->putbits(0,3); /* orc_file chunk */
	orc->put(*out);    /* put the orchestra */
	out->putbits(1,1); /* more data */
	out->putbits(5,3); /* symbol table chunk */
	symtab.put(*out);  /* put the symbol table */
	out->putbits(0,1); /* no more header data */

	return;
}

orc_file *parse_saol(Bitstream *out) {
	/* suck in a SAOL file and tokenize it */
  int lexel;
  orch_token t;
  int i;
  orc_file *o;

  o = new orc_file;
  
  while (lexel = yylex()) { /* yylex() returns one lexical token at a time */
    
	 if (lexel > STRCONST)  /* it's a punctuation mark */
         t.token = lexel_map(lexel);  /* look it up in the bitstream token table */
	 
	 if (lexel == IDENT) {  /* it's a identifier */

		 if (is_builtin(yytext) != -1)  /* see if it's a builtin */
			 t.token = is_builtin(yytext); /* (reserved word, core opcode, etc.) */
		 else { /* sa_symbol */
			 t.token = TOK_SYMBOL;      /* it's not */
			 t.sym = symtab.add(yytext);  /* add it to the symbol table */
		 }
	 }

	 if (lexel == NUMBER) { /* it's a float */
		 t.token = TOK_NUMBER;
		 t.val = atof(yytext);
	 }
	 
	 if (lexel == INTGR) { /* it's an int */
		 i = atoi(yytext);
		 if (i >=0 && i <= 255) /* use BYTE token for short ints */
			 t.token = TOK_BYTE;
		 else
			 t.token = TOK_INT;
		 t.ival = atoi(yytext);
	 }
	 if (lexel == STRCONST) {
		 /* Must be a sound file */
		 printf("Can't deal with sound file '%s'.",&yytext[1]);
	 }
	 
	 o->add(t); /* add the token to the orchestra chunk */
  }

  return o;
}

/***************************************************************/
// functions for dealing with the SDIF file 
/***************************************************************/

/* this table associates all of the streamIDs from the SDIF file with
   the streamType that they're supposed to be. The ID of a stream is
   just for us to keep track of how many we've seen.  */

struct stream_table_struct {
	int len;
	unsigned int streamID[MAX_STREAMS];
	ftypes stream_type[MAX_STREAMS];
} stream_table;

void deal_with_SDIF(FILE *SDIF,cmdinfo *cmd,Bitstream *out) {
	SDIF_FrameHeader fh;
	int done = 0, rtn, ct = 0; 
	sdif_float64 curtime = 0;

	stream_table.len = 0;

	while (!done) { /* breaks out at EOF */

		/* get the next frame */
		rtn	= SDIF_ReadFrameHeader(&fh, SDIF);
		if (rtn < 0) fatal_SDIF_error("reading frame");
		if (!rtn) /* EOF */ { done = 1;  break; }
		
		/* check if this frame is at the same time as the previous one */
		if (fh.time > curtime + cmd->kpd) { /* can reset tables */ 
			curtime = fh.time;
			reset_table_index();
		}

		convert_frame_to_table(fh,SDIF,out);
		ct++;
	}

	/* end of file -- make end event */
	make_end_event(out,(sa_real)(curtime + 0.25));
	printf("Processed %d frames over %.2f seconds.\n",ct,curtime);
}

void convert_frame_to_table(SDIF_FrameHeader fh, FILE *SDIF, Bitstream *out) {
	int stream_instr, table_index, rtn;
	wavetable *w; 
	SA_access_unit au;
	char s[100];
	SDIF_Frame f;
	SDIF_Matrix m;
	int first_frame = 0;
	sa_real len, size;

	if (!useful_frame_type(fh.frameType)) {
		rtn = SDIF_SkipFrame(&fh,SDIF);
		if (rtn != 1)
			fatal_SDIF_error("skipping frame");
		return;
	}

	if ((stream_instr = lookup_stream(fh.streamID)) == -1) {
		/* never seen this stream before */
		stream_instr = new_stream(fh,&au);
		first_frame = 1;
	}

	if (ft_to_tag(fh.frameType) != stream_type(stream_instr)) {
		/* type of frame doesn't match */
		sprintf(s,"Stream #%d changed types.",fh.streamID);
		fatal_error(s);
	}

	table_index = next_table_index();

	w = new wavetable(stream_instr,table_index,fh.time);
	
	w->push(fh.matrixCount); /* number of matrixes */


	if (!(f = SDIF_ReadFrameContents(&fh,SDIF,NULL,NULL)))
		fatal_SDIF_error("reading frame contents");

	for (m=f->matrices;m;m=m->next) {
		/* frame-matrix tag */
		w->push(matrix_type_tag(fh.frameType,m->header.matrixType)); 
		w->push(m->header.rowCount);
		w->push(m->header.columnCount);
		w->push(m->data,m->header.rowCount,m->header.columnCount,m->header.matrixDataType);

		if (ft_to_tag(fh.frameType) == FT_1STF &&
			matrix_type_tag(fh.frameType,m->header.matrixType) == 1 &&
			first_frame) {
			/* have to get length from STFT info matrix - ugly */
			len = ((sdif_float32 *)m->data)[1];
		}
		if (ft_to_tag(fh.frameType) == FT_1STF &&
			matrix_type_tag(fh.frameType,m->header.matrixType) == 2 &&
			first_frame) {
			/* have to get size from STFT data matrix - ugly */
			size = m->header.rowCount;
		}
	}
	if (first_frame && ft_to_tag(fh.frameType) == FT_1STF) {
		au_event *ev = au.events[0]; /* instr event */

		ev->score_ev.inst.num_pf = 2;
		ev->score_ev.inst.pf[0] = size;
		ev->score_ev.inst.pf[1] = len;
	}

	w->make_events(&au);

	au.dts = fh.time;
	au.put(*out);
	return;
}
	

int useful_frame_type(char *type) {
	/* return 1 iff this is a frame type we know how to deal with */
	int i;

	for (i=0;i!=NUM_FRAME_TYPES;i++) {
		if (SDIF_Str4Eq(frame_def[i].SDIFname,type)) return 1;
	}
	
	return 0;
}

char *get_streamType_instr(char *type) {
	/* return the name of the instrument to use for synthesis of a 
	   given frame type */
	int i;

	for (i=0;i!=NUM_FRAME_TYPES;i++) {
		if (SDIF_Str4Eq(frame_def[i].SDIFname,type)) 
			return frame_def[i].instr;
	}
	return NULL;
}


int stream_type(int num) {
	/* given the stream number (not the streamID) return its type */
	return stream_table.stream_type[num];
}

ftypes ft_to_tag(char *type) {
	/* return the tag for the given frame type */
	int i;

	for (i=0;i!=NUM_FRAME_TYPES;i++) {
		if (SDIF_Str4Eq(frame_def[i].SDIFname,type)) 
			return frame_def[i].ftype;
	}
	return UNK_FT;
}


int matrix_type_tag(char *ft, char *mt) {
	/* return the matrix tag for a given frame and matrix type */
	int i,j;

	for (i=0;i!=NUM_FRAME_TYPES;i++) {
		/* look up the frame type */
		if (SDIF_Str4Eq(frame_def[i].SDIFname,ft))  {
			/* look for the matrix name within the list of matrix types */
			for (j=0;j!=frame_def[i].num_mtypes;j++) {
				if (SDIF_Str4Eq(frame_def[i].matname[j],mt)) {
					return frame_def[i].mtag[j];
				}
			}
			return -1;
		}
	}
	return -1;
}

int lookup_stream(unsigned int streamID) { 
	/* see if we know about a stream with the given streamID */
	int i;

	for (i=0;i!=stream_table.len;i++) {
		if (stream_table.streamID[i] == streamID)
			return i;
	}

	return -1;
}

int new_stream(SDIF_FrameHeader fh, SA_access_unit *au) {
	/* make a new stream:

       + remember it in the stream table
	   + add an instr event to the MP4 file for the right sort of instrument 
	*/
	au_event *ev;
	int id;
	
	id = stream_table.len;

	if (id == MAX_STREAMS) /* table was full */
		fatal_error("Too many streams in SDIF file (increase MAX_STREAMS).");

	stream_table.len++;
	stream_table.streamID[id] = fh.streamID; 
	stream_table.stream_type[id] = ft_to_tag(fh.frameType); /* lookup stream type */
	
	/* make an instr event */
	ev = new au_event;
	ev->event_type = 0; /* score line */
	ev->score_ev.has_time = 1;
	ev->score_ev.time = fh.time;
	ev->score_ev.type = 0; /* instrument event */
	ev->score_ev.inst.dur = -1; /* unknown duration */
	ev->score_ev.inst.has_label = 1; /* has a label */
	/* find the symbol for the label */
	ev->score_ev.inst.label = symtab.add(get_instr_id_label(id));
	/* figure out which instr to use */
	ev->score_ev.inst.iname_sym = symtab.add(get_streamType_instr(fh.frameType));
	/* no p-fields */
	ev->score_ev.inst.num_pf = 0;
	ev->score_ev.priority = 0;
	ev->score_ev.use_if_late = 0;
	
	/* add this event to the current au */
	au->events.push_back(ev);
	au->num_events++;
	return id;
}

/* within each k-cycle, the table_index tells us which table number the
   next frame should use */

int table_index;

void reset_table_index() {
	table_index = 0;
}

int next_table_index() {
	++table_index;
	if (table_index > MAX_TABLE_INDEX)
		fatal_error("Too many frames with the same timestamp (increase MAX_TABLE_INDEX\nand add tables to SAOL orchestra).");
	return(table_index);
}

/***************************************************************/
// functions for pushing data into wavetables 
/***************************************************************/

wavetable::wavetable(int instr_, int index_, sa_real time_) {
	data.clear();
	instr = instr_;
	index = index_;
	time = time_;
}

void wavetable::push(int i) { 
	/* add an int to the end of the table */
	data.push_back((sa_real) i);
}

void wavetable::push(sdif_float32 f) {
	/* add a float to the end of the table */
	data.push_back((sa_real) f);
}

void wavetable::push(void *vdata, int r, int c, int dataType) {
	/* add a whole matrix to the end of the table */
	int i,j;
	char s[100];

	switch (dataType) {
	case SDIF_FLOAT32: 
		{
			sdif_float32 *fdata = (sdif_float32 *)vdata;
			for (i=0;i!=r;i++) {
				for (j=0;j!=c;j++) {
					data.push_back(fdata[i*c+j]);
				}
			}
		} break;
	default:
		sprintf(s,"Can't deal with matrix data type %d.",dataType);
		fatal_error(s);
	}
}

void wavetable::make_events(SA_access_unit *AU) {
	/* add the events representing the wavetable to the AU */
	au_event *ctrl, *table;
	int i;

	// <time> control <instr label> changed <index>

	ctrl = new au_event;
	ctrl->event_type = 0; /* score event */
	ctrl->score_ev.type = 1; /* control event */
	ctrl->score_ev.has_time = 1;
	ctrl->score_ev.time = time;
	ctrl->score_ev.priority = 0;
	ctrl->score_ev.use_if_late = 0;
	ctrl->score_ev.control.has_label = 1;
	ctrl->score_ev.control.label = symtab.add(get_instr_id_label(instr));
	ctrl->score_ev.control.varsym = symtab.add(CHANGE_CONTROLLER_NAME);
	ctrl->score_ev.control.value = (sa_real)index;

	// <time> table <name> data <size> <val1> <val2> ...

	table = new au_event;
	table->event_type = 0; /* score event */
	table->score_ev.type = 2; /* table event */
	table->score_ev.has_time = 1;
	table->score_ev.time = time;
	table->score_ev.priority = 0;
	table->score_ev.use_if_late = 0;
	table->score_ev.table.destroy = 0;
	table->score_ev.table.num_pf = data.size()+1; /* one for the size */
	table->score_ev.table.refers_to_sample = 0;
	table->score_ev.table.tgen = is_builtin("data");
	table->score_ev.table.tname = symtab.add(get_table_name(index));
	table->score_ev.table.pf = new sa_real [data.size()];
	table->score_ev.table.pf[0] = data.size();

	for (i=0;i!=data.size();i++)
	  table->score_ev.table.pf[i+1] = data[i]; /* get the size from the sample */
	
	// now add these two events to the AU

	AU->events.push_back(ctrl);
	AU->events.push_back(table);
	
	AU->num_events += 2;
}

void make_end_event (Bitstream *out, sa_real time) {
	/* make an end event to terminate the synthesis */
	SA_access_unit *AU;
	au_event *end;

	end = new au_event;
	end->event_type = 0; /* score event */
	end->score_ev.has_time = 1;
	end->score_ev.time = time;
	end->score_ev.priority = 0;
	end->score_ev.use_if_late = 0;
	end->score_ev.type = 4; /* end event */

	AU = new SA_access_unit;
	AU->events.push_back(end);
	AU->num_events = 1;
	AU->put(*out);
}

/***************************************************************/
// miscellany 
/***************************************************************/

char *get_instr_id_label(int id) {
	/* make a unique label for the note to send the "changed' 
	   controller to */
	char s[100];

	sprintf(s,"instr%d",id);
	return(strdup(s));
}

char *get_table_name(int index) {
	/* get the name of the i'th table in the SAOL orchestra 
	   (has to match the SAOL code) */
	char s[100];

	sprintf(s,"%s%d",TABLE_NAME_BASE,index);
	return(strdup(s));
}

void fatal_SDIF_error(char *loc) {
	char *s;
	
	s = SDIF_GetLastErrorString();

	printf("Fatal SDIF error while %s: %s.\n",loc,s);
	exit(1);
}

void fatal_error(char *s) {
	printf("Fatal error: %s.\n",s);
	exit(1);
}

void *malloc_wrapper(int size) {
	void *p;

	p = malloc(size);
	return(p);
}

void free_wrapper(void *p, int size) {
	free(p);
}

