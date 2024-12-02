var fs = require ('fs');
var path = require ('path');
var assert = require ('assert');

var occtimportjs = require ('../build/wasm/Release/occt-import-js.js')();

var occt = null;
before (async function () {
    if (occt !== null) {
        return;
    }
    occt = await occtimportjs;
});

function LoadFile (format, fileUrl)
{
    let fileContent = fs.readFileSync (fileUrl);
    return occt.ReadFile (format, fileContent, null);
}

function LoadStepFile (fileUrl)
{
    let fileContent = fs.readFileSync (fileUrl);
    return occt.ReadStepFile (fileContent, null);
}

function LoadIgesFile (fileUrl)
{
    let fileContent = fs.readFileSync (fileUrl);
    return occt.ReadIgesFile (fileContent, null);
}

function LoadBrepFile (fileUrl)
{
    let fileContent = fs.readFileSync (fileUrl);
    return occt.ReadBrepFile (fileContent, null);
}

function LoadStepFileWithParams (fileUrl, params)
{
    let fileContent = fs.readFileSync (fileUrl);
    return occt.ReadStepFile (fileContent, params);
}

describe ('Step Import', function () {
    
it ('simple-basic-cube', function () {
    let result = LoadStepFile ('./test/testfiles/simple-basic-cube/cube.stp');
    assert (result.success);
    assert.strictEqual (result.meshes.length, 1);
    assert.deepStrictEqual (result.root, {
        name : "",
        meshes : [],
        children : [
            {
                name : "cube",
                meshes : [0],
                children : []            
            }
        ]
    });
});

it ('as1_pe_203', function () {
    let result = LoadStepFile ('./test/testfiles/cax-if/as1_pe_203.stp');

    assert.strictEqual (result.meshes.length, 18);
    for (let mesh of result.meshes) {
        assert (mesh.name !== undefined);
        assert (mesh.color !== undefined);
    }
    
    assert.equal (1980, result.meshes[0].index.array.length);
    assert.equal (1356, result.meshes[1].index.array.length);
    assert.equal (612, result.meshes[2].index.array.length);
    assert.equal (360, result.meshes[3].index.array.length);
    assert.equal (612, result.meshes[4].index.array.length);
    assert.equal (360, result.meshes[5].index.array.length);
    assert.equal (612, result.meshes[6].index.array.length);
    assert.equal (360, result.meshes[7].index.array.length);
    assert.equal (1356, result.meshes[8].index.array.length);
    assert.equal (612, result.meshes[9].index.array.length);
    assert.equal (360, result.meshes[10].index.array.length);
    assert.equal (612, result.meshes[11].index.array.length);
    assert.equal (360, result.meshes[12].index.array.length);
    assert.equal (612, result.meshes[13].index.array.length);
    assert.equal (360, result.meshes[14].index.array.length);
    assert.equal (300, result.meshes[15].index.array.length);
    assert.equal (360, result.meshes[16].index.array.length);
    assert.equal (360, result.meshes[17].index.array.length);
    
    assert.deepStrictEqual (result.root, {
        name : "",
        meshes : [],
        children : [
            {
                name : "AS1_PE_ASM",
                meshes : [],
                children : [
                    {
                        name : "PLATE",
                        meshes : [0],
                        children : []
                    },
                    {
                        name : "L_BRACKET_ASSEMBLY_ASM",
                        meshes : [1, 2, 3, 4, 5, 6, 7],
                        children : []
                    },
                    {
                        name : "L_BRACKET_ASSEMBLY_ASM",
                        meshes : [8, 9, 10, 11, 12, 13, 14],
                        children : []
                    },
                    {
                        name : "ROD_ASM",
                        meshes : [15, 16, 17],
                        children : []
                    }
                ]            
            }
        ]
    });
});

it ('as1-oc-214', function () {
    let result = LoadStepFile ('./test/testfiles/cax-if/as1-oc-214.stp');
    assert (result.success);
    assert.strictEqual (result.meshes.length, 18);
});

it ('as1-tu-203', function () {
    let result = LoadStepFile ('./test/testfiles/cax-if/as1-tu-203.stp');
    assert (result.success);
    assert.strictEqual (result.meshes.length, 18);
});

it ('io1-cm-214', function () {
    let result = LoadStepFile ('./test/testfiles/cax-if/io1-cm-214.stp');
    assert (result.success);
    assert.strictEqual (result.meshes.length, 1);
});

it ('io1-tu-203', function () {
    let result = LoadStepFile ('./test/testfiles/cax-if/io1-tu-203.stp');
    assert (result.success);
    assert.strictEqual (result.meshes.length, 1);
});

it ('dm1-id-214', function () {
    let result = LoadStepFile ('./test/testfiles/cax-if/dm1-id-214.stp');
    assert (result.success);
    assert.strictEqual (result.meshes.length, 7);
    assert.deepStrictEqual (result.meshes[0].color, [ 1.0, 0.0, 0.0 ]);
    assert.deepStrictEqual (result.meshes[1].color, [ 1.0, 0.39312317967414856, 0.0 ]);
    assert.deepStrictEqual (result.meshes[2].color, [ 1.0, 0.39312317967414856, 0.0 ]);
    assert.deepStrictEqual (result.meshes[3].color, [ 1.0, 0.39312317967414856, 0.0 ]);
    assert.deepStrictEqual (result.meshes[4].color, [ 1.0, 0.08898153156042099, 0.0 ]);
    assert.deepStrictEqual (result.meshes[5].color, [ 1.0, 0.08898153156042099, 0.0 ]);
    assert.deepStrictEqual (result.meshes[6].color, [ 1.0, 0.08898153156042099, 0.0 ]);
});

it ('sg1-c5-214', function () {
    let result = LoadStepFile ('./test/testfiles/cax-if/sg1-c5-214.stp');
    assert (result.success);
    assert.strictEqual (result.meshes.length, 1);
});

it ('as1-oc-214', function () {
    let result = LoadStepFile ('./test/testfiles/cax-if/as1-oc-214/as1-oc-214.stp');
    assert (result.success);
    assert.strictEqual (result.meshes.length, 18);
});

it ('Cube 10x10', function () {
    let result = LoadStepFile ('./test/testfiles/cube-10x10mm/Cube 10x10.stp');
    assert (result.success);
    assert.strictEqual (result.meshes.length, 1);
    assert.strictEqual (36, result.meshes[0].index.array.length);
    assert.deepStrictEqual (result.root, {
        name : "",
        meshes : [],
        children : [
            {
                name : "Cube 10x10",
                meshes : [0],
                children : []       
            }
        ]
    });
});

it ('cube-fcstd', function () {
    let result = LoadStepFile ('./test/testfiles/cube-fcstd/cube.step');
    assert (result.success);
    assert.deepStrictEqual (result.root, {
        name : "",
        meshes : [],
        children : [
            {
                name : "Cube",
                meshes : [ 0 ],
                children : []
            }
        ]
    });
    assert.strictEqual (result.meshes.length, 1);
    assert.deepStrictEqual (result.meshes[0].name, 'Cube');
    assert.deepStrictEqual (result.meshes[0].index.array.length, 36);
    assert.deepStrictEqual (result.meshes[0].color, [ 0.6038273572921753, 0.6038273572921753, 0.6038273572921753 ]);
    assert.deepStrictEqual (result.meshes[0].brep_faces, [
        { first: 0, last: 1, color: [ 1, 0, 0 ] },
        { first: 2, last: 3, color: null },
        { first: 4, last: 5, color: null },
        { first: 6, last: 7, color: [ 0, 0, 1 ] },
        { first: 8, last: 9, color: null },
        { first: 10, last: 11, color: [ 0, 0.4019778072834015, 0 ] }
    ]);
});

it ('conical-surface', function () {
    let result = LoadStepFile ('./test/testfiles/conical-surface/conical-surface.step');
    assert (result.success);
    assert.deepStrictEqual (result.root, {
        name : "",
        meshes : [],
        children : [
            {
                name : "(Unsaved)",
                meshes : [ 0 ],
                children : []
            }
        ]
    });
    assert.strictEqual (result.meshes.length, 1);
    assert.equal (3954, result.meshes[0].index.array.length);
});

});

describe ('Iges Import', function () {

it ('Cube 10x10', function () {
    let result = LoadIgesFile ('./test/testfiles/cube-10x10mm/Cube 10x10.igs');
    assert (result.success);
    assert.strictEqual (result.meshes.length, 1);
    assert.strictEqual (36, result.meshes[0].index.array.length);
    assert.deepStrictEqual (result.root, {
        name : "",
        meshes : [],
        children : [
            {
                name : "Solid1",
                meshes : [0],
                children : []       
            }
        ]
    });
});

});

describe ('Brep Import', function () {

it ('as1_pe_203', function () {
    let result = LoadBrepFile ('./test/testfiles/cax-if-brep/as1_pe_203.brep');
    assert (result.success);
    assert.strictEqual (result.meshes.length, 18);
    
    assert.equal (1980, result.meshes[0].index.array.length);
    assert.equal (1356, result.meshes[1].index.array.length);
    assert.equal (612, result.meshes[2].index.array.length);
    assert.equal (360, result.meshes[3].index.array.length);
    assert.equal (612, result.meshes[4].index.array.length);
    assert.equal (360, result.meshes[5].index.array.length);
    assert.equal (612, result.meshes[6].index.array.length);
    assert.equal (360, result.meshes[7].index.array.length);
    assert.equal (1356, result.meshes[8].index.array.length);
    assert.equal (612, result.meshes[9].index.array.length);
    assert.equal (360, result.meshes[10].index.array.length);
    assert.equal (612, result.meshes[11].index.array.length);
    assert.equal (360, result.meshes[12].index.array.length);
    assert.equal (612, result.meshes[13].index.array.length);
    assert.equal (360, result.meshes[14].index.array.length);
    assert.equal (300, result.meshes[15].index.array.length);
    assert.equal (360, result.meshes[16].index.array.length);
    assert.equal (360, result.meshes[17].index.array.length);    
});

});

describe ('Deflection', function () {

function CheckVertexParamCount (params, vertexParamCount) {
    let result = LoadStepFileWithParams ('./test/testfiles/rounded-cube/rounded-cube.step', params);
    assert (result.success);
    assert.equal (vertexParamCount, result.meshes[0].attributes.position.array.length);
}        

it ('Auto deflection', function () {
    CheckVertexParamCount (null, 294);
    CheckVertexParamCount ({}, 294);
});

it ('Manual deflection with ratio of bounding box', function () {
    CheckVertexParamCount ({ linearDeflectionType : 'bounding_box_ratio', linearDeflection : 0.001 }, 294);
    CheckVertexParamCount ({ linearDeflectionType : 'bounding_box_ratio', linearDeflection : 0.1 }, 162);
});

it ('Manual deflection with absolute value', function () {
    CheckVertexParamCount ({ linearDeflectionType : 'absolute_value', linearDeflection : 10 }, 162);
    CheckVertexParamCount ({ linearDeflectionType : 'absolute_value', linearDeflection : 20 }, 126);
    
    CheckVertexParamCount ({ linearDeflectionType : 'absolute_value', linearDeflection : 10, angularDeflection : 0.5 }, 162);
    CheckVertexParamCount ({ linearDeflectionType : 'absolute_value', linearDeflection : 10, angularDeflection : 2.0 }, 114);
});

it ('Manual deflection with absolute value and units', function () {
    CheckVertexParamCount ({ linearUnit : 'millimeter', linearDeflectionType : 'absolute_value', linearDeflection : 10 }, 162);
	CheckVertexParamCount ({ linearUnit : 'meter', linearDeflectionType : 'absolute_value', linearDeflection : 0.01 }, 162);
});

});

describe ('Units', function () {

function CheckXSize (params, fileName, xSizeRef) {
    let result = LoadStepFileWithParams ('./test/testfiles/cube-units/' + fileName, params);
    assert (result.success);
	let xMin = Infinity;
	let xMax = -Infinity;
	for (let mesh of result.meshes) {
		for (let i = 0; i < mesh.attributes.position.array.length; i += 3) {
			xMin = Math.min (mesh.attributes.position.array[i], xMin);
			xMax = Math.max (mesh.attributes.position.array[i], xMax);
		}
	}
	let xSize = xMax - xMin;
	assert.ok (Math.abs (xSize - xSizeRef) < 1e-5);
}

it ('Default unit is mm', function () {
	CheckXSize ({}, 'cube-m.step', 1000.0);
	CheckXSize ({}, 'cube-mm.step', 1000.0);
	CheckXSize ({}, 'cube-in.step', 1000.0);
});

it ('Convert to m', function () {
	CheckXSize ({ linearUnit : 'meter' }, 'cube-m.step', 1.0);
	CheckXSize ({ linearUnit : 'meter' }, 'cube-mm.step', 1.0);
	CheckXSize ({ linearUnit : 'meter' }, 'cube-in.step', 1.0);
});

it ('Convert to other units', function () {
	CheckXSize ({ linearUnit : 'millimeter' }, 'cube-m.step', 1000.0);
	CheckXSize ({ linearUnit : 'centimeter' }, 'cube-m.step', 100.0);
	CheckXSize ({ linearUnit : 'meter' }, 'cube-m.step', 1.0);
	CheckXSize ({ linearUnit : 'inch' }, 'cube-m.step', 39.37007);
	CheckXSize ({ linearUnit : 'foot' }, 'cube-m.step', 3.28084);
});

});

describe ('General Import', function () {

it ('Format string test', function () {
    let stepResult = LoadFile ('step', './test/testfiles/simple-basic-cube/cube.stp');
    assert (stepResult.success);
    let igesResult = LoadFile ('iges', './test/testfiles/cube-10x10mm/Cube 10x10.igs');
    assert (igesResult.success);
    let brepResult = LoadFile ('brep', './test/testfiles/cax-if-brep/as1_pe_203.brep');
    assert (brepResult.success);
    let otherResult = LoadFile ('other', './test/testfiles/simple-basic-cube/cube.stp');
    assert (!otherResult.success);
});

});
