/* Example using asynchronous callback-based PCSC API.
 * In case anything goes wrong (eg. card is quickly removed) an exception
 *   is raised.
 */

const pcsc = require('../pcsclite');

// Establish pcsc context
const context = pcsc.establish();

function readerStateChanged(error, reader) {
	if (error) {
		console.error(error);
	} else {
		console.log('Reader' + reader + 'state changed');
	}
}

function globalStateChanged(error, state) {
	if (error) {
		console.error(error);
		return;
	}
	console.log('Global state changed');
	const context = pcsc.establish();
	try {
		const readers = pcsc.getReaders(context);
		if (readers.length > 0) {
			const reader = readers[0];
			console.log('  Assign callback to first reader: ' + reader);
			const status = pcsc.getStatus(context, reader);
			pcsc.readerChangeSubscribe(context, reader, status, readerStateChanged);
		}
	} catch (error) {
		console.error(error);
	}
}

try {
	pcsc.globalChangeSubscribe(context, globalStateChanged);
} catch (error) {
	console.error(error);
}
