/* Example using asynchronous promise-based PCSC API.
 * In case anything goes wrong (eg. card is quickly removed) an exception
 *   is raised.
 */

const pcsc = require('../pcsclite');

function readerChangeHandler(context, reader) {
	console.log('  ', reader, 'status changed');
	pcsc.getReaderStatusChange(context, reader).then(
		(_newState) => readerChangeHandler(context, reader),
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
			(_newState) => readerChangeHandler(readerContext, reader),
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
