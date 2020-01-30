/* Example using synchronous PCSC API calling low-level functions directly
 * It is advised to use the much simpler asynchronous event API instead
 *   as there is no way to convert blocking calls into events in NodeJS.
 * In case anything goes wrong (eg. card is quickly removed) an exception
 *   is raised.
 */

let pcsc = require('bindings')('pcsclite');

// Estabilish pcsc context
let context = pcsc.estabilish();
try {
	// Block until a reader is connected
	pcsc.waitUntilReaderConnected(context);

	// Get reader name(s)
	let readers = pcsc.getReaders(context);
	console.log('Readers: ' + readers);

	while (true) {
		// Block until a card is present
		pcsc.waitUntilReaderState(context, readers, pcsc.statePresent);

		// Connect to the card
		let handle = pcsc.connect(context, readers);

		// Send data to the card
		let sendData = new ArrayBuffer(5);
		let sendRaw = new Uint8Array(sendData);
		sendRaw.set([0xFF, 0xB0, 0x00, 0x0D, 0x04]);
		let received = pcsc.transmit(handle, sendData);
		console.log(received)

		// Disconnect from the card
		pcsc.disconnect(handle);

		// Block until card is away
		pcsc.waitUntilReaderState(context, readers, pcsc.stateEmpty);
	}
} catch (error) {
	console.error(error);
} finally {
	// Releace pcsc context
	pcsc.release(context);
}