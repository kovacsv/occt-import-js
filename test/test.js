var fs = require ('fs');
var path = require ('path');
var assert = require ('assert');

var occtimportjs = require ('../build_wasm/Release/occt-import-js.js')();

var occt = null;
before (async function () {
	if (occt !== null) {
		return;
	}
	occt = await occtimportjs;
});

function LoadStepFile (fileUrl)
{
	let fileContent = fs.readFileSync (fileUrl);
	return occt.ReadStepFile (fileContent);
}

describe ('Step Import', function () {
	
it ('simple-basic-cube', function () {
	let result = LoadStepFile ('./test/testfiles/simple-basic-cube/cube.stp');
	assert (result.success);
	assert.strictEqual (result.meshes.length, 1);
});

it ('as1_pe_203.stp', function () {
	let result = LoadStepFile ('./test/testfiles/cax-if/as1_pe_203.stp');
	assert (result.success);
	assert.strictEqual (result.meshes.length, 18);
});

it ('as1-oc-214.stp', function () {
	let result = LoadStepFile ('./test/testfiles/cax-if/as1-oc-214.stp');
	assert (result.success);
	assert.strictEqual (result.meshes.length, 18);
});

it ('as1-tu-203.stp', function () {
	let result = LoadStepFile ('./test/testfiles/cax-if/as1-tu-203.stp');
	assert (result.success);
	assert.strictEqual (result.meshes.length, 18);
});

});
