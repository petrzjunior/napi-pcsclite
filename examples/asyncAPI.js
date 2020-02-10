/* Example using asynchronous callback-based PCSC API.
 * In case anything goes wrong (eg. card is quickly removed) an exception
 *   is raised.
 */

const pcsc = require('../pcsclite');

// Establish pcsc context
const context = pcsc.establish();

function sleep(time) {
	var stop = new Date().getTime();
	while (new Date().getTime() < stop + time) {
		;
	}
}

try {
	pcsc.globalChangeSubscribe(context, (readerName) => {
		console.log(readerName);
	});
} catch (error) {
	console.error(error);
} finally {
	// Release pcsc context
	//pcsc.release(context);
}
