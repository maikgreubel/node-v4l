#ifndef V4L_TO_JPG
#define V4L_TO_JPG

int toJpeg(const unsigned char *ppmSource, size_t ppmSize, unsigned char **jpgDest, size_t *jpgSize, char **errMsg);

#endif
