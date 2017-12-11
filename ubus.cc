#include <nan.h>

extern "C"
{
	#include <libubus.h>
	#include <json-c/json.h>
	#include <libubox/blobmsg.h>
	#include <libubox/blobmsg_json.h>
}
#include "ubus.hpp"
using namespace v8;

namespace node_ubus {
	static struct blob_buf b;  //must be declared as global static
	static void receive_ubus_event(struct ubus_context *ctx, struct ubus_event_handler *ev,
	                          const char *type, struct blob_attr *msg)
	{
		struct ubus_event_handler_extended  * ev_ext = (struct ubus_event_handler_extended * )ev;

		UBus * ubus = (UBus * )ev_ext->data;

		Local<Value> callback_v = Nan::Get(ubus->handle(), Nan::New("callback").ToLocalChecked()).ToLocalChecked();
	    if (!callback_v->IsFunction()) {
	        return;
	    }

	    Local<Function> callback = Local<Function>::Cast(callback_v);
	    Local<Value> argv[2];

	    char *str = blobmsg_format_json(msg, true);
	    if (str == NULL)
	    	return;
	    argv[0] = Nan::New(type).ToLocalChecked();
	    argv[1] = Nan::New(str).ToLocalChecked();
	    free(str);
	    Nan::MakeCallback(ubus->handle(), callback, 2, argv);
	}

	UBus::UBus() : ctx_(0), poll_(0) {
    }

    void
    UBus::Initialize(Handle<Object> target) {
        Local<FunctionTemplate> t = Nan::New<FunctionTemplate>(New);

        Local<String> symbol = Nan::New("UBus").ToLocalChecked();
        t->SetClassName(symbol);
        t->InstanceTemplate()->SetInternalFieldCount(1);
        Nan::SetPrototypeMethod(t, "connect", UBus::Connect);
        Nan::SetPrototypeMethod(t, "send", UBus::Send);
        Nan::Set(target, symbol, Nan::GetFunction(t).ToLocalChecked());
    }

    NAN_METHOD(UBus::New) {
        UBus *u = new UBus();
        u->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }

    NAN_METHOD(UBus::Connect) {
        UBus *ubus = Nan::ObjectWrap::Unwrap<UBus>(info.Holder());

        if (info.Length() != 1 || !info[0]->IsString())
		{
			return Nan::ThrowSyntaxError("Usage: Connect(type)");
		}

		//return Nan::ThrowSyntaxError("Usage: Connect(type) passed");
		String::Utf8Value type(info[0]->ToString());
		if (!ubus->Connect(*type) )
    		return Nan::ThrowSyntaxError("Connect to UBUS failed");
    }

    NAN_METHOD(UBus::Send) {
        UBus *ubus = Nan::ObjectWrap::Unwrap<UBus>(info.Holder());

        if (info.Length() != 2 || !info[0]->IsString() || !info[1]->IsString())
		{
			return Nan::ThrowSyntaxError("Usage: Send(type, json_string)");
		}

		String::Utf8Value type(info[0]->ToString());
		String::Utf8Value json_string(info[1]->ToString());

		if (!ubus->Send(*type, *json_string))
    		return Nan::ThrowSyntaxError("Connect to UBUS failed");
    }

    bool
    UBus::Connect(const char * type) {
    	if (ctx_ != NULL || poll_ != NULL)
    		return false;

    	ctx_ = ubus_connect(NULL);
		if (ctx_ == NULL)
			return false;

		memset(&listener_, 0, sizeof(listener_));
		listener_.event_handler.cb = receive_ubus_event;
		listener_.data = this;
		printf("type = %s\n", type);
		if (ubus_register_event_handler(ctx_, &listener_.event_handler, type))
			return false;

		poll_ = new uv_poll_t;
        memset(poll_,0,sizeof(uv_poll_t));
        poll_->data = this;
        uv_poll_init(uv_default_loop(), poll_, ctx_->sock.fd);

        if (!uv_is_active((uv_handle_t*)poll_)) {
            uv_poll_start(poll_, UV_READABLE, &UBus::Callback);
        }

		return true;
    }

    bool UBus::Send(const char * type, const char * json_string) {
    	blob_buf_init(&b, 0);
    	blobmsg_add_json_from_string(&b, json_string);
    	ubus_send_event(ctx_, type, b.head);
    	return true;
    }

	void
	UBus::Callback(uv_poll_t *w, int status, int revents) {
	    Nan::HandleScope scope;
	    UBus *watcher = static_cast<UBus*>(w->data);
	    assert(w == watcher->poll_);
		ubus_handle_event(watcher->ctx_);
	}
}

NAN_MODULE_INIT(Init)
{
	node_ubus::UBus::Initialize(target);
}

NODE_MODULE(ubus_bindings, Init)

