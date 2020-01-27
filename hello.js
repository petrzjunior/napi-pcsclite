var pscs = require('bindings')('pcscreader');

var reader = new pscs.PCSCReader();
console.log(reader.waitUntilReaderConnected());