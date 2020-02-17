/* Example using asynchronous promise-based PCSC API.
 * In case anything goes wrong (eg. card is quickly removed) an exception
 *   is raised.
 */

const pcsc = require('../pcsclite');

function readerChangeHandler(context, reader, state) {
	console.log(' ', reader, 'status changed');
	if (pcsc.compareState(state, pcsc.statePresent)) {
		console.log('    Card is present');
	}
	if (pcsc.compareState(state, pcsc.stateEmpty)) {
		console.log('    Card is empty');
	}
	pcsc.getReaderStatusChange(context, reader).then(
		(newState) => readerChangeHandler(context, reader, newState),
		(error) => {
			console.error(error);
		}
	);
}

let knownReaders = new Map();

function globalChangeHandler(context) {
	console.log('Global state changed');
	const connectedReaders = pcsc.getReaders(context);
	console.log('Connected readers:', connectedReaders);
	for (const reader of [...knownReaders.keys(), ...connectedReaders]) {
		if (connectedReaders.includes(reader) && !knownReaders.has(reader)) {
			// New reader, create context, add callback
			console.log('Reader', reader, 'connected');
			const readerContext = pcsc.establish();
			knownReaders.set(reader, readerContext);
			pcsc.getReaderStatusChange(readerContext, reader).then(
				(newState) => readerChangeHandler(readerContext, reader, newState),
				(error) => console.error(error)
			);
		}
		if (knownReaders.has(reader) && !connectedReaders.includes(reader)) {
			// Disconnected reader, cancel callback, release context
			console.log('Reader', reader, 'disconnected');
			const readerContext = knownReaders.get(reader);
			pcsc.cancel(readerContext);
			pcsc.release(readerContext);
			knownReaders.delete(reader);
			console.log('OK');
		}
	}
	pcsc.getGlobalStatusChange(context).then(
		(_changed) => globalChangeHandler(context),
		error => {
			console.error(error);
			pcsc.release(context);
		}
	);
}

const globalContext = pcsc.establish();
globalChangeHandler(globalContext);
