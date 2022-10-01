#include "importer-xcaf.hpp"
#include "importer-utils.hpp"

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDocStd_Document.hxx>
#include <TDataStd_Name.hxx>
#include <Quantity_Color.hxx>
#include <BRep_Tool.hxx>
#include <XCAFDoc_DocumentTool.hxx>

static std::string GetLabelName (const TDF_Label& label)
{
    Handle (TDataStd_Name) nameAttribute = new TDataStd_Name ();
    if (!label.FindAttribute (nameAttribute->GetID (), nameAttribute)) {
        return std::string ();
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

static std::string GetShapeName (const TopoDS_Shape& shape, const Handle (XCAFDoc_ShapeTool)& shapeTool)
{
    TDF_Label shapeLabel;
    if (!shapeTool->Search (shape, shapeLabel)) {
        return std::string ();
    }
    return GetLabelName (shapeLabel);
}

static bool GetShapeColor (const TopoDS_Shape& shape, const Handle (XCAFDoc_ColorTool)& colorTool, Color& color)
{
    Quantity_Color qColor;
    if (colorTool->GetColor (shape, XCAFDoc_ColorSurf, qColor)) {
        color = Color (qColor.Red (), qColor.Green (), qColor.Blue ());
        return true;
    }
    if (colorTool->GetColor (shape, XCAFDoc_ColorCurv, qColor)) {
        color = Color (qColor.Red (), qColor.Green (), qColor.Blue ());
        return true;
    }
    if (colorTool->GetColor (shape, XCAFDoc_ColorGen, qColor)) {
        color = Color (qColor.Red (), qColor.Green (), qColor.Blue ());
        return true;
    }
    return false;
}

class XcafFace : public OcctFace
{
public:
    XcafFace (const TopoDS_Face& face, const Handle (XCAFDoc_ColorTool)& colorTool) :
        OcctFace (face),
        colorTool (colorTool)
    {

    }

    virtual bool GetColor (Color& color) const override
    {
        return GetShapeColor ((const TopoDS_Shape&) face, colorTool, color);
    }

private:
    const Handle (XCAFDoc_ColorTool)& colorTool;
};

class XcafShapeMesh : public Mesh
{
public:
    XcafShapeMesh (const TopoDS_Shape& shape, const Handle (XCAFDoc_ShapeTool)& shapeTool, const Handle (XCAFDoc_ColorTool)& colorTool) :
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

    virtual bool GetColor (Color& color) const override
    {
        return GetShapeColor (shape, colorTool, color);
    }

    virtual void EnumerateFaces (const std::function<void (const Face& face)>& onFace) const override
    {
        for (TopExp_Explorer ex (shape, TopAbs_FACE); ex.More (); ex.Next ()) {
            const TopoDS_Face& face = TopoDS::Face (ex.Current ());
            XcafFace outputFace (face, colorTool);
            onFace (outputFace);
        }
    }

private:
    const TopoDS_Shape& shape;
    const Handle (XCAFDoc_ShapeTool)& shapeTool;
    const Handle (XCAFDoc_ColorTool)& colorTool;
};

class XcafStandaloneFacesMesh : public Mesh
{
public:
    XcafStandaloneFacesMesh (const TopoDS_Shape& shape, const Handle (XCAFDoc_ColorTool)& colorTool) :
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

    virtual bool GetColor (Color&) const override
    {
        return false;
    }

    virtual void EnumerateFaces (const std::function<void (const Face& face)>& onFace) const override
    {
        for (TopExp_Explorer ex (shape, TopAbs_FACE, TopAbs_SHELL); ex.More (); ex.Next ()) {
            const TopoDS_Face& face = TopoDS::Face (ex.Current ());
            XcafFace outputFace (face, colorTool);
            onFace (outputFace);
        }
    }

private:
    const TopoDS_Shape& shape;
    const Handle (XCAFDoc_ColorTool)& colorTool;
};

class XcafNode : public Node
{
public:
    XcafNode (const TDF_Label& label, const Handle (XCAFDoc_ShapeTool)& shapeTool, const Handle (XCAFDoc_ColorTool)& colorTool) :
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
                children.push_back (std::make_shared<const XcafNode> (
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
            XcafShapeMesh outputShapeMesh (currentShape, shapeTool, colorTool);
            onMesh (outputShapeMesh);
        }

        // Enumerate shells that are not part of a solid
        for (TopExp_Explorer ex (shape, TopAbs_SHELL, TopAbs_SOLID); ex.More (); ex.Next ()) {
            const TopoDS_Shape& currentShape = ex.Current ();
            XcafShapeMesh outputShapeMesh (currentShape, shapeTool, colorTool);
            onMesh (outputShapeMesh);
        }

        // Create a mesh from faces that are not part of a shell
        XcafStandaloneFacesMesh standaloneFacesMesh (shape, colorTool);
        if (standaloneFacesMesh.HasFaces ()) {
            onMesh (standaloneFacesMesh);
        }
    }

    TDF_Label label;
    const Handle (XCAFDoc_ShapeTool)& shapeTool;
    const Handle (XCAFDoc_ColorTool)& colorTool;
};

class XcafRootNode : public Node
{
public:
    XcafRootNode (const Handle (XCAFDoc_ShapeTool)& shapeTool, const Handle (XCAFDoc_ColorTool)& colorTool) :
        shapeTool (shapeTool),
        colorTool (colorTool)
    {

    }

    virtual std::string GetName () const override
    {
        return std::string ();
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
                children.push_back (std::make_shared<const XcafNode> (
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

    virtual void EnumerateMeshes (const std::function<void (const Mesh&)>&) const override
    {

    }

private:
    const Handle (XCAFDoc_ShapeTool)& shapeTool;
    const Handle (XCAFDoc_ColorTool)& colorTool;
};

ImporterXcaf::ImporterXcaf () :
    Importer (),
    document (nullptr),
    shapeTool (nullptr),
    colorTool (nullptr)
{

}

Importer::Result ImporterXcaf::LoadFile (const std::vector<std::uint8_t>& fileContent)
{
    if (!TransferToDocument (fileContent)) {
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

NodePtr ImporterXcaf::GetRootNode () const
{
    return std::make_shared<const XcafRootNode> (shapeTool, colorTool);
}
