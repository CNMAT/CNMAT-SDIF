/*

  sdifu.c --

  Implementation of the functions prototyped in sdifu.h.  Unless you
  are making changes to the library, you shouldn't need to look at this.

  Sami Khoury, 11/5/1998

  

*/


#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "sdifu.h"

static int error_code = ESDIFU_NONE;
static char *error_string_array[ESDIFU_NUM_ERRORS] = {
    "Everything's cool",
    (char *) NULL,  /* this will be set to strerror(errno). */
    (char *) NULL,  /* this will be set to SDIF_GetLastErrorString(). */
    "Sinusoid born with non-zero amplitude",
    "Sinusoid died with non-zero amplitude"
};


/* prototypes for structs and functions used only in this file. */

typedef struct viiNode {
    sdif_uint32 type;
    sdif_uint32 num_pixels;
    sdif_uint32 *pixels;
    struct viiNode *next;
} viiNode;

static void set_error_code(int code);
static streamIndexNode * new_streamIndexNode(void);
static void update_stream_list(streamIndexNode *list, SDIF_FrameHeader *f);
static char * append_string(const char *head, const char *tail);
static viiNode * new_viiNode(void);
static viiNode * create_sustain_node(sdif_int32 num_pixels,
				     sdif_uint32 *previous_image,
				     sdif_uint32 *current_image,
				     sdif_uint32 *pos);
static viiNode * create_refresh_node(sdif_int32 num_pixels,
				     sdif_uint32 *previous_image,
				     sdif_uint32 *current_image,
				     sdif_uint32 *pos);
static void destroy_viiNode_list(viiNode *list);


SDIFU_SanityCheckFunc
SDIFU_GetSanityCheckFunc(char frame_type[4]) {

    char *sdif_lib;
    char *frames, *lib, *tail, *p, *q;
    char *full_path_name;
    int total_length;
    void *dl_handle;

    frames = "/frames/";
    lib = "/lib";
    tail = "_SanityCheck.so";

    if ((sdif_lib = getenv("SDIF_LIB")) == NULL) {
	sdif_lib = "/usr/local/sdif/lib";
    }

    /* sdif_lib + frames + frame_type + lib + frame_type + tail + null */
    total_length = strlen(sdif_lib) + 8 + 4 + 4 + 4 + 15 + 1;

    /* build the full pathname to the shared library. */
    full_path_name = malloc(total_length * sizeof(char));
    strcpy(full_path_name, sdif_lib);
    p = &full_path_name[strlen(sdif_lib)];
    strcpy(p, frames);
    p += strlen(frames);
    strncpy(p, frame_type, 4);
    p += 4;
    strcpy(p, lib);
    p += strlen(lib);
    strncpy(p, frame_type, 4);
    p += 4;
    strcpy(p, tail);

    /* try to open it. */
    if ((dl_handle = dlopen(full_path_name, RTLD_LAZY)) == NULL) return NULL;

    /* build the function name. */
    p = strrchr(full_path_name, '/');
    p += 3;
    *p = '_';
    q = strrchr(p, '.');
    *q = '\0';

    /* search for the sanity check function. */
    return (SDIFU_SanityCheckFunc) dlsym(dl_handle, q);

}


streamIndexNode *
SDIFU_GetIndexFromFile(const char *f) {

    streamIndexNode *s;
    FILE *fp;

    if ((fp = SDIF_OpenRead(f)) == NULL) {
	set_error_code(ESDIFU_SEE_ESDIF);
	return NULL;
    }

    s = SDIFU_GetIndexFromHandle(fp);
    SDIF_CloseRead(fp);
    return s;

}


streamIndexNode *
SDIFU_GetIndexFromHandle(FILE *f) {

    streamIndexNode *stream_list = NULL;
    SDIF_FrameHeader fh;

    if (SDIF_ReadFrameHeader(&fh, f) != 1) return NULL;
    if (SDIF_SkipFrame(&fh, f) != 1) {
	set_error_code(ESDIFU_SEE_ESDIF);
	return NULL;
    }

    stream_list = new_streamIndexNode();
    stream_list->id = fh.streamID;
    SDIF_Copy4Bytes(stream_list->frameType, fh.frameType);
    stream_list->first = stream_list->last = fh.time;
    
    /* for each frame in the SDIF file, append a node to the stream list
       if it is the first frame of a stream, or update the time of the last
       frame for that stream. */
    while (SDIF_ReadFrameHeader(&fh, f) == 1) {
	update_stream_list(stream_list, &fh);
	if (SDIF_SkipFrame(&fh, f) != 1) {
	    set_error_code(ESDIFU_SEE_ESDIF);
	    return NULL;
	}
    }

    return stream_list;

}


void
SDIFU_DestroyStreamIndex(streamIndexNode *index) {

    streamIndexNode *p, *q;

    assert(index != NULL);

    p = q = index;
    while (p != NULL) {
	q = p->next;
	free(p);
	p = q;
    }

}


char *
SDIFU_GetIndexStringFromFile(const char *f) {

    char *s;
    FILE *fp;

    if ((fp = SDIF_OpenRead(f)) == NULL) {
	set_error_code(ESDIFU_SEE_ESDIF);
	return NULL;
    }

    s = SDIFU_GetIndexStringFromHandle(fp);
    SDIF_CloseRead(fp);
    return s;

}


char *
SDIFU_GetIndexStringFromHandle(FILE *f) {

    char buf[100];
    char *prev = NULL, *index = NULL;
    streamIndexNode *p = NULL, *stream_list = NULL;

    stream_list = SDIFU_GetIndexFromHandle(f);
    p = stream_list;
    while (p != NULL) {

	sprintf(buf, "stream_id:%d,", p->id);
	prev = index;
	index = append_string(index, buf);
	if (prev != NULL) free(prev);

	sprintf(buf, "frame_type:%c%c%c%c,",
		p->frameType[0], p->frameType[1],
		p->frameType[2], p->frameType[3]);
	prev = index;
	index = append_string(index, buf);
	free(prev);

	sprintf(buf, "first_time:%f,", p->first);
	prev = index;
	index = append_string(index, buf);
	free(prev);

	sprintf(buf, "last_time:%f;\n", p->last);
	prev = index;
	index = append_string(index, buf);
	free(prev);

	p = p->next;

    }

    SDIFU_DestroyStreamIndex(stream_list);

    return index;

}


int
SDIFU_ImageToVIDFrame(FILE *outfp,
		      sdif_int32 stream_id,
		      sdif_float32 time_stamp,
		      sdif_int32 height, sdif_int32 width,
		      sdif_uint32 *previous_image,
		      sdif_uint32 *current_image) {

    int i, num_pixels = width*height;
    sdif_uint32 pos;
    char padding[4] = { 0, 0, 0, 0 };
    viiNode *block_list, *last;
    SDIF_FrameHeader fh;
    SDIF_MatrixHeader mh_vir, mh_vii;

    assert(outfp != NULL);
    assert(current_image != NULL);
    assert(height > 0);
    assert(width > 0);

    /* setup the frame header stuff which we know in advance. */
    SDIF_Copy4Bytes(fh.frameType, "1VID");
    fh.time = time_stamp;
    fh.streamID = stream_id;
    fh.matrixCount = 2;

    /* setup the 1VIR matrix header stuff which we know in advance. */
    SDIF_Copy4Bytes(mh_vir.matrixType, "1VIR");
    mh_vir.matrixDataType = SDIF_UINT32;
    mh_vir.rowCount = 1;
    mh_vir.columnCount = 2;

    /* setup the 1VII matrix header stuff which we know in advance. */
    SDIF_Copy4Bytes(mh_vii.matrixType, "1VII");
    mh_vii.matrixDataType = SDIF_UINT32;
    mh_vii.rowCount = 1;

    /* if the previous image is NULL the encoding is a single refresh block. */
    if (previous_image == NULL) {

	/* compute the size of the entire frame and write out the header. */
	fh.size = 16	/* frame header minus type and size. */
	    + 2*16	/* two matrix headers. */
	    + 8		/* the size of the VIR matrix. */
	    + 4		/* for VII_REFRESH */
	    + 4		/* for the number of pixels */
	    + 4*height*width;  /* sizeof(sdif_uint32) * number of pixels. */
	if (((fh.size % 8) / 4) == 1) fh.size += 4;
	SDIF_WriteFrameHeader(&fh, outfp);

	/* write out the VIR matrix header and data. */
	SDIF_WriteMatrixHeader(&mh_vir, outfp);
	SDIF_Write4(&height, 1, outfp);
	SDIF_Write4(&width, 1, outfp);

	/* write out the VII matrix header and data. */
	mh_vii.columnCount = 2  /* for VII_REFRESH and num_pixels */
	    + num_pixels;
	SDIF_WriteMatrixHeader(&mh_vii, outfp);
	i = VII_REFRESH;
	SDIF_Write4(&i, 1, outfp);
	SDIF_Write4(&num_pixels, 1, outfp);
	for (i=0; i < num_pixels; i++) {
	    SDIF_Write4(&current_image[i], 1, outfp);
	}

	return 1;

    }

    /* otherwise, the encoding of "current_image" into the VII matrix is
       going to be a sequence of sustain and refresh blocks.  since the
       size of the entire frame must be written into the frame header,
       build the encoding into a linked list, traverse the list to compute
       the size, and then write out the frame header and the two matrices. */

    /* initialize the block list. */
    block_list = (current_image[0] == previous_image[0]) ?
	create_sustain_node(num_pixels, previous_image, current_image, &pos) :
	create_refresh_node(num_pixels, previous_image, current_image, &pos);
    last = block_list;

    /* encode the rest of the image. */
    while (pos < num_pixels) {
	last->next = (current_image[pos] == previous_image[pos]) ?
	    create_sustain_node(num_pixels,
				&previous_image[pos], &current_image[pos],
				&pos) :
	    create_refresh_node(num_pixels,
				&previous_image[pos], &current_image[pos],
				&pos);
	last = last->next;
    }

    /* calculate the column count for the VII matrix. */
    i = 0;
    last = block_list;
    while (last != NULL) {
	i += 8  /* size of block type and pixel count value. */
	    + 4*last->num_pixels;
	last = last->next;
    }
    mh_vii.columnCount = i/4;

    /* calculate the frame's size. */
    fh.size = 16	/* frame header minus type and size. */
	+ 2*16		/* two matrix headers. */
	+ 8		/* the size of the VIR matrix. */
	+ i;
    if (((fh.size % 8) / 4) == 1) fh.size += 4;

    /* write it all out. */
    SDIF_WriteFrameHeader(&fh, outfp);
    SDIF_WriteMatrixHeader(&mh_vir, outfp);
    SDIF_Write4(&height, 1, outfp);
    SDIF_Write4(&width, 1, outfp);
    SDIF_WriteMatrixHeader(&mh_vii, outfp);
    last = block_list;
    while (last != NULL) {
	SDIF_Write4(&last->type, 1, outfp);
	SDIF_Write4(&last->num_pixels, 1, outfp);
	if (last->type == VII_REFRESH) {
	    for (i=0; i < last->num_pixels; i++) {
		SDIF_Write4(&last->pixels[i], 1, outfp);
	    }
	}
	last = last->next;
    }
    if (((fh.size % 8) / 4) == 1) SDIF_Write4(padding, 1, outfp);

    destroy_viiNode_list(block_list);

    return 1;

}


int
SDIFU_GetLastErrorCode(void) {
    return error_code;
}


char *
SDIFU_GetLastErrorString(void) {
    return error_string_array[error_code];
}



/* static function definitions follow. */


static void
set_error_code(int code) {
    error_code = code;
    if (code == 1) error_string_array[1] = strerror(errno);
    if (code == 2) error_string_array[2] = SDIF_GetLastErrorString();
}


static streamIndexNode *
new_streamIndexNode(void) {

    char c[4] = { 0, 0, 0, 0 };
    streamIndexNode *s = (streamIndexNode *) malloc(sizeof(streamIndexNode));
    s->id = 0;
    SDIF_Copy4Bytes(s->frameType, c);
    s->first = s->last = 0;
    s->next = NULL;
    return s;

}


static void
update_stream_list(streamIndexNode *list, SDIF_FrameHeader *f) {

    streamIndexNode *p, *prev;

    assert(list != NULL);
    assert(f != NULL);

    /* try to find the streamID of this frame in the list. */
    p = prev = list;
    while (p != NULL) {

	/* here it is, so just update the time of the last frame and return. */
	if (p->id == f->streamID) {
	    p->last = f->time;
	    return;
	}

	prev = p;
	p = p->next;

    }

    /* this must be a new stream, so add a node to the list. */
    prev->next = new_streamIndexNode();
    p = prev->next;
    p->id = f->streamID;
    SDIF_Copy4Bytes(p->frameType, f->frameType);
    p->first = p->last = f->time;

    return;

}


static char *
append_string(const char *head, const char *tail) {

    int i, j, h_len, t_len;
    char *s;

    if (head == NULL) h_len = 0;
    else h_len = strlen(head);

    if (tail == NULL) t_len = 0;
    else t_len = strlen(tail);

    s = (char *) malloc(h_len + t_len + 1);
    for (i=0; i < h_len; i++) s[i] = head[i];
    for (j=0; j < t_len; j++) s[i++] = tail[j];
    s[i] = '\0';

    return s;

}







static viiNode *
new_viiNode(void) {

    viiNode *v;
    if ((v = (viiNode *) malloc(sizeof(viiNode))) == NULL) return NULL;
    v->type = -1;
    v->num_pixels = 0;
    v->pixels = NULL;
    v->next = NULL;
    return v;

}


static viiNode *
create_sustain_node(sdif_int32 num_pixels,
		    sdif_uint32 *previous_image, sdif_uint32 *current_image,
		    sdif_uint32 *pos) {

    int j=0;
    viiNode *v;

    assert(num_pixels > 0);
    assert(previous_image != NULL);
    assert(current_image != NULL);
    assert(pos != NULL);

    if ((v = new_viiNode()) == NULL) return NULL;
    v->type = VII_SUSTAIN;
    while ((j < num_pixels) && (previous_image[j] == current_image[j])) j++;
    v->num_pixels = j;
    *pos = j+1;
    return v;

}


static viiNode *
create_refresh_node(sdif_int32 num_pixels,
		    sdif_uint32 *previous_image, sdif_uint32 *current_image,
		    sdif_uint32 *pos) {

    int i, j=0;
    viiNode *v;

    assert(num_pixels > 0);
    assert(previous_image != NULL);
    assert(current_image != NULL);
    assert(pos != NULL);

    if ((v = new_viiNode()) == NULL) return NULL;
    v->type = VII_REFRESH;
    while ((j < num_pixels) && (previous_image[j] != current_image[j])) j++;
    v->num_pixels = j;
    v->pixels = (sdif_uint32 *) malloc(j * sizeof(sdif_uint32));
    if (v->pixels == NULL) return NULL;
    for (i=0; i < j; i++) v->pixels[i] = current_image[i];
    *pos = j+1;
    return v;

}


static void
destroy_viiNode_list(viiNode *list) {

    viiNode *p, *q;

    assert(list != NULL);

    p = list;
    while (p != NULL) {
	q = p;
	p = p->next;
	if (q->pixels != NULL) free(q->pixels);
	free(q);
    }

}
