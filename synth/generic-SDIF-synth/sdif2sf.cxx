#include <iostream.h>
#include <stdlib.h>
#include <string.h>

#include "sdifu.h"


int
main(int argc, char **argv) {

    // parse the command line.
    if (argc < 2) {
	cerr << "Usage: " << argv[0] << " [-streams <#,#,#,...>] "
	     << "[-format <aiff | wav | raw>] <sdif file>\n";
	return 1;
    }

    int i = 1;
    char *streams = NULL;
    char *format = NULL;
    char *infile = NULL;
    while (i < argc) {

	if (!strcmp("-streams", argv[i])) {
	    if ((i+1) > (argc-1)) {
		cerr << argv[0] << ": -streams: an argument is required.\n";
		return 1;
	    }
	    if ((i+1) == (argc-1)) {
		cerr << argv[0] << ": no input file was specified.\n";
		return 1;
	    }
	    streams = argv[i+1];
	    i += 2;
	    continue;
	}

	if (!strcmp("-format", argv[i])) {
	    if ((i+1) > (argc-1)) {
		cerr << argv[0] << ": -format: an argument is required.\n";
		return 1;
	    }
	    if ((i+1) == (argc-1)) {
		cerr << argv[0] << ": no input file was specified.\n";
		return 1;
	    }
	    format = argv[i+1];
	    if (*format == '.') format++;
	    if (strcasecmp("aiff", format) &&
		strcasecmp("wav", format) &&
		strcasecmp("raw", format)) {
		cerr << argv[0] << ": unrecognized format string: \""
		     << argv[i+1] << "\".  "
		     << "Must be \"aiff\", \"wav\", or \"raw\".\n";
		return 1;
	    }
	    i += 2;
	    continue;
	}

	if (i == (argc-1)) {
	    infile = argv[i];
	    break;
	} else {
	    cerr << argv[0] << ": unknown option: \"" << argv[i] << "\"\n";
	    return 1;
	}

    }

    if (format == NULL) format = "aiff";

    // index the SDIF file.
    // this needs to be done whether or not the user asked to see it.
    streamIndexNode *idx_list, *p;
    if ((idx_list = SDIFU_GetIndexFromFile(infile)) == NULL) {
	cerr << argv[0] << ": error indexing \"" << infile << "\": "
	     << SDIFU_GetLastErrorString() << endl;
	return 1;
    }

    // get the comma-delimited string of stream numbers the user wants.
    // a stream number is not a "stream id" -- it is just the order in
    // which the first frame from each stream appears in the SDIF data.
    int stream_num;
    char *str, *stream_string = strdup(streams);
    str = strtok(stream_string, ",");
    if ((stream_num = atoi(str)) < 1) {
	cerr << argv[0] << ": bad stream number: \"" << stream_num << "\"\n";
	return 1;
    }




    return 0;

}
