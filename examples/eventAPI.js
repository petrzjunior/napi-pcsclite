const EventEmitter = require('events').EventEmitter
const pcscEmitter = require('bindings')('pcsclite').pcscEmitter
const inherits = require('util').inherits

inherits(pcscEmitter, EventEmitter);
const emitter = new pcscEmitter();

emitter.on('reader', () => {
    console.log('Reader connected');
});

emitter.on('present', (reader) => {
    console.log('Card present');
    let sendData = new ArrayBuffer(5);
    let sendRaw = new Uint8Array(sendData);
    sendRaw.set([0xFF, 0xB0, 0x00, 0x0D, 0x04]);
    console.log('Sending:', sendData);
    console.log('Received:', reader.send(sendData));
});

emitter.on('empty', () => {
    console.log('No card');
});

emitter.watch();