#ifndef V4L_GRAB
#define V4L_GRAB

int grab(const char *devName, size_t w, size_t h, size_t skipFrames, unsigned char **ppmDest, size_t *ppmSize, char **errMsg);

#endif
