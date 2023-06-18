let fs = require ('fs');
const occtimportjs = require ('../dist/occt-import-js.js')();

class TimeLogger
{
    constructor ()
    {
        this.beginDate = new Date ();
    }

    LogTime (message)
    {
        let currentDate = new Date ();
        console.log (message + ': ' + (currentDate - this.beginDate).toString ());
        this.beginDate = currentDate;
    }
}

let args = process.argv.splice (2);
if (args.length !== 2) {
    console.log ('Usage: node stp_to_obj.js <stpFilePath> <objFilePath>');
    process.exit (1);
}

let stpFilePath = args[0];
let objFilePath = args[1];

let timeLogger = new TimeLogger ();
occtimportjs.then ((occt) => {
    timeLogger.LogTime ('Library load');

	let fileContent = fs.readFileSync (stpFilePath);
    timeLogger.LogTime ('File read');

	let stpContent = occt.ReadStepFile (fileContent, null);
    timeLogger.LogTime ('Step import');

    if (!stpContent.success) {
        process.exit (1);
    }

    let objWriter = fs.createWriteStream (objFilePath);
    let meshCount = 0;
    let vertexCount = 0;
    for (let mesh of stpContent.meshes) {
        if (!mesh.attributes || !mesh.attributes.position || !mesh.index) {
            continue;
        }
        objWriter.write ('g Mesh' + (meshCount + 1).toString ().padStart (4, "0") + '\n');
        let meshVertexCount = 0;
        let positions = mesh.attributes.position.array;
        for (let i = 0; i < positions.length; i += 3) {
            objWriter.write ('v ' + positions[i] + ' ' + positions[i + 1] + ' ' + positions[i + 2] + '\n');
            meshVertexCount += 1;
        }
        let hasNormals = false;
        if (mesh.attributes.normal) {
            let normals = mesh.attributes.normal.array;
            hasNormals = (positions.length === normals.length);
            if (hasNormals) {
                for (let i = 0; i < normals.length; i += 3) {
                    objWriter.write ('vn ' + normals[i] + ' ' + normals[i + 1] + ' ' + normals[i + 2] + '\n');
                }
            }
        }
        let indices = mesh.index.array;
        for (let i = 0; i < indices.length; i += 3) {
            objWriter.write ('f ');
            let index1 = vertexCount + indices[i] + 1;
            let index2 = vertexCount + indices[i + 1] + 1;
            let index3 = vertexCount + indices[i + 2] + 1;
            if (hasNormals) {
                objWriter.write (index1 + '//' + index1 + ' ');
                objWriter.write (index2 + '//' + index2 + ' ');
                objWriter.write (index3 + '//' + index3 + ' ');
            } else {
                objWriter.write (index1 + ' ');
                objWriter.write (index2 + ' ');
                objWriter.write (index3 + ' ');
            }
            objWriter.write ('\n');
        }
        meshCount += 1;
        vertexCount += meshVertexCount;
    }
    timeLogger.LogTime ('Obj export');

    objWriter.close ();
});
