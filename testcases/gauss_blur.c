
//#include <stdlib.h>
// TODO: stddef.h not found.

int main(){



              
        char Gauss[] = {99, 68, 35, 10};
        int g_acc;
        int tot = 0;
        int i, k = 0, l, x, y;

        unsigned char** g_tmp_image;
        unsigned char * gauss_image;
        unsigned char * outputImage;
        unsigned char * inputImage;

        int pictureHight = 50;
        int pictureWidth = 50;

        const int GB = 1;
        const int NB = 1;

        char maxdiff, val;

        gauss_image = (unsigned char*) malloc(pictureHight * pictureWidth * sizeof (unsigned char));
        outputImage = (unsigned char*) malloc(pictureHight * pictureWidth * sizeof (unsigned char));
        g_tmp_image = (unsigned char **) malloc(pictureWidth * sizeof (unsigned char*));
        inputImage = (unsigned char*) malloc(pictureHight * pictureWidth * sizeof (unsigned char));


        for (i = 0; i < pictureHight * pictureWidth; i++) {
            inputImage[i] = k;
            k++;
            if (k == 256) {
                k = 0;
            }
        }

        //Perform Gauss operations on image
        for (i = 0; i < pictureWidth; i++) {
            *(g_tmp_image + i) = (unsigned char*) malloc(pictureHight * sizeof (unsigned char));
        }

        for (k = -GB; k <= GB; k++) {
            tot += Gauss[abs(k)];
        }

        /*
         Horizontal Gauss blur*/
        for (x = 0; x < pictureWidth; x++) {
            for (y = 0; y < pictureHight; y++) {
                *(*(g_tmp_image + x) + y) = 0;
            }
        }

        for (x = GB; x < pictureWidth - GB; x++) {
            for (y = GB; y < pictureHight - GB; y++) {
                g_acc = 0;

                for (k = -GB; k <= GB; k++) {
                    g_acc += inputImage[x + k + pictureWidth * y] * Gauss[abs(k)];
                }
                *(*(g_tmp_image + x) + y) = g_acc / tot;
            }
        }

        //Vertival Gauss blur
        for (i = 0; i < pictureWidth * pictureHight; i++) {
            gauss_image[i] = 0;
        }

        for (x = GB; x < pictureWidth - GB; x++) {
            for (y = GB; y < pictureHight - GB; y++) {
                g_acc = 0;
                for (k = -GB; k < GB; k++) {
                    g_acc += (*(*(g_tmp_image + x) + y + k)) * Gauss[abs(k)];

                }
                gauss_image[x + y * pictureWidth] = g_acc / tot;
            }
        }


        for (i = 0; i < pictureWidth * pictureHight; i++) {
            outputImage[i] = 0;
        }

        for (x = NB; x < pictureWidth - NB; x++) {
            for (y = NB; y < pictureHight - NB; y++) {
                maxdiff = 0;
                for (k = -NB; k <= NB; k++) {
                    for (l = -NB; l <= NB; l++) {
                        if (k != 0 || l != 0) {
                            val = abs(((int) gauss_image[x + k + pictureWidth * (y + 1)] -
                                    gauss_image[x + pictureWidth * y]));
                            if (val > maxdiff) {
                                maxdiff = val;
                            }
                        }
                    }
                }

                outputImage[x + pictureWidth * y] = maxdiff;
            }
        }

        /*Memory deallocation*/
        free(gauss_image);
        free(outputImage);
        free(inputImage);

        for (i = 0; i < pictureWidth; i++) {
            free(*(g_tmp_image + i));
        }

        free(g_tmp_image);


    return 0;
}
    
