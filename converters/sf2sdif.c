/*

  sf2sdif --

  A program to convert files consisting of time-domain samples (such as
  AIFF, WAV, etc) into SDIF files.

  Sami Khoury
  11/6/1998

*/


#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sdif.h"
#include "sdifu.h"
#include "soundfiles.h"


int
main(int argc, char **argv) {

    int i, num_channels, num_frames, samples_per_frame, read_amt;
    int num_read;
    float time_per_frame = 0;
    char *infile, *outfile;
    SFHandle sf_handle;
    double sample_rate;
    sdif_float32 **samples, timestamp;
    FILE *sdif_handle;

    /* parse the command line. */
    if ((argc != 3) && (argc != 5)) {
	fprintf(stderr,
		"Usage: %s [-time_per_frame <seconds>] "
		"<input file> <output file>\n", argv[0]);
	return 1;
    } else if (argc == 3) {
	infile = argv[1];
	outfile = argv[2];
    } else if (argc == 5) {

	if (strcmp("-time_per_frame", argv[1])) {
	    fprintf(stderr,
		    "Usage: %s [-time_per_frame <seconds>] "
		    "<input file> <output file>\n", argv[0]);
	    return 1;
	}

	time_per_frame = atof(argv[2]);
	if (time_per_frame <= 0) {
	    fprintf(stderr, "%s: time per frame must be positive.\n", argv[0]);
	    return 1;
	}

	if (time_per_frame == HUGE_VAL) {
	    fprintf(stderr,
		    "%s: time per frame is huge -- frame granularity "
		    "will be low.\n", argv[0]);
	    time_per_frame = 0;
	}

	infile = argv[3];
	outfile = argv[4];

    }

    if (SDIF_Init()) {
	fprintf(stderr, "%s: couldn't initialize SDIF library\n", argv[0]);
    }


    /* try to open the input file. */
    if ((sf_handle = SFOpenRead(infile)) == NULL) {
	fprintf(stderr, "%s: error opening \"%s\": %s\n",
		argv[0], infile, SFGetLastErrorAsString());
	return 1;
    }

    /* try to open the output file.  don't overwrite anything. */
    if ((sdif_handle = fopen(outfile, "r")) != NULL) {
	fprintf(stderr, "%s: file \"%s\" already exists.  Exiting...\n",
		argv[0], outfile);
	return 1;
    }
    fclose(sdif_handle);


    if ((sdif_handle = SDIF_OpenWrite(outfile)) == NULL) {
	fprintf(stderr, "%s: error opening \"%s\": %s\n",
		argv[0], outfile, SDIF_GetLastErrorString());
	return 1;
    }

    sample_rate = SFSampleRate(sf_handle);
    num_frames = SFNumFrames(sf_handle);

    /* if time_per_frame is 0, put all the sample data in one SDIF frame. */
    if ((samples_per_frame = time_per_frame * sample_rate) == 0) {
	samples_per_frame = num_frames;
    }

    /* allocate space for the samples. */
    num_channels = SFNumChannels(sf_handle);
    samples = (sdif_float32 **) malloc(num_channels * sizeof(sdif_float32 *));
    for (i=0; i < num_channels; i++) {
	samples[i] = (sdif_float32 *) malloc(4 * samples_per_frame);
    }

    /* do the conversion. */
    timestamp = 0;
    num_read = 0;
    while (num_frames > 0) {

	if (num_frames > samples_per_frame) read_amt = samples_per_frame;
	else read_amt = num_frames;

	for (i=0; i < num_channels; i++) {
	    SFReadChannel(sf_handle, read_amt, samples[i], 1, num_read, i);
	}

	SDIFU_SamplesToTDSFrame(sdif_handle,
				1234,  /* arbitrary stream id. */
				timestamp,
				sample_rate,
				num_channels,
				read_amt,
				samples);

	num_read += read_amt;
	num_frames -= read_amt;
	timestamp += (read_amt / sample_rate);

    }

    /* finish up. */
    SFCloseRead(sf_handle);
    SDIF_CloseWrite(sdif_handle);

    return 0;

}
