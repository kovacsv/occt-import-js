#define IGES_SUPPORT

#include "importer.hpp"

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
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

#ifdef IGES_SUPPORT
#include <IGESCAFControl_Reader.hxx>
#endif

#include <iostream>
#include <fstream>

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

static bool IsFreeShape (const TDF_Label& label, const Handle (XCAFDoc_ShapeTool)& shapeTool)
{
	TopoDS_Shape tmpShape;
	return shapeTool->GetShape (label, tmpShape) && shapeTool->IsFree (label);
}

static bool TriangulateShape (TopoDS_Shape& shape)
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
	return Color ();
}

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
	OcctFace (const TopoDS_Face& face, const Handle(XCAFDoc_ColorTool)& colorTool) :
		Face (),
		face (face),
		colorTool (colorTool)
	{
		triangulation = BRep_Tool::Triangulation (face, location);
		if (HasTriangulation ()) {
			triangulation->ComputeNormals ();
		}
	}

	virtual bool HasNormals () const override
	{
		return HasTriangulation () && triangulation->HasNormals ();
	}

	virtual Color GetColor () const override
	{
		return GetShapeColor ((const TopoDS_Shape&) face, colorTool);
	}

	virtual void EnumerateVertices (const std::function<void (double, double, double)>& onVertex) const override
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

	virtual void EnumerateNormals (const std::function<void (double, double, double)>& onNormal) const override
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

	virtual void EnumerateTriangles (const std::function<void (int, int, int)>& onTriangle) const override
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

private:
	bool HasTriangulation () const
	{
		if (triangulation.IsNull () || triangulation->NbNodes () == 0 || triangulation->NbTriangles () == 0) {
			return false;
		}
		return true;
	}

	const TopoDS_Face&					face;
	const Handle(XCAFDoc_ColorTool)&	colorTool;
	Handle(Poly_Triangulation)			triangulation;
	TopLoc_Location						location;
};

class OcctFacesMesh : public Mesh
{
public:
	OcctFacesMesh (const TopoDS_Shape& shape, const Handle(XCAFDoc_ShapeTool)& shapeTool, const Handle(XCAFDoc_ColorTool)& colorTool) :
		Mesh (),
		shape (shape),
		shapeTool (shapeTool),
		colorTool (colorTool)
	{

	}

	virtual std::string GetName () const override
	{
		return GetShapeName (shape, shapeTool);
	}

	virtual Color GetColor () const override
	{
		return GetShapeColor (shape, colorTool);
	}

	virtual void EnumerateFaces (const std::function<void (const Face& face)>& onFace) const override
	{
		for (TopExp_Explorer ex (shape, TopAbs_FACE); ex.More (); ex.Next ()) {
			const TopoDS_Face& face = TopoDS::Face (ex.Current ());
			OcctFace outputFace (face, colorTool);
			onFace (outputFace);
		}
	}

private:
	const TopoDS_Shape& shape;
	const Handle(XCAFDoc_ShapeTool)& shapeTool;
	const Handle(XCAFDoc_ColorTool)& colorTool;
};

class OcctStandaloneFacesMesh : public Mesh
{
public:
	OcctStandaloneFacesMesh (const TopoDS_Shape& shape, const Handle(XCAFDoc_ColorTool)& colorTool) :
		Mesh (),
		shape (shape),
		colorTool (colorTool)
	{

	}

	bool HasFaces () const
	{
		TopExp_Explorer ex (shape, TopAbs_FACE, TopAbs_SHELL);
		return ex.More ();
	}

	virtual std::string GetName () const override
	{
		return std::string ();
	}

	virtual Color GetColor () const override
	{
		return Color ();
	}

	virtual void EnumerateFaces (const std::function<void (const Face& face)>& onFace) const override
	{
		for (TopExp_Explorer ex (shape, TopAbs_FACE, TopAbs_SHELL); ex.More (); ex.Next ()) {
			const TopoDS_Face& face = TopoDS::Face (ex.Current ());
			OcctFace outputFace (face, colorTool);
			onFace (outputFace);
		}
	}

private:
	const TopoDS_Shape& shape;
	const Handle(XCAFDoc_ColorTool)& colorTool;
};

class DocNode : public Node
{
public:
	DocNode (const TDF_Label& label, const Handle (XCAFDoc_ShapeTool)& shapeTool, const Handle (XCAFDoc_ColorTool)& colorTool) :
		label (label),
		shapeTool (shapeTool),
		colorTool (colorTool)
	{

	}

	virtual std::string GetName () const override
	{
		return GetLabelName (label);
	}

	virtual std::vector<NodePtr> GetChildren () const override
	{
		if (IsMeshNode ()) {
			return {};
		}

		std::vector<NodePtr> children;
		for (TDF_ChildIterator it (label); it.More (); it.Next ()) {
			TDF_Label childLabel = it.Value ();
			if (IsFreeShape (childLabel, shapeTool)) {
				children.push_back (std::make_shared<const DocNode> (
					childLabel, shapeTool, colorTool
				));
			}
		}
		return children;
	}

	virtual bool IsMeshNode () const override
	{
		// if there are no children, it is a mesh node
		if (!label.HasChild ()) {
			return true;
		}

		// if it has a subshape child, treat it as mesh node
		bool hasSubShapeChild = false;
		for (TDF_ChildIterator it (label); it.More (); it.Next ()) {
			TDF_Label childLabel = it.Value ();
			if (shapeTool->IsSubShape (childLabel)) {
				hasSubShapeChild = true;
				break;
			}
		}
		if (hasSubShapeChild) {
			return true;
		}

		// if it doesn't have a freeshape child, treat it as a mesh node
		bool hasFreeShapeChild = false;
		for (TDF_ChildIterator it (label); it.More (); it.Next ()) {
			TDF_Label childLabel = it.Value ();
			if (IsFreeShape (childLabel, shapeTool)) {
				hasFreeShapeChild = true;
				break;
			}
		}
		if (!hasFreeShapeChild) {
			return true;
		}

		return false;
	}

	virtual void EnumerateMeshes (const std::function<void (const Mesh&)>& onMesh) const override
	{
		if (!IsMeshNode ()) {
			return;
		}

		TopoDS_Shape shape = shapeTool->GetShape (label);
		EnumerateShapeMeshes (shape, onMesh);
	}

private:
	void EnumerateShapeMeshes (const TopoDS_Shape& shape, const std::function<void (const Mesh&)>& onMesh) const
	{
		// Enumerate solids
		for (TopExp_Explorer ex (shape, TopAbs_SOLID); ex.More (); ex.Next ()) {
			const TopoDS_Shape& currentShape = ex.Current ();
			OcctFacesMesh outputShapeMesh (currentShape, shapeTool, colorTool);
			onMesh (outputShapeMesh);
		}

		// Enumerate shells that are not part of a solid
		for (TopExp_Explorer ex (shape, TopAbs_SHELL, TopAbs_SOLID); ex.More (); ex.Next ()) {
			const TopoDS_Shape& currentShape = ex.Current ();
			OcctFacesMesh outputShapeMesh (currentShape, shapeTool, colorTool);
			onMesh (outputShapeMesh);
		}

		// Create a mesh from faces that are not part of a shell
		OcctStandaloneFacesMesh standaloneFacesMesh (shape, colorTool);
		if (standaloneFacesMesh.HasFaces ()) {
			onMesh (standaloneFacesMesh);
		}
	}

	TDF_Label label;
	const Handle (XCAFDoc_ShapeTool)& shapeTool;
	const Handle (XCAFDoc_ColorTool)& colorTool;
};

class RootNode : public Node
{
public:
	RootNode (const Handle (XCAFDoc_ShapeTool)& shapeTool, const Handle (XCAFDoc_ColorTool)& colorTool) :
		shapeTool (shapeTool),
		colorTool (colorTool)
	{

	}

	virtual std::string GetName () const override
	{
		return "";
	}

	virtual std::vector<NodePtr> GetChildren () const override
	{
		TDF_Label mainLabel = shapeTool->Label ();

		std::vector<NodePtr> children;
		for (TDF_ChildIterator it (mainLabel); it.More (); it.Next ()) {
			TDF_Label childLabel = it.Value ();
			if (IsFreeShape (childLabel, shapeTool)) {
				TopoDS_Shape shape = shapeTool->GetShape (childLabel);
				if (!TriangulateShape (shape)) {
					continue;
				}
				children.push_back (std::make_shared<const DocNode> (
					childLabel, shapeTool, colorTool
				));
			}
		}

		return children;
	}

	virtual bool IsMeshNode () const override
	{
		return false;
	}

	virtual void EnumerateMeshes (const std::function<void (const Mesh&)>& onMesh) const override
	{

	}

private:
	const Handle (XCAFDoc_ShapeTool)& shapeTool;
	const Handle (XCAFDoc_ColorTool)& colorTool;
};

class ImporterImpl
{
public:
	ImporterImpl () :
		document (nullptr),
		shapeTool (nullptr),
		colorTool (nullptr)
	{

	}

	Importer::Result LoadFile (Importer::Format format, const std::string& filePath)
	{
		std::ifstream inputStream (filePath, std::ios::binary);
		if (!inputStream.is_open ()) {
			return Importer::Result::FileNotFound;
		}
		Importer::Result result = LoadFile (format, inputStream);
		inputStream.close ();
		return result;
	}

	Importer::Result LoadFile (Importer::Format format, const std::vector<std::uint8_t>& fileContent)
	{
		VectorBuffer fileBuffer (fileContent);
		std::istream fileStream (&fileBuffer);
		return LoadFile (format, fileStream);
	}

	Importer::Result LoadFile (Importer::Format format, std::istream& inputStream)
	{
		if (format == Importer::Format::Step) {
			if (!TransferStepFileToDocument (inputStream)) {
				return Importer::Result::ImportFailed;
			}
		} else if (format == Importer::Format::Iges) {
			if (!TransferIgesFileToDocument (inputStream)) {
				return Importer::Result::ImportFailed;
			}
		} else {
			return Importer::Result::ImportFailed;
		}

		TDF_Label mainLabel = document->Main ();
		shapeTool = XCAFDoc_DocumentTool::ShapeTool (mainLabel);
		colorTool = XCAFDoc_DocumentTool::ColorTool (mainLabel);

		TDF_LabelSequence labels;
		shapeTool->GetFreeShapes (labels);
		if (labels.IsEmpty ()) {
			return Importer::Result::ImportFailed;
		}

		return Importer::Result::Success;
	}

	NodePtr GetRootNode () const
	{
		return std::make_shared<const RootNode> (shapeTool, colorTool);
	}

	void DumpHierarchy ()
	{
		std::function<void (const TDF_Label&, int)> dumpLabel = [&] (const TDF_Label& label, int depth) {
			for (int i = 0; i < depth; i++) {
				std::cout << "  ";
			}
			std::string name = GetLabelName (label);
			if (!name.empty ()) {
				std::cout << name << " ";
			}
			TopoDS_Shape tmpShape;
			std::cout << "( ";
			std::cout << (shapeTool->IsShape (label) ? "shape " : "");
			std::cout << (shapeTool->GetShape (label, tmpShape) ? "hasshape " : "");
			std::cout << (shapeTool->IsSimpleShape (label) ? "simple " : "");
			std::cout << (shapeTool->IsFree (label) ? "free " : "");
			std::cout << (shapeTool->IsCompound (label) ? "compound " : "");
			std::cout << (shapeTool->IsSubShape (label) ? "subshape " : "");
			std::cout << (shapeTool->IsReference (label) ? "reference " : "");
			std::cout << (shapeTool->IsAssembly (label) ? "assembly " : "");
			std::cout << ")" << std::endl;
			for (TDF_ChildIterator it (label); it.More (); it.Next ()) {
				TDF_Label child = it.Value ();
				TopoDS_Shape tmpChildShape;
				if (shapeTool->GetShape (child, tmpChildShape) && shapeTool->IsFree (child)) {
					dumpLabel (child, depth + 1);
				}
			}
		};

		dumpLabel (shapeTool->Label (), 0);
	}

private:
	bool TransferStepFileToDocument (std::istream& inputStream)
	{
		STEPCAFControl_Reader stepCafReader;
		stepCafReader.SetColorMode (true);
		stepCafReader.SetNameMode (true);

		STEPControl_Reader& stepReader = stepCafReader.ChangeReader ();
		std::string dummyFileName = "stp";
		IFSelect_ReturnStatus readStatus = stepReader.ReadStream (dummyFileName.c_str (), inputStream);
		if (readStatus != IFSelect_RetDone) {
			return false;
		}

		document = new TDocStd_Document ("XmlXCAF");
		if (!stepCafReader.Transfer (document)) {
			return false;
		}

		return true;
	}

	bool TransferIgesFileToDocument (std::istream& inputStream)
	{
#ifdef IGES_SUPPORT
		// IGESCAFControl_Reader::ReadStream is not implemented, so the stream
		// should be written to a temporary file to import the content from
		std::string dummyFileName = "temp.igs";
		std::ofstream dummyFile;
		dummyFile.open (dummyFileName);
		std::streampos fileLength = inputStream.tellg ();
		char* fileContent = new char[fileLength];
		inputStream.read (fileContent, fileLength);
		dummyFile.write (fileContent, fileLength);
		delete[] fileContent;
		dummyFile.close ();

		IGESCAFControl_Reader igesCafReader;
		igesCafReader.SetColorMode (true);
		igesCafReader.SetNameMode (true);

		IFSelect_ReturnStatus readStatus = igesCafReader.ReadFile (dummyFileName.c_str ());
		if (readStatus != IFSelect_RetDone) {
			std::remove (dummyFileName.c_str ());
			return false;
		}

		document = new TDocStd_Document ("XmlXCAF");
		if (!igesCafReader.Transfer (document)) {
			std::remove (dummyFileName.c_str ());
			return false;
		}

		std::remove (dummyFileName.c_str ());
		return true;
#else
		return false;
#endif
	}
	
	Handle(TDocStd_Document) document;
	Handle(XCAFDoc_ShapeTool) shapeTool;
	Handle(XCAFDoc_ColorTool) colorTool;
};

Importer::Importer () :
	impl (new ImporterImpl ())
{

}

Importer::~Importer ()
{
	delete impl;
}

Importer::Result Importer::LoadFile (Format format, const std::string& filePath)
{
	return impl->LoadFile (format, filePath);
}

Importer::Result Importer::LoadFile (Format format, std::istream& inputStream)
{
	return impl->LoadFile (format, inputStream);
}

Importer::Result Importer::LoadFile (Format format, const std::vector<std::uint8_t>& fileContent)
{
	return impl->LoadFile (format, fileContent);
}

NodePtr Importer::GetRootNode () const
{
	return impl->GetRootNode ();
}

void Importer::DumpHierarchy () const
{
	impl->DumpHierarchy ();
}

