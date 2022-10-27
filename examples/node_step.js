let fs = require ('fs');
let util = require ('util')
const occtimportjs = require ('../dist/occt-import-js.js')();

occtimportjs.then ((occt) => {
	let fileUrl = '../test/testfiles/simple-basic-cube/cube.stp';
	let fileContent = fs.readFileSync (fileUrl);
	let result = occt.ReadStepFile (fileContent, null);
	console.log (util.inspect (result, { showHidden: false, depth: null, colors: true }));
});
