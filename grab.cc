#include <errno.h>
#include <fcntl.h>
#include <libv4l2.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/videodev2.h>

#include <sys/mman.h>
#include <sys/stat.h>

#define MAX_TRIES 30

struct ioBuffer
{
  void *start;
  size_t len;
};

static int grabIoctl(int fd, int req, void *args, char **errMsg)
{
  int r = 0, num_tries = 0;
  char err[1024];
  do
  {
    r = v4l2_ioctl(fd, req, args);
    num_tries++;
  } while(r == -1 && ((errno == EAGAIN) || (errno == EINTR)) && num_tries < MAX_TRIES);

  if(r == -1)
  {
    memset(err, 0, sizeof(err));
    sprintf(err, "ioctl: %s", strerror(errno));
    *errMsg = strdup(err);
    return -1;
  }

  return 0;
}

int grab(const char *devName, size_t w, size_t h, size_t skipFrames, unsigned char **ppmDest, size_t *ppmSize, char **errMsg)
{
  int fd = -1, r = 0, io = 0;
  struct stat                 stb;
  struct v4l2_format          format;
  struct v4l2_buffer          buffer;
  struct v4l2_requestbuffers  request;
  enum v4l2_buf_type          bufferType;
  fd_set                      fdset;
  struct timeval              tval;
  struct ioBuffer            *ioBuffers = NULL;
  unsigned int                numBuffers = 0, i = 0, headerLen = 0, dataLen = 0;
  unsigned char              *p = NULL, *ppm = NULL;
  char                        header[20] = {0};

  *ppmDest = NULL;

  if((r = stat(devName, &stb)) != 0)
  {
    *errMsg = strdup(strerror(errno));
    goto grabErrorOut;
  }

  if((r = access(devName, R_OK)) != 0)
  {
    *errMsg = strdup(strerror(errno));
    goto grabErrorOut;
  }
  
  fd = v4l2_open(devName,  O_RDWR | O_NONBLOCK, 0);
  if(fd < 0)
  {
    r = -ENODEV;
    *errMsg = strdup("Could not open device!");
    goto grabErrorOut;
  }

  memset(&format, 0, sizeof(struct v4l2_format));
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  format.fmt.pix.width = w;
  format.fmt.pix.height = h;
  format.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
  format.fmt.pix.field       = V4L2_FIELD_INTERLACED;

  if(grabIoctl(fd, VIDIOC_S_FMT, &format, errMsg) < 0)
  {
    r = -EIO;
    goto grabErrorOut;
  }

  if(format.fmt.pix.pixelformat != V4L2_PIX_FMT_RGB24)
  {
    r = -EINVAL;
    *errMsg = strdup("Wrong pixel format!");
    goto grabErrorOut;
  }

  memset(&request, 0, sizeof(struct v4l2_requestbuffers));
  request.count = 2;
  request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  request.memory = V4L2_MEMORY_MMAP;

  if(grabIoctl(fd, VIDIOC_REQBUFS, &request, errMsg) < 0)
  {
    r = -EIO;
    goto grabErrorOut;
  }

  ioBuffers = (struct ioBuffer *)calloc(request.count, sizeof(struct ioBuffer));
  for(numBuffers = 0; numBuffers < request.count; numBuffers++)
  {
    memset(&buffer, 0, sizeof(struct v4l2_buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = numBuffers;

    if(grabIoctl(fd, VIDIOC_QUERYBUF, &buffer, errMsg))
    {
      r = -EIO;
      goto grabErrorOut;
    }

    ioBuffers[numBuffers].len = buffer.length;
    ioBuffers[numBuffers].start = v4l2_mmap(NULL, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buffer.m.offset);

    if(MAP_FAILED == ioBuffers[numBuffers].start)
    {
      r = -ENOMEM;
      *errMsg = strdup("Error mapping the video memory into user space!");
      goto grabErrorOut;
    }
  }

  for(i = 0; i < numBuffers; i++)
  {
    memset(&buffer, 0, sizeof(struct v4l2_buffer));

    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = i;

    if(grabIoctl(fd, VIDIOC_QBUF, &buffer, errMsg) < 0)
    {
      r = -EIO;
      goto grabErrorOut;
    }
  }

  bufferType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if(grabIoctl(fd, VIDIOC_STREAMON, &bufferType, errMsg) < 0)
  {
    r = -EIO;
    goto grabErrorOut;
  }

  for(i = 0; i < skipFrames; i++)
  {
    do
    {
      FD_ZERO(&fdset);
      FD_SET(fd, &fdset);
      tval.tv_sec = 1;
      tval.tv_usec = 0;
      io = select(fd + 1, &fdset, NULL, NULL, &tval);
    } while((io == -1) && (errno == EINTR));
  
    if(-1 == io)
    {
      r = -EBUSY;
      *errMsg = strdup("Error retrieving stream of device!");
      goto grabErrorOut;
    }
    memset(&buffer, 0, sizeof(struct v4l2_buffer));

    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    if(grabIoctl(fd, VIDIOC_DQBUF, &buffer, errMsg) < 0)
    {
      r = -EIO;
      goto grabErrorOut;
    }

    if(i == skipFrames - 1)
    {
      memset(header, 0, sizeof(header));
      sprintf(header, "P6\n%d %d 255\n", format.fmt.pix.width, format.fmt.pix.height);
      headerLen = strlen(header);
      dataLen = buffer.bytesused;
      ppm = (unsigned char *)malloc(headerLen + dataLen);
      if(NULL == ppm)
      {
        r = -ENOMEM;
        *errMsg = strdup("Error allocating memory for destination buffer!");
        goto grabErrorOut;
      }
      memset(ppm, 0, headerLen + dataLen);
      p = ppm;

      memcpy(p, header, headerLen);
      memcpy(p + headerLen, ioBuffers[buffer.index].start, buffer.bytesused);
    }

    if(grabIoctl(fd, VIDIOC_QBUF, &buffer, errMsg) < 0 )
    {
      r = -EIO;
      goto grabErrorOut;
    }
  }

  // Prepare results
  *ppmSize = headerLen + dataLen;
  *ppmDest = ppm;

  bufferType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if(grabIoctl(fd, VIDIOC_STREAMOFF, &bufferType, errMsg) < 0)
  {
    r = -EIO;
    goto grabErrorOut;
  }

  for(i = 0; i < numBuffers; i++)
  {
    v4l2_munmap(ioBuffers[i].start, ioBuffers[i].len);
  }
  
  free(ioBuffers);

  v4l2_close(fd);

  return r;

grabErrorOut:
  if(NULL != ioBuffers)
    free(ioBuffers);
  if(NULL != *ppmDest)
    free(*ppmDest);
  return r;
}
