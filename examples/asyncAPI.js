/* Example using asynchronous callback-based PCSC API.
 * In case anything goes wrong (eg. card is quickly removed) an exception
 *   is raised.
 */

const pcsc = require('../pcsclite');

// Establish pcsc context
const context = pcsc.establish();

function globalChangeHandler() {
	console.log('Global state changed');
	console.log('Connected readers:', pcsc.getReaders(context));
}

function getGlobalChange() {
	pcsc.getGlobalStatusChange(context).then(
		(changed) => {
			if (changed) {
				globalChangeHandler();
			}
		},
		(error) => console.error(error)
	).then(
		// Run in infinite loop
		() => getGlobalChange()
	);
}

getGlobalChange();
