# napi-pcsclite
Simple yet powerful NodeJS library for communicating with SmartCard.

<<<<<<< HEAD
Provides both blocking and **event-driven API** on **Windows, macOS and Linux** for SmartCard readers built upon the [PC/SC](https://en.wikipedia.org/wiki/PC/SC) standard.
=======
Provides both blocking and **event-driven API** on **Windows, macOS and Linux** for SmartCard readers built upon the [PC/SC](https://en.wikipedia.org/wiki/PC/SC)standard.
>>>>>>> 46a0be1... Use const

## Usage
Import and create the emitter:
```js
const pcscEmitter = require('pcsclite').pcscEmitter;
const emitter = new pcscEmitter();
```
Add event handlers:
```js
emitter.on('reader', () => {
	console.log('New reader connected');
});

emitter.on('present', (reader) => {
	console.log('Card present');

	// Send some data
<<<<<<< HEAD
	const sendData = new ArrayBuffer(5);
	const sendRaw = new Uint8Array(sendData);
	sendRaw.set([0xFF, 0xB0, 0x00, 0x0D, 0x04]);
	console.log('Sending:', sendData);
	const received = reader.send(sendData);
=======
	let sendData = new ArrayBuffer(5);
	let sendRaw = new Uint8Array(sendData);
	sendRaw.set([0xFF, 0xB0, 0x00, 0x0D, 0x04]);
	console.log('Sending:', sendData);
	let received = reader.send(sendData);
>>>>>>> 46a0be1... Use const
	console.log('Received:', received);	
});
```
Launch the event emitter
```js
emitter.watch();
```
It's that simple!
Find more examples in the [examples](/examples) folder.