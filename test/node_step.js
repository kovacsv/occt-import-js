let fs = require ('fs');
const occtimportjs = require ('../dist/occt-import-js.js')();

occtimportjs.then ((occt) => {
	let fileUrl = 'testfiles/simple-basic-cube/cube.stp';
	let fileContent = fs.readFileSync (fileUrl);
	let result = occt.ReadStepFile (fileContent);
	console.log (result);
});
