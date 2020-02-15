const pcsc = require('../pcsclite');

for (const exp in pcsc) {
	if (!pcsc[exp]) {
		throw new ReferenceError('Missing export: pcsc.' + exp)
	}
}
