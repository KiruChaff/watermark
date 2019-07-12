#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "stb/stb_image_resize.h"

// returns the minimum value of two arguments
#define min(a,b)\
   ({ __typeof__(a) _a = a;\
      __typeof__(b) _b = b;\
      _a > _b ? _b : _a;})


//define the mode of the program -- set a new Watermark(default): #define SET_WATERMARK - remove imprinted watermark: #define REMOVE_WATERMARK
//#define REMOVE_WATERMARK

#ifndef REMOVE_WATERMARK
#define SET_WATERMARK
#endif

// prints the pixels of the converted integer array on the screen -- debugging purposes only --
void showPixels(unsigned int p[100][100]){
        for (int i=0; i<100; i++) {
                for(int j=0; j<100; j++)
                        printf("%06x ", p[i][j]);
                printf("\n");
        }
}

// filters red out of an 8-bit integer
unsigned char red(int color){
        return color>>16;
}

// filters green out of an 8-bit integer
unsigned char green(int color){
        if (color>>16<<16)
                return (color%(color>>16<<16))>>8;
        return color>>8;
}

// filters blue out of an 8-bit integer
unsigned char blue(int color){
        if (color>>8<<8)
                return color%(color>>8<<8);
        return color;
}

// converts an unsigned character array consisting of three units per color to an integer array
void convertToIntArray(unsigned char *img, int width, int height, unsigned int pix[height][width]){
        for (int i=0; i<height; i++)
                for (int j=0; j<width; j++) {
                        pix[i][j]=*(img+((i*width+j)*3))<<16|*(img+((i*width+j)*3)+1)<<8|*(img+((i*width+j)*3)+2);
                }

}

// takes the average of the two colors
int additiveColorMixing(int r1, int g1, int b1, int r2, int g2, int b2){
        int result = 0;
        result += min((r1 + r2)/2,255);
        result = result << 8;
        result += min((g1 + g2)/2, 255);
        result = result << 8;
        result += min((b1 + b2)/2, 255);
        return result;
}

int reverseAdditiveColorMixing(int r1, int g1, int b1, int r2, int g2, int b2){
   int result = 0;
   result += min(2*r1 - r2,255);
   result = result << 8;
   result += min(2*g1 - g2, 255);
   result = result << 8;
   result += min(2*b1 - b2, 255);
   return result;
}

int main(){

        // loading two images
        int x1,y1,n1, init_x2, init_y2, n2;
        unsigned char *data = stbi_load("stockimage.jpg", &x1, &y1, &n1, 3);
        unsigned char *watermark = stbi_load("smiley.jpg", &init_x2, &init_y2, &n2, 3);

        int x2, y2;

        // resize watermark to a 1:3 ratio in height
        y2 = (int) y1/5;
        x2 = (int) init_x2 *  y2 / init_y2;

        if (y2 > init_y2) {
           y2 = init_y2;
           x2 = init_x2;
        } else
          stbir_resize_uint8(watermark, init_x2, init_y2, 0, watermark, x2, y2, 0, n2);


        unsigned int pixels[y2][x2];

        // if the two images were successfuly loaded
        if (data && watermark) {
                //converting three byte channel into integer whilst reasorting to a two dimensional array
                convertToIntArray(watermark, x2, y2, pixels);

                // merging two images
                int newColor;
                // iterate through every pixel of the original image (each pixel has three channels)
                for (int i=0; i<x1*y1*3; i+=3) {
                        // evaluate the indecies of the watermark -- looping around
                        int y = (i/(3*x1))%y2;
                        int x = ((i/3)%x1)%x2;
                        // adds pixel of watermark if it's not white
                        int watermarkPixel = pixels[y][x];
                        // the threshold is carefully chosen so that with a black and white text only the letters are rendered
                        unsigned char threshold = 0x33;
                        if (red(watermarkPixel) < threshold || green(watermarkPixel) < threshold || blue(watermarkPixel) < threshold) {
                              #ifdef SET_WATERMARK
                                newColor = additiveColorMixing(*(data+i), *(data+i+1),*(data+i+2),red(pixels[y][x]),green(pixels[y][x]),blue(pixels[y][x]));
                              #elif defined REMOVE_WATERMARK
                                newColor = reverseAdditiveColorMixing(*(data+i), *(data+i+1),*(data+i+2),red(pixels[y][x]),green(pixels[y][x]),blue(pixels[y][x]));
                              #endif
                                // update the newly evaluated color
                                *(data+i)=red(newColor);
                                *(data+i+1)=green(newColor);
                                *(data+i+2)=blue(newColor);
                        }
                }
                // write the new image
                stbi_write_jpg("output/resultingimage.jpg", x1, y1, n1, data, 100);
        }
        // remove negative sign: *hex-num*^0xff000000
        return 0;
}
