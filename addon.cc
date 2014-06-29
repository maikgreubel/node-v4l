#include <node.h>
#include "v4l.h"

using namespace v8;

void InitAll(Handle<Object> exports)
{
	V4L::Init(exports);
}

NODE_MODULE(addon, InitAll);
