#ifndef NODE_UBUS_INCLUDED
#define NODE_UBUS_INCLUDED
#include <nan.h>

extern "C"
{
	#include <libubus.h>
	#include <json-c/json.h>
	#include <libubox/blobmsg.h>
	#include <libubox/blobmsg_json.h>
}

extern "C" {
	struct ubus_event_handler_extended
	{
		ubus_event_handler event_handler;
		void * data;
	};
}
namespace node_ubus {

class UBus : public Nan::ObjectWrap {
	public:
		UBus();
		static void Initialize(v8::Handle<v8::Object> target);
	private:
		struct ubus_context *ctx_;
		struct ubus_event_handler_extended listener_;
		uv_poll_t * poll_;
		static NAN_METHOD(New);
		static NAN_METHOD(Connect);
		static NAN_METHOD(Send);
		bool Connect(const char * type);
		bool Send(const char * type, const char * json_string);
		static void Callback(uv_poll_t *w, int status, int events);
	};
}
#endif
