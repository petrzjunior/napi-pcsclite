const EventEmitter = require('events').EventEmitter
const pcscEmitter = require('bindings')('pcsclite').pcscEmitter
const inherits = require('util').inherits

inherits(pcscEmitter, EventEmitter);
const emitter = new pcscEmitter();
emitter.on('start', () => {
    console.log('DEBUG: Start');
});

emitter.on('reader', () => {
    console.log('Reader connected');
});

emitter.on('present', (evt) => {
    console.log('Card present');
});

emitter.on('empty', () => {
    console.log('No card');
});

emitter.watch();