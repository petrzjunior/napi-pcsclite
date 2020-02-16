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
		(newState) => {
			readerChangeHandler(context, reader, newState);
		},
		(error) => {
			console.error(error);
			pcsc.release(context);
		}
	);
}

function globalChangeHandler(context) {
	console.log('Global state changed');
	const readers = pcsc.getReaders(globalContext);
	console.log('Connected readers:', readers);
	for (const reader of readers) {
		const readerContext = pcsc.establish();
		pcsc.getReaderStatusChange(readerContext, reader).then(
			(newState) => readerChangeHandler(readerContext, reader, newState),
			(error) => console.error(error)
		);
	}
	pcsc.getGlobalStatusChange(globalContext).then(
		(_changed) => globalChangeHandler(context),
		error => {
			console.error(error);
			pcsc.release(context);
		}
	);
}

const globalContext = pcsc.establish();
globalChangeHandler(globalContext);
