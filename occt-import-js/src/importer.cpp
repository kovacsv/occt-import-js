#include "importer.hpp"

#include <STEPControl_Reader.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <BRepBndLib.hxx>
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
	OcctFace (const opencascade::handle<Poly_Triangulation>& triangulation, const TopLoc_Location& location) :
		triangulation (triangulation),
		location (location)
	{

	}

	virtual bool HasNormals () const override
	{
		return triangulation->HasNormals ();
	}

	virtual void EnumerateVertices (const std::function<void (double, double, double)>& onVertex) const override
	{
		gp_Trsf transformation = location.Transformation ();
		for (Standard_Integer nodeIndex = 1; nodeIndex <= triangulation->NbNodes (); nodeIndex++) {
			gp_Pnt vertex = triangulation->Node (nodeIndex);
			vertex.Transform (transformation);
			onVertex (vertex.X (), vertex.Y (), vertex.Z ());
		}
	}

	virtual void EnumerateNormals (const std::function<void (double, double, double)>& onNormal) const override
	{
		if (!triangulation->HasNormals ()) {
			return;
		}

		gp_Trsf transformation = location.Transformation ();
		for (Standard_Integer nodeIndex = 1; nodeIndex <= triangulation->NbNodes (); nodeIndex++) {
			gp_Dir normal = triangulation->Normal (nodeIndex);
			normal.Transform (transformation);
			onNormal (normal.X (), normal.Y (), normal.Z ());
		}
	}

	virtual void EnumerateTriangles (const std::function<void (int, int, int)>& onTriangle) const override
	{
		for (Standard_Integer triangleIndex = 1; triangleIndex <= triangulation->NbTriangles (); triangleIndex++) {
			Poly_Triangle triangle = triangulation->Triangle (triangleIndex);
			onTriangle (triangle (1) - 1, triangle (2) - 1, triangle (3) - 1);
		}
	}

	const opencascade::handle<Poly_Triangulation>&	triangulation;
	const TopLoc_Location&							location;
};

class OcctMesh : public Mesh
{
public:
	OcctMesh ()
	{
	
	}

	void ProcessFace (const TopoDS_Face& face, const std::function<void (const Face& face)>& onFace) const
	{
		TopLoc_Location location;
		Handle (Poly_Triangulation) triangulation = BRep_Tool::Triangulation (face, location);
		if (triangulation.IsNull () || triangulation->NbNodes () == 0 || triangulation->NbTriangles () == 0) {
			return;
		}
		triangulation->ComputeNormals ();
		OcctFace outputFace (triangulation, location);
		onFace (outputFace);
	}
};

class OcctFacesMesh : public OcctMesh
{
public:
	OcctFacesMesh (const TopoDS_Shape& shape) :
		shape (shape)
	{

	}

	virtual void EnumerateFaces (const std::function<void (const Face& face)>& onFace) const override
	{
		for (TopExp_Explorer ex (shape, TopAbs_FACE); ex.More (); ex.Next ()) {
			const TopoDS_Face& face = TopoDS::Face (ex.Current ());
			ProcessFace (face, onFace);
		}
	}

	const TopoDS_Shape& shape;
};

class OcctStandaloneFacesMesh : public OcctMesh
{
public:
	OcctStandaloneFacesMesh (const TopoDS_Shape& shape) :
		shape (shape)
	{

	}

	bool HasFaces () const
	{
		TopExp_Explorer ex (shape, TopAbs_FACE, TopAbs_SHELL);
		return ex.More ();
	}

	virtual void EnumerateFaces (const std::function<void (const Face& face)>& onFace) const override
	{
		for (TopExp_Explorer ex (shape, TopAbs_FACE, TopAbs_SHELL); ex.More (); ex.Next ()) {
			const TopoDS_Face& face = TopoDS::Face (ex.Current ());
			ProcessFace (face, onFace);
		}
	}

	const TopoDS_Shape& shape;
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
	
	output.OnBegin ();

	for (Standard_Integer rank = 1; rank <= stepReader.NbShapes (); rank++) {
		TopoDS_Shape shape = stepReader.Shape (rank);

		// Calculate triangulation
		Bnd_Box boundingBox;
		BRepBndLib::Add (shape, boundingBox, false);
		Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
		boundingBox.Get (xMin, yMin, zMin, xMax, yMax, zMax);
		Standard_Real avgSize = ((xMax - xMin) + (yMax - yMin) + (zMax - zMin)) / 3.0;
		Standard_Real linDeflection = avgSize / 1000.0;
		Standard_Real angDeflection = 0.5;
		BRepMesh_IncrementalMesh mesh (shape, linDeflection, Standard_False, angDeflection);

		for (TopExp_Explorer ex (shape, TopAbs_SHELL); ex.More (); ex.Next ()) {
			OcctFacesMesh outputShapeMesh (ex.Current ());
			output.OnMesh (outputShapeMesh);
		}

		OcctStandaloneFacesMesh standaloneFacesMesh (shape);
		if (standaloneFacesMesh.HasFaces ()) {
			output.OnMesh (standaloneFacesMesh);
		}
	}

	output.OnEnd ();

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
