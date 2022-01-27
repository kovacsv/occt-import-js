let fs = require ('fs');
const occtImportJS = require ('../build_wasm/Release/occt-import-js.js')();

console.log ('started');
occtImportJS.then ((occt) => {
	console.log ('loaded');
	let fileContent = fs.readFileSync ('testfiles/simple-basic-cube/cube.stp');
	let result = occt.ReadStepFile (fileContent);
	console.log (result);
	console.log (result.shapes);
});
