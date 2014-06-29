#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <turbojpeg.h>

int toJpeg(const unsigned char *ppmSource, size_t ppmSize, unsigned char **jpgDest, size_t *jpgSize, char **errMsg)
{
  tjhandle handle = NULL;
  int r = 0;
  unsigned char *p, *t;
  unsigned int width = -1, height = -1, maxColors = -1;
  unsigned long size = 0;

  unsigned char *localBuf = NULL;
  *jpgDest = NULL;
  unsigned char *jpg;

  localBuf = (unsigned char *)malloc(ppmSize);
  if(NULL == localBuf)
  {
    r = -ENOMEM;
    *errMsg = strdup("Error creating memory buffer for local ppm");
    goto toJpegErrorOut;
  }
  jpg = (unsigned char *)malloc(ppmSize);
  if(NULL == jpg)
  {
    r = -ENOMEM;
    *errMsg = strdup("Error creating memory buffer for jpg");
    goto toJpegErrorOut;
  }
  memset(jpg, 0, ppmSize);
  memcpy(localBuf, ppmSource, ppmSize);
  p = localBuf;

  // Check header
  if(*p != 'P' || *(p+1) != '6')
  {
    r = -EINVAL;
    goto toJpegErrorOut;
  }
  // Move to width index (after new line)
  p = p + 3;
  // Start of width pointer
  t = p;
  // Until space is not found
  while(*t != ' ') t++;
  // Terminate string
  *t = 0;
  // Grab the width as int
  width = atoi((const char *)p);
  // Set new start for height 
  p = ++t;
  // Until space is not found
  while(*t != ' ') t++;
  // Terminate string
  *t = 0;
  // Grab the width as int
  height = atoi((const char *)p);
  // Set new start for max color (per byte) 
  p = ++t;
  // Until new line is not found
  while(*t != '\n') t++;
  // Terminate string
  *t = 0;
  // Grab the max colors as int
  maxColors = atoi((const char *)p);
  // Santity check...
  if(width > 4000 || height > 4000 || maxColors > 255)
  {
    r = -EINVAL;
    *errMsg = strdup("Error with dimensions!");
    goto toJpegErrorOut;
  }
  // Now we have the raw image data pointer
  p = ++t;
  // Init compressor
  handle = tjInitCompress();
  if(NULL == handle)
  {
    r = -EINVAL;
    *errMsg = strdup("Error initializing jpeg compressor!");
    goto toJpegErrorOut;
  }
  // Start compressing
  if(tjCompress2(handle, p, width, 0, height, TJPF_RGB, &jpg, &size, TJSAMP_444, 60, TJFLAG_NOREALLOC) == -1)
  {
    r = -EINVAL;
    *errMsg = strdup("Error performing jpeg compressor!");
    goto toJpegErrorOut;
  }
  // Cleanup
  tjDestroy(handle);
  free(localBuf);
  // Prepare results
  *jpgSize = (size_t)size;
  *jpgDest = jpg;
  // Finally everything is well
  return r;

toJpegErrorOut:
  // Ouch! we had an error, cleanup is necessary
  if(NULL != handle)
    tjDestroy(handle);
  if(NULL != localBuf)
    free(localBuf);
  if(NULL != jpg)
    free(jpg);

  return r;
}
