let pcsc = require('bindings')('pcsclite');

console.log(pcsc);
let context = pcsc.estabilish();
let readers = pcsc.getReaders(context);
let handle = pcsc.connect(context, readers);
let sendData = new ArrayBuffer(5);
let sendRaw = new Uint8Array(sendData);
sendRaw.set([0xFF, 0xB0, 0x00, 0x0D, 0x04]);
console.log(pcsc.transmit(handle, sendData));
pcsc.disconnect(handle);
pcsc.release(context);