/*

  sdif2sf --

  A program to convert from SDIF files to time domain sample files (such as
  AIFF, WAV, etc).

  Sami Khoury
  11/10/1998

*/


#include <iostream>

#include "sdif.h"
#include "sdifu.h"
#include "soundfiles.h"


// each SDIF stream which is being converted is allocated such an object.
typedef struct {
    SFWHandle sf_handle;
    conversion_func *funk;
    char *filename;
    int32 streamID;
} conv;


int
main(int argc, char **argv) {

    // parse the command line.
    if (argc != 3) {
	cerr << "Usage: " << argv[0] << " <input file> <output file>\n";
	return 1;
    }

    // try to open the input file.
    char *infile = argv[1];
    char *outfile = argv[2];
    FILE *sdif_in;
    if ((sdif_in = OpenSDIFRead(infile)) == NULL) {
	cerr << argv[0] << ": error opening \"" << infile << "\": "
	     << SDIF_GetLastErrorAsString() << endl;
	return 1;
    }

    // get an index of all the streams and their types in the input file.
    streamIndexNode *index;
    if ((index = SDIFU_GetIndexFromHandle(sdif_in)) == NULL) {
	cerr << argv[0] << ": error parsing \"" << infile << "\": "
	     << SDIFU_GetLastErrorAsString() << endl;
	return 1;
    }

    // set up the table relating SDIF frame types and conversion functions.
    hash_map<const char *, SDIFFrameToSamplesFunc *, hash<const char *>, eqstr>
	funks;
    funks["1TDS"] = SDIFU_TDStoSamples;
    funks["1TRC"] = SDIFU_TRCtoSamples;

    // determine which of the streams can be converted to time domain samples.
    streamIndexNode *p = index;
    while (p != NULL) {

	
	p = p->next;

    }
    SDIFU_DestroyStreamIndex(index);


    SDIF_FrameHeader fh;


    while (1) {
	UpdateTime(SDIF_NextFrameTime(sdif_in));
    }

    
    return 0;

}
