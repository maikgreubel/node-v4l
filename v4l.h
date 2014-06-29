#ifndef NODE_V4L
#define NODE_V4L

#include <node.h>

class V4L : public node::ObjectWrap
{
	public:
		static void Init(v8::Handle<v8::Object> exports);

	private:
		explicit V4L();
		~V4L();

		static v8::Handle<v8::Value> New(const v8::Arguments& args);
		static v8::Handle<v8::Value> Open(const v8::Arguments& args);
		static v8::Persistent<v8::Function> constructor;
};

#endif
