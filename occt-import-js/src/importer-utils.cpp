#include "importer-utils.hpp"

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepMesh_IncrementalMesh.hxx>

VectorBuffer::VectorBuffer (const std::vector<uint8_t>& v)
{
    setg ((char*) v.data (), (char*) v.data (), (char*) (v.data () + v.size ()));
}

bool TriangulateShape (TopoDS_Shape& shape)
{
    Bnd_Box boundingBox;
    BRepBndLib::Add (shape, boundingBox, false);
    if (boundingBox.IsVoid ()) {
        return false;
    }

    Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
    boundingBox.Get (xMin, yMin, zMin, xMax, yMax, zMax);
    Standard_Real avgSize = ((xMax - xMin) + (yMax - yMin) + (zMax - zMin)) / 3.0;
    Standard_Real linDeflection = avgSize / 1000.0;
    Standard_Real angDeflection = 0.5;
    BRepMesh_IncrementalMesh mesh (shape, linDeflection, Standard_False, angDeflection);
    return true;
}
