#include "importer.hpp"

#include <STEPControl_Reader.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <BRepMesh_IncrementalMesh.hxx>

#include <iostream>
#include <fstream>

class VectorBuffer : public std::streambuf
{
public:
    VectorBuffer (const std::vector<uint8_t>& v)
	{
        setg ((char*) v.data (), (char*) v.data (), (char*) (v.data () + v.size ()));
    }

    ~VectorBuffer ()
	{

	}
};

class OcctFace : public Face
{
public:
	OcctFace (const opencascade::handle<Poly_Triangulation>& triangulation) :
		triangulation (triangulation)
	{

	}

	virtual void EnumerateVertices (const std::function<void (double, double, double)>& onVertex) const override
	{
		// TODO: get transformation from location
		for (Standard_Integer nodeIndex = 1; nodeIndex <= triangulation->NbNodes (); nodeIndex++) {
			gp_Pnt vertex = triangulation->Node (nodeIndex);
			onVertex (vertex.X (), vertex.Y (), vertex.Z ());
		}
	}

	virtual void EnumerateTriangles (const std::function<void (int, int, int)>& onTriangle) const override
	{
		for (Standard_Integer triangleIndex = 1; triangleIndex <= triangulation->NbTriangles (); triangleIndex++) {
			Poly_Triangle triangle = triangulation->Triangle (triangleIndex);
			onTriangle (triangle (1), triangle (2), triangle (3));
		}
	}

	const opencascade::handle<Poly_Triangulation>&	triangulation;
};

class OcctShape : public Shape
{
public:
	OcctShape (TopoDS_Shape& shape) :
		shape (shape)
	{

	}

	virtual void EnumerateFaces (const std::function<void (const Face& face)>& onFace) const override
	{
		for (TopExp_Explorer explorer (shape, TopAbs_FACE); explorer.More (); explorer.Next ()) {
			const TopoDS_Face& face = TopoDS::Face (explorer.Current ());
			TopLoc_Location location;
			Handle (Poly_Triangulation) triangulation = BRep_Tool::Triangulation (face, location);
			if (triangulation.IsNull () || triangulation->NbNodes () == 0 || triangulation->NbTriangles () == 0) {
				continue;
			}
			OcctFace outputFace (triangulation);
			onFace (outputFace);
		}
	}

	TopoDS_Shape& shape;
};

static Result ReadStepFile (std::istream& inputStream, Output& output)
{
	STEPControl_Reader stepReader;
	std::string dummyFileName = "stp";
 	IFSelect_ReturnStatus readStatus = stepReader.ReadStream (dummyFileName.c_str (), inputStream);
	if (readStatus != IFSelect_RetDone) {
		return Result::ImportFailed;
	}

	stepReader.TransferRoots ();
	
	for (Standard_Integer rank = 1; rank <= stepReader.NbShapes (); rank++) {
		TopoDS_Shape shape = stepReader.Shape (rank);

		// TODO: calculate accuracy based on bounding box
		double accuracy = 1.0;

		// Calculate triangulation
		BRepMesh_IncrementalMesh mesh (shape, accuracy);

		OcctShape outputShape (shape);
		output.OnShape (outputShape);
	}

	return Result::Success;
}

Result ReadStepFile (const std::string& filePath, Output& output)
{
	std::ifstream inputStream (filePath, std::ios::binary);
	if (!inputStream.is_open ()) {
		return Result::FileNotFound;
	}
	Result result = ReadStepFile (inputStream, output);
	inputStream.close ();
	return result;
}

Result ReadStepFile (const std::vector<std::uint8_t>& fileContent, Output& output)
{
    VectorBuffer fileBuffer (fileContent);
	std::istream fileStream (&fileBuffer);
	return ReadStepFile (fileStream, output);
}
