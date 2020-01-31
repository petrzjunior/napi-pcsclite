# napi-pcsclite
![CI](https://github.com/petrzjunior/napi-pcsclite/workflows/CI/badge.svg?branch=master&event=push)

Simple yet powerful Node.js library for communicating with SmartCard.

Provides both blocking and **event-driven API** on **Windows, macOS and Linux** for SmartCard readers built upon the [PC/SC](https://en.wikipedia.org/wiki/PC/SC) standard. Feel the power of universal N-API no matter if your computer runs Windows [winscard](https://docs.microsoft.com/en-us/windows/win32/api/winscard/) or Unix [pscslite](https://pcsclite.apdu.fr/).

C++ binding is done through N-API which provides binaries compatible across multiple Node.js versio. Prebuilt binaries comming soon!

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
	const sendData = new ArrayBuffer(5);
	const sendRaw = new Uint8Array(sendData);
	sendRaw.set([0xFF, 0xB0, 0x00, 0x0D, 0x04]);
	console.log('Sending:', sendData);
	const received = reader.send(sendData);
	console.log('Received:', received);	
});
```
Launch the event emitter
```js
emitter.watch();
```
It's that simple!
Find more examples in the [examples](/examples) folder.

## Building from source
You will need a C++ compiler installed (gcc/clang on Linux, XCode on macOS, Visual Studio on Windows).
```console
$ npm install
```
This will install all dependencies and compile the PC/SC binding with node-gyp. Binary will be created in `build/Release` and you can run example:
```console
$ npm run exmaple
```