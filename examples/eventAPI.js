/* Example using asynchronous event API
 * pcscEmitter once started emits events 'reader', 'present' and 'empty'
 * It is possible to transmit data only in the 'present' event via the
 *   pcscReader object passed as parameter.
 * In case anything goes wrong (eg. card is quickly removed) an 'error'
 *   event is emitted.
 */

const pcscEmitter = require('../pcsclite').pcscEmitter;

// Create event emitter
const emitter = new pcscEmitter();

emitter.on('reader', () => {
	console.log('Reader connected');
});

emitter.on('present', (reader) => {
	console.log('Card present');

	// Now we can send data to the card
	const sendData = new ArrayBuffer(5);
	const sendRaw = new Uint8Array(sendData);
	sendRaw.set([0xFF, 0xB0, 0x00, 0x0D, 0x04]);
	console.log('Sending:', sendData);
	try {
		let received = reader.send(sendData);
		console.log('Received:', received);
	}
	catch (exception) {
		console.error(exception);
	}
});

emitter.on('empty', () => {
	console.log('No card');
});

emitter.on('error', (exception) => {
	console.error(exception);
});

// Launch the event emitter
emitter.watch();