#include <stdio.h>

#include "sdif.h"
#include "sdifu.h"


int
main() {

    sdif_uint32 *previous_image;
    sdif_uint32 *current_image;
    FILE *outfp;


    // test case 1: null previous image.
    current_image = new sdif_uint32 [25];
    for (int i=0; i < 5; i++) {
	for (int j=0; j < 5; j++) {
	    current_image[5*i+j] = (i==j);
	}
    }
    outfp = SDIF_OpenWrite("test01.sdif");
    SDIFU_ImageToVIDFrame(outfp, 1234, 0, 5, 5, NULL, current_image);
    SDIF_CloseWrite(outfp);
    delete current_image;


    // test case 2: previous and current image are completely different.
    previous_image = new sdif_uint32 [25];
    current_image = new sdif_uint32 [25];
    for (int i=0; i < 25; i++) {
	previous_image[i] = 0;
	previous_image[i] = 0xffffffff;
    }
    outfp = SDIF_OpenWrite("test02.sdif");
    SDIFU_ImageToVIDFrame(outfp, 1234, 0, 5, 5, previous_image, current_image);
    SDIF_CloseWrite(outfp);
    delete previous_image, current_image;


    // test case 3: previous and current image are identical.
    previous_image = new sdif_uint32 [25];
    current_image = new sdif_uint32 [25];
    for (int i=0; i < 25; i++) {
	previous_image[i] = current_image[i] = 0;
    }
    outfp = SDIF_OpenWrite("test03.sdif");
    SDIFU_ImageToVIDFrame(outfp, 1234, 0, 5, 5, previous_image, current_image);
    SDIF_CloseWrite(outfp);
    delete previous_image, current_image;

    return 0;

}
