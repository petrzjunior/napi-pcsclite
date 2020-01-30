const EventEmitter = require('events').EventEmitter
const pcsc = require('bindings')('pcsclite');
const inherits = require('util').inherits

inherits(pcsc.pcscEmitter, EventEmitter);

module.exports = {
    // Objects
    pcscEmitter: pcsc.pcscEmitter,

    // Functions
    estabilish: pcsc.estabilish,
    release: pcsc.release,
    getReaders: pcsc.getReaders,
    connect: pcsc.connect,
    disconnect: pcsc.disconnect,
    transmit: pcsc.transmit,
    getStatus: pcsc.getStatus,
    waitUntilGlobalChange: pcsc.waitUntilGlobalChange,
    waitUntilReaderChange: pcsc.waitUntilReaderChange,
    waitUntilReaderConnected: pcsc.waitUntilReaderConnected,
    waitUntilReaderState: pcsc.waitUntilReaderState,

    // Constants
    statePresent: pcsc.statePresent,
    stateEmpty: pcsc.stateEmpty
}
