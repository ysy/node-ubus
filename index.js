var addon = require('bindings')('ubus_bindings');
var util  = require('util');
var events= require('events');

function UBus() {
	events.EventEmitter.call(this);
	var self = this;
	self.ubus = new addon.UBus;
	self.ubus.callback = function(type, json_string) {
		self.emit('message', type, json_string);
	} ;
}

util.inherits(UBus, events.EventEmitter);
exports.UBus = UBus;

UBus.prototype.connect = function connect(type) {
	this.ubus.connect(type);
}

UBus.prototype.send = function send(type, json_string) {
	this.ubus.send(type, json_string);
}