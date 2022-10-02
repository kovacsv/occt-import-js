#include "importer-utils.hpp"

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepMesh_IncrementalMesh.hxx>

VectorBuffer::VectorBuffer (const std::vector<uint8_t>& v)
{
    setg ((char*) v.data (), (char*) v.data (), (char*) (v.data () + v.size ()));
}

OcctFace::OcctFace (const TopoDS_Face& face) :
    Face (),
    face (face),
    triangulation (),
    location ()
{
    triangulation = BRep_Tool::Triangulation (face, location);
    if (HasTriangulation ()) {
        triangulation->ComputeNormals ();
    }
}

bool OcctFace::HasNormals () const
{
    return HasTriangulation () && triangulation->HasNormals ();
}

bool OcctFace::GetColor (Color&) const
{
    return false;
}

void OcctFace::EnumerateVertices (const std::function<void (double, double, double)>& onVertex) const
{
    if (!HasTriangulation ()) {
        return;
    }

    gp_Trsf transformation = location.Transformation ();
    for (Standard_Integer nodeIndex = 1; nodeIndex <= triangulation->NbNodes (); nodeIndex++) {
        gp_Pnt vertex = triangulation->Node (nodeIndex);
        vertex.Transform (transformation);
        onVertex (vertex.X (), vertex.Y (), vertex.Z ());
    }
}

void OcctFace::EnumerateNormals (const std::function<void (double, double, double)>& onNormal) const
{
    if (!HasTriangulation () || !triangulation->HasNormals ()) {
        return;
    }

    bool isReversed = (face.Orientation () == TopAbs_REVERSED);
    gp_Trsf transformation = location.Transformation ();
    for (Standard_Integer nodeIndex = 1; nodeIndex <= triangulation->NbNodes (); nodeIndex++) {
        gp_Dir normal = triangulation->Normal (nodeIndex);
        normal.Transform (transformation);
        if (isReversed) {
            onNormal (-normal.X (), -normal.Y (), -normal.Z ());
        } else {
            onNormal (normal.X (), normal.Y (), normal.Z ());
        }
    }
}

void OcctFace::EnumerateTriangles (const std::function<void (int, int, int)>& onTriangle) const
{
    if (!HasTriangulation ()) {
        return;
    }

    bool isReversed = (face.Orientation () == TopAbs_REVERSED);
    for (Standard_Integer triangleIndex = 1; triangleIndex <= triangulation->NbTriangles (); triangleIndex++) {
        Poly_Triangle triangle = triangulation->Triangle (triangleIndex);
        if (isReversed) {
            onTriangle (triangle (1) - 1, triangle (3) - 1, triangle (2) - 1);
        } else {
            onTriangle (triangle (1) - 1, triangle (2) - 1, triangle (3) - 1);
        }
    }
}

bool OcctFace::HasTriangulation () const
{
    if (triangulation.IsNull () || triangulation->NbNodes () == 0 || triangulation->NbTriangles () == 0) {
        return false;
    }
    return true;
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
    if (linDeflection < Precision::Confusion ()) {
        linDeflection = 1.0;
    }
    Standard_Real angDeflection = 0.5;
    BRepMesh_IncrementalMesh mesh (shape, linDeflection, Standard_False, angDeflection);
    return true;
}
