#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "v4l.h"
#include "tojpg.h"
#include "grab.h"

using namespace v8;

Handle<Value> dump(const Arguments& args)
{
  unsigned char *ppm = NULL, *jpg = NULL;
  size_t ppmsize = 0, jpgsize = 0;
  char *errMsg = NULL;
  // remark to free device string on cleanup
  short freeDevString = 0;
  // default values...
  char *dev = (char*)"/dev/video0";
  size_t width = 640, height = 480, skipFrames = 10;

  HandleScope scope;

  if(args.Length() >= 1)
  {
    if(args[0]->IsString())
    {
      String::AsciiValue v(args[0]->ToString());
      dev = (char *)malloc(v.length() + 1);
      if(NULL == dev)
      {
        ThrowException(Exception::Error(String::New("Out of memory!")));
        scope.Close(Undefined());
      }
      else
      {
        strcpy(dev, *v);
        freeDevString = 1;
      }
    }
    else
    {
      ThrowException(Exception::Error(String::New("Invalid type for argument 1: String expected")));
      scope.Close(Undefined());
    }
  }

  if(args.Length() >= 2)
  {
    if(args[1]->IsInt32())
    {
      Local<Integer> i = Local<Integer>::Cast(args[1]);
      width = (int)(i->Int32Value());
    }
    else
    {
      if(freeDevString)
        free(dev);

      ThrowException(Exception::Error(String::New("Invalid type for argument 2: Integer expected")));
      scope.Close(Undefined());
    }
  }

  if(args.Length() >= 3)
  {
    if(args[2]->IsInt32())
    {
      Local<Integer> i = Local<Integer>::Cast(args[2]);
      height = (int)(i->Int32Value());
    }
    else
    {
      if(freeDevString)
        free(dev);

      ThrowException(Exception::Error(String::New("Invalid type for argument 3: Integer expected")));
      scope.Close(Undefined());
    }
  }
  if(args.Length() >= 4)
  {
    if(args[3]->IsInt32())
    {
      Local<Integer> i = Local<Integer>::Cast(args[3]);
      skipFrames = (int)(i->Int32Value());
    }
    else
    {
      if(freeDevString)
        free(dev);

      ThrowException(Exception::Error(String::New("Invalid type for argument 2: Integer expected")));
      scope.Close(Undefined());
    }
  }
  if(grab(dev, width, height, skipFrames, &ppm, &ppmsize, &errMsg) != 0)
  {
    if(freeDevString)
      free(dev);

    if(NULL != errMsg)
    {
      ThrowException(Exception::Error(String::New(errMsg)));
      free(errMsg);
    }
    else
    {
      ThrowException(Exception::Error(String::New("Unhandled error in grab()")));
    }
    return scope.Close(Undefined());
  }
  if(freeDevString)
    free(dev);

#ifdef DEBUG
  if(NULL != ppm)
  {
    FILE *f = fopen("test.ppm", "wb");
    if(NULL != f)
    {
      fwrite(ppm, ppmsize, 1, f);
      fflush(f);
      fclose(f);
    }
  }
#endif

  if(toJpeg(ppm, ppmsize, &jpg, &jpgsize, &errMsg) != 0)
  {
    if(NULL != errMsg)
    {
      ThrowException(Exception::Error(String::New(errMsg)));
      free(errMsg);
    }
    else
    {
      ThrowException(Exception::Error(String::New("Unhandled error in toJpeg()")));
    }
    return scope.Close(Undefined());
  }

#ifdef DEBUG
  if(NULL != jpg)
  {
    FILE *f = fopen("test.jpg", "wb");
    if(NULL != f)
    {
      fwrite(jpg, jpgsize, 1, f);
      fflush(f);
      fclose(f); 
    }
  }
#endif

  if(NULL != ppm)
  {
    free(ppm);
  }

  if(NULL != jpg)
  {
    node::Buffer *buffer = node::Buffer::New(jpgsize);
    memcpy(node::Buffer::Data(buffer), jpg, jpgsize);

    Local<Object> globalObj = Context::GetCurrent()->Global();
    Local<Function> ctor = Local<Function>::Cast(globalObj->Get(String::New("Buffer")));
    Handle<Value> ctorArgs[3] = { buffer->handle_, Integer::New(jpgsize), Integer::New(0) };
    Local<Object> result = ctor->NewInstance(3, ctorArgs);

    free(jpg);
    return scope.Close(result);
  }

  return scope.Close(Undefined());
}

void init(Handle<Object> exports)
{
  exports->Set(String::NewSymbol("dump"), FunctionTemplate::New(dump)->GetFunction());
}

NODE_MODULE(v4l, init);
