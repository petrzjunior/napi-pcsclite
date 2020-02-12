/* Example using asynchronous callback-based PCSC API.
 * In case anything goes wrong (eg. card is quickly removed) an exception
 *   is raised.
 */

const pcsc = require('../pcsclite');

try {
	// Establish pcsc context
	const context = pcsc.establish();
	pcsc.globalChangeSubscribe(context, (state) => {
		try {
			console.log('Global state changed');
			// Context in local scope need to be created
			const context = pcsc.establish();
			const readers = pcsc.getReaders(context);
			console.log(readers);
			if (readers.length > 0) {
				const reader = readers[0];
				console.log('First reader: ' + reader);
				const status = pcsc.getStatus(context, reader);

				pcsc.readerChangeSubscribe(context, reader, status, (state) => {
					console.log('Reader' + reader + 'state changed');
				});
			}
		} catch (error) {
			console.error(error);
		}
	});
} catch (error) {
	console.error(error);
} finally {
	// Release pcsc context
	pcsc.release(context);
}
