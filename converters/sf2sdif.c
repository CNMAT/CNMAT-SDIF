/*
  sf2sdif --

  A program to convert files consisting of time-domain samples (such as
  AIFF, WAV, etc) into SDIF files.

  Sami Khoury
  11/6/1998

  Modified heavily 6/1/2000, Matt Wright

*/


#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


#include "sdif.h"
#include "soundfiles.h"




void FloatSamplesToTDSFrame(FILE *outfp,
			    sdif_int32 stream_id,
			    sdif_float64 time_stamp,
			    sdif_float32 sample_rate,
			    int num_channels,
			    int num_sampleframes,
			    sdif_float32 *floatsamples) {

    int sz;
    char padding[6] = { 0, 0, 0, 0, 0, 0 };
    SDIF_FrameHeader fh;
    SDIF_MatrixHeader tds, itds;

#define DEBUG
#ifdef DEBUG
    printf("FloatSamplesToTDSFrame(outfp 0x%p, streamID %d, time %f,\n", outfp, stream_id, time_stamp);
    printf("                       srate %f, %d channels, %d frames, samples 0x%p)\n",
	   sample_rate, num_channels, num_sampleframes, floatsamples);

    printf("**FloatSamplesToTDSFrame: last sample is %f\n",
	   floatsamples[(num_channels*num_sampleframes)-1]);

#endif


    /* setup the frame header stuff which we know in advance. */
    SDIF_Copy4Bytes(fh.frameType, "1TDS");
    fh.time = time_stamp;
    fh.streamID = stream_id;
    fh.matrixCount = 2;

    /* setup the ITDS matrix header stuff which we know in advance. */
    SDIF_Copy4Bytes(itds.matrixType, "ITDS");
    itds.matrixDataType = SDIF_FLOAT32;
    itds.rowCount = 1;
    itds.columnCount = 1;

    /* setup the 1TDS matrix header stuff which we know in advance. */
    SDIF_Copy4Bytes(tds.matrixType, "1TDS");
    tds.matrixDataType = SDIF_FLOAT32;
    tds.rowCount = num_sampleframes;
    tds.columnCount = num_channels;

    /* compute the frame size. */
    sz = 4 * num_channels * num_sampleframes;
    fh.size = 16	/* frame header minus type and size. */
	+ 2*16		/* two matrix headers. */
	+ 8		/* the size of the ITDS matrix. */
	+ sz;		/* the size of the 1TDS matrix. */
    fh.size += SDIF_PaddingRequired(&tds);

    /* write out the frame header. */
    SDIF_WriteFrameHeader(&fh, outfp);

    /* write out the ITDS matrix header and the matrix itself. */
    SDIF_WriteMatrixHeader(&itds, outfp);
    SDIF_Write4(&sample_rate, 1, outfp);
    SDIF_Write4(padding, 1, outfp);

    /* write out the 1TDS matrix header. */
    SDIF_WriteMatrixHeader(&tds, outfp);

    /* write out the 1TDS matrix. */
    SDIF_Write4(floatsamples, num_sampleframes * num_channels, outfp);

    /* pad the 1TDS matrix. */
    SDIF_Write1(padding, SDIF_PaddingRequired(&tds), outfp);
}

void ShortSamplesToTDSFrame(FILE *outfp,
			    sdif_int32 stream_id,
			    sdif_float64 time_stamp,
			    sdif_float32 sample_rate,
			    int num_channels,
			    int num_sampleframes,
			    sdif_int16 *shortsamples) {

    int sz;
    char padding[6] = { 0, 0, 0, 0, 0, 0 };
    SDIF_FrameHeader fh;
    SDIF_MatrixHeader tds, itds;

    /* setup the frame header stuff which we know in advance. */
    SDIF_Copy4Bytes(fh.frameType, "1TDS");
    fh.time = time_stamp;
    fh.streamID = stream_id;
    fh.matrixCount = 2;

    /* setup the ITDS matrix header stuff which we know in advance. */
    SDIF_Copy4Bytes(itds.matrixType, "ITDS");
    itds.matrixDataType = SDIF_FLOAT32;
    itds.rowCount = 1;
    itds.columnCount = 1;

    /* setup the 1TDS matrix header stuff which we know in advance. */
    SDIF_Copy4Bytes(tds.matrixType, "1TDS");
    tds.matrixDataType = SDIF_INT16;
    tds.rowCount = num_sampleframes;
    tds.columnCount = num_channels;

    /* compute the frame size. */
    sz = 2 * num_channels * num_sampleframes;
    fh.size = 16	/* frame header minus type and size. */
	+ 2*16		/* two matrix headers. */
	+ 8		/* the size of the ITDS matrix. */
	+ sz;		/* the size of the 1TDS matrix. */
    fh.size += SDIF_PaddingRequired(&tds);

    /* write out the frame header. */
    SDIF_WriteFrameHeader(&fh, outfp);

    /* write out the ITDS matrix header and the matrix itself. */
    SDIF_WriteMatrixHeader(&itds, outfp);
    SDIF_Write4(&sample_rate, 1, outfp);
    SDIF_Write4(padding, 1, outfp);

    /* write out the 1TDS matrix header. */
    SDIF_WriteMatrixHeader(&tds, outfp);

    /* write out the 1TDS matrix. */
    SDIF_Write2(shortsamples, num_sampleframes * num_channels, outfp);

    /* pad the 1TDS matrix. */
    SDIF_Write1(padding, SDIF_PaddingRequired(&tds), outfp);
}


int
main(int argc, char **argv) {
    int i, num_channels, num_frames, samples_per_frame, read_amt;
    int num_read;
    float time_per_frame = 0;
    char *infile, *outfile;
    SFHandle sf_handle;
    double sample_rate;
    sdif_float64 timestamp;
    FILE *sdif_handle;
    SDIFresult r;
    int convertToFloat = 0;
    sdif_int16 *shortsamples;
    sdif_float32 *floatsamples;

    /* parse the command line. */
    if (argc < 2) goto usage;

    for (i = 1; i < argc; ++i) {
	if (strcmp(argv[i], "-float") == 0) {
	    convertToFloat = 1;
	} else if (strcmp(argv[i], "-time_per_frame") == 0) {
	    if (i+1 < argc) {
		time_per_frame = atof(argv[i+1]);
		if (time_per_frame <= 0) {
		    fprintf(stderr,
			    "%s: time per frame must be positive.\n", argv[0]);
		    return 1;
		}
		++i;
	    } else goto usage;
	} else {
	    /* better be just input file and output file by now. */
	    if (i != argc - 2) goto usage;
	    infile = argv[i];
	    outfile = argv[i+1];
	    ++i;
	}
    }

    printf("in %s, out %s, time_per_frame %f, convertToFloat %d\n",
	   infile, outfile, time_per_frame, convertToFloat);

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

    r = SDIF_OpenWrite(outfile, &sdif_handle);
    if (r != ESDIF_SUCCESS) {
	fprintf(stderr, "%s: error opening \"%s\": %s\n",
		argv[0], outfile, SDIF_GetErrorString(r));
	return 1;
    }

    sample_rate = SFSampleRate(sf_handle);
    num_frames = SFNumFrames(sf_handle);
    num_channels = SFNumChannels(sf_handle);

    /* if time_per_frame is 0, put all the sample data in one SDIF frame. */
    samples_per_frame = time_per_frame * sample_rate;
    if (samples_per_frame == 0) {
	samples_per_frame = num_frames;
    }

    /* allocate space for the samples. */
    shortsamples = (short *) malloc(num_channels * samples_per_frame * 
				    sizeof(short));

    if (convertToFloat) {
	floatsamples = (float *) malloc(num_channels * samples_per_frame * 
				    sizeof(float));
    }

    /* do the conversion. */
    timestamp = 0;
    num_read = 0;
    while (num_frames > 0) {

	if (num_frames > samples_per_frame) read_amt = samples_per_frame;
	else read_amt = num_frames;

	if (SFReadInterleavedShorts(sf_handle, read_amt, shortsamples, num_read) == FALSE) {
	    fprintf(stderr, "Error reading samples: %s", 
		    SFGetLastErrorAsString());
	    exit(-1);
	}

	if (convertToFloat) {
	    float shortScaleFactor = 1.0f / -((float) SHRT_MIN);
	    for (i = 0; i < read_amt*num_channels; ++i) {
		floatsamples[i] = ((float) shortsamples[i]) * shortScaleFactor;
	    }

	    printf("* before FSTTF: last sample %f\n", 
		   floatsamples[(num_channels*read_amt)-1]);


	    FloatSamplesToTDSFrame(sdif_handle,
				   1,  /* arbitrary stream id. */
				   timestamp,
				   sample_rate,
				   num_channels,
				   read_amt,
				   floatsamples);
	} else {
	    ShortSamplesToTDSFrame(sdif_handle,
                                   1,  /* arbitrary stream id. */
                                   timestamp,
                                   sample_rate,
                                   num_channels,
                                   read_amt,
                                   shortsamples);
	}

	num_read += read_amt;
	num_frames -= read_amt;
	timestamp += (read_amt / sample_rate);

    }

    /* finish up. */
    SFCloseRead(sf_handle);
    SDIF_CloseWrite(sdif_handle);

    return 0;

usage:

    fprintf(stderr,  "Usage: %s [-float] [-time_per_frame <seconds>] " 
	    "<input file> <output file>\n", argv[0]);
    return 1;
}
