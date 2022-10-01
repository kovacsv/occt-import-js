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

function LoadIgesFile (fileUrl)
{
    let fileContent = fs.readFileSync (fileUrl);
    return occt.ReadIgesFile (fileContent);
}

function LoadBrepFile (fileUrl)
{
    let fileContent = fs.readFileSync (fileUrl);
    return occt.ReadBrepFile (fileContent);
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

it ('as1_pe_203.stp', function () {
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
                        name : "L_BRACKET_ASSEMBLY",
                        meshes : [1, 2, 3, 4, 5, 6, 7],
                        children : []
                    },
                    {
                        name : "L_BRACKET_ASSEMBLY",
                        meshes : [8, 9, 10, 11, 12, 13, 14],
                        children : []
                    },
                    {
                        name : "ROD",
                        meshes : [15, 16, 17],
                        children : []
                    }
                ]            
            }
        ]
    });
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

it ('io1-cm-214.stp', function () {
    let result = LoadStepFile ('./test/testfiles/cax-if/io1-cm-214.stp');
    assert (result.success);
    assert.strictEqual (result.meshes.length, 1);
});

it ('io1-tu-203.stp', function () {
    let result = LoadStepFile ('./test/testfiles/cax-if/io1-tu-203.stp');
    assert (result.success);
    assert.strictEqual (result.meshes.length, 1);
});

it ('dm1-id-214.stp', function () {
    let result = LoadStepFile ('./test/testfiles/cax-if/dm1-id-214.stp');
    assert (result.success);
    assert.strictEqual (result.meshes.length, 7);
});

it ('sg1-c5-214.stp', function () {
    let result = LoadStepFile ('./test/testfiles/cax-if/sg1-c5-214.stp');
    assert (result.success);
    assert.strictEqual (result.meshes.length, 1);
});

it ('as1-oc-214.stp', function () {
    let result = LoadStepFile ('./test/testfiles/cax-if/as1-oc-214/as1-oc-214.stp');
    assert (result.success);
    assert.strictEqual (result.meshes.length, 18);
});

it ('Cube 10x10.stp', function () {
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

});

describe ('Iges Import', function () {

it ('Cube 10x10.igs', function () {
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

it ('as1_pe_203.brep', function () {
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
