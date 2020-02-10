//const EventEmitter = require('events').EventEmitter;
const pcsc = require('./build/Release/pcsclite');
//const inherits = require('util').inherits;

//inherits(pcsc.pcscEmitter, EventEmitter);

module.exports = {
	// Objects
	//pcscEmitter: pcsc.pcscEmitter,

	// Functions
	establish: pcsc.establish,
	release: pcsc.release,
	getReaders: pcsc.getReaders,
	connect: pcsc.connect,
	disconnect: pcsc.disconnect,
	transmit: pcsc.transmit,
	getStatus: pcsc.getStatus,
	globalChangeSubscribe: pcsc.globalChangeSubscribe,
	waitUntilGlobalChange: pcsc.waitUntilGlobalChange,
	waitUntilReaderChange: pcsc.waitUntilReaderChange,
	waitUntilReaderConnected: pcsc.waitUntilReaderConnected,
	waitUntilReaderState: pcsc.waitUntilReaderState,

	// Constants
	statePresent: pcsc.statePresent,
	stateEmpty: pcsc.stateEmpty
};
