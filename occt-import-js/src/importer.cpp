#include "importer.hpp"

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <BRepBndLib.hxx>
#include <BRepMesh_IncrementalMesh.hxx>

#include <STEPConstruct.hxx>
#include <STEPConstruct_Styles.hxx>
#include <StepShape_ShapeRepresentation.hxx>
#include <StepVisual_StyledItem.hxx>
#include <StepVisual_PresentationStyleByContext.hxx>

#include <TDF_ChildIterator.hxx>
#include <TDocStd_Document.hxx>
#include <TDataStd_Name.hxx>
#include <Quantity_Color.hxx>
#include <STEPControl_Reader.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <STEPCAFControl_Reader.hxx>

#include <iostream>
#include <fstream>

Color::Color () :
	hasValue (false),
	r (0),
	g (0),
	b (0)
{

}

Color::Color (double r, double g, double b) :
	hasValue (true),
	r (r),
	g (g),
	b (b)
{

}

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
	OcctFace (const Handle(Poly_Triangulation)& triangulation, const TopLoc_Location& location) :
		Face (),
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

	const Handle(Poly_Triangulation)&	triangulation;
	const TopLoc_Location&				location;
};

class OcctMesh : public Mesh
{
public:
	OcctMesh (const std::string& name, const Color& color) :
		Mesh (),
		name (name),
		color (color)
	{
	
	}

	virtual const std::string& GetName () const override
	{
		return name;
	}

	virtual const Color& GetColor () const override
	{
		return color;
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

private:
	std::string		name;
	Color			color;
};

class OcctFacesMesh : public OcctMesh
{
public:
	OcctFacesMesh (const TopoDS_Shape& shape, const std::string& name, const Color& color) :
		OcctMesh (name, color),
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
		OcctMesh (std::string (), Color ()),
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

static std::string GetLabelName (const TDF_Label& label)
{
	Handle(TDataStd_Name) nameAttribute = new TDataStd_Name ();
	if (!label.FindAttribute (nameAttribute->GetID (), nameAttribute)) {
		return "";
	}
	Standard_Integer utf8NameLength = nameAttribute->Get ().LengthOfCString ();
	char* nameBuf = new char[utf8NameLength + 1];
	nameAttribute->Get ().ToUTF8CString (nameBuf);
	std::string name (nameBuf, utf8NameLength);
	delete[] nameBuf;
	return name;
}

static std::string GetShapeName (const TopoDS_Shape& shape, const Handle(XCAFDoc_ShapeTool)& shapeTool)
{
	TDF_Label shapeLabel;
	if (!shapeTool->Search (shape, shapeLabel)) {
		return "";
	}
	return GetLabelName (shapeLabel);
}

static Color GetShapeColor (const TopoDS_Shape& shape, const Handle(XCAFDoc_ColorTool)& colorTool)
{
    Quantity_Color color;
    if (colorTool->GetColor (shape, XCAFDoc_ColorSurf, color)) {
        return Color (color.Red (), color.Green (), color.Blue ());
    }
    if (colorTool->GetColor (shape, XCAFDoc_ColorCurv, color)) {
        return Color (color.Red (), color.Green (), color.Blue ());
    }
    if (colorTool->GetColor (shape, XCAFDoc_ColorGen, color)) {
        return Color (color.Red (), color.Green (), color.Blue ());
    }
	// TODO: handle face colors
    return Color ();
}

static void ProcessShape (const TopoDS_Shape& shape, const Handle(XCAFDoc_ShapeTool)& shapeTool, const Handle(XCAFDoc_ColorTool)& colorTool, Output& output)
{
	// Calculate triangulation
	Bnd_Box boundingBox;
	BRepBndLib::Add (shape, boundingBox, false);
	Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
	boundingBox.Get (xMin, yMin, zMin, xMax, yMax, zMax);
	Standard_Real avgSize = ((xMax - xMin) + (yMax - yMin) + (zMax - zMin)) / 3.0;
	Standard_Real linDeflection = avgSize / 1000.0;
	Standard_Real angDeflection = 0.5;
	BRepMesh_IncrementalMesh mesh (shape, linDeflection, Standard_False, angDeflection);

	// Enumerate solids
	for (TopExp_Explorer ex (shape, TopAbs_SOLID); ex.More (); ex.Next ()) {
		const TopoDS_Shape& currentShape = ex.Current ();
		std::string meshName = GetShapeName (currentShape, shapeTool);
		Color meshColor = GetShapeColor (currentShape, colorTool);
		OcctFacesMesh outputShapeMesh (currentShape, meshName, meshColor);
		output.OnMesh (outputShapeMesh);
	}

	// Enumerate shells that are not part of a solid
	for (TopExp_Explorer ex (shape, TopAbs_SHELL, TopAbs_SOLID); ex.More (); ex.Next ()) {
		const TopoDS_Shape& currentShape = ex.Current ();
		std::string meshName = GetShapeName (currentShape, shapeTool);
		Color meshColor = GetShapeColor (currentShape, colorTool);
		OcctFacesMesh outputShapeMesh (currentShape, meshName, meshColor);
		output.OnMesh (outputShapeMesh);
	}

	// Create a mesh from faces that are not part of a shell
	OcctStandaloneFacesMesh standaloneFacesMesh (shape);
	if (standaloneFacesMesh.HasFaces ()) {
		output.OnMesh (standaloneFacesMesh);
	}
}

static Result ReadStepFile (std::istream& inputStream, Output& output)
{
	STEPCAFControl_Reader stepCafReader;
	stepCafReader.SetColorMode (true);
	stepCafReader.SetNameMode (true);

	STEPControl_Reader& stepReader = stepCafReader.ChangeReader ();
	std::string dummyFileName = "stp";
 	IFSelect_ReturnStatus readStatus = stepReader.ReadStream (dummyFileName.c_str (), inputStream);
	if (readStatus != IFSelect_RetDone) {
		return Result::ImportFailed;
	}

	Handle(TDocStd_Document) document = new TDocStd_Document ("XmlXCAF");
	if (!stepCafReader.Transfer (document)) {
		return Result::ImportFailed;
	}

	TDF_Label mainLabel = document->Main ();
	Handle(XCAFDoc_ShapeTool) shapeTool = XCAFDoc_DocumentTool::ShapeTool (mainLabel);
	Handle(XCAFDoc_ColorTool) colorTool = XCAFDoc_DocumentTool::ColorTool (mainLabel);

    TDF_LabelSequence labels;
    shapeTool->GetFreeShapes (labels);
	if (labels.IsEmpty ()) {
		return Result::ImportFailed;
	}

	output.OnBegin ();
	for (Standard_Integer labelIndex = 1; labelIndex <= labels.Length (); labelIndex++) {
		TopoDS_Shape shape = shapeTool->GetShape (labels.Value (labelIndex));
		ProcessShape (shape, shapeTool, colorTool, output);
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
