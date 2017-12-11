var addon = require('bindings')('ubus_bindings');

ubus = new addon.UBus;

function ubus_on_message(type, json_string) {
	var obj = JSON.parse(json_string);
	ubus.send("test", JSON.stringify(obj));
}

ubus.callback = ubus_on_message;
ubus.connect("*");