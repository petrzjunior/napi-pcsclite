const pcsc = require('./build/Release/pcsclite');

module.exports = {
	// Functions
	compareState: pcsc.compareState,
	establish: pcsc.establish,
	release: pcsc.release,
	getReaders: pcsc.getReaders,
	connect: pcsc.connect,
	disconnect: pcsc.disconnect,
	cancel: pcsc.cancel,
	transmit: pcsc.transmit,
	getStatus: pcsc.getStatus,
	directCommand: pcsc.directCommand,
	getGlobalStatusChange: pcsc.getGlobalStatusChange,
	getReaderStatusChange: pcsc.getReaderStatusChange,

	// Constants
	statePresent: pcsc.statePresent,
	stateEmpty: pcsc.stateEmpty
};
