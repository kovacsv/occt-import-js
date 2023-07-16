#include "importer-brep.hpp"
#include "importer-utils.hpp"

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>

class BrepShapeMesh : public Mesh
{
public:
    BrepShapeMesh (const TopoDS_Shape& shape) :
        Mesh (),
        shape (shape)
    {

    }

    virtual std::string GetName () const override
    {
        return std::string ();
    }

    virtual bool GetColor (Color&) const override
    {
        return false;
    }

    virtual void EnumerateFaces (const std::function<void (const Face&)>& onFace) const override
    {
        for (TopExp_Explorer ex (shape, TopAbs_FACE); ex.More (); ex.Next ()) {
            const TopoDS_Face& face = TopoDS::Face (ex.Current ());
            OcctFace outputFace (face);
            onFace (outputFace);
        }
    }

private:
    const TopoDS_Shape& shape;
};

class BrepStandaloneFacesMesh : public Mesh
{
public:
    BrepStandaloneFacesMesh (const TopoDS_Shape& shape) :
        Mesh (),
        shape (shape)
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
            OcctFace outputFace (face);
            onFace (outputFace);
        }
    }

private:
    const TopoDS_Shape& shape;
};

class BrepNode : public Node
{
public:
    BrepNode (const TopoDS_Shape& shape) :
        shape (shape)
    {

    }

    virtual std::string GetName () const override
    {
        return std::string ();
    }

    virtual std::vector<NodePtr> GetChildren () const override
    {
        return {};
    }

    virtual bool IsMeshNode () const override
    {
        return true;
    }

    virtual void EnumerateMeshes (const std::function<void (const Mesh&)>& onMesh) const override
    {
        // Enumerate solids
        for (TopExp_Explorer ex (shape, TopAbs_SOLID); ex.More (); ex.Next ()) {
            const TopoDS_Shape& currentShape = ex.Current ();
            BrepShapeMesh outputShapeMesh (currentShape);
            onMesh (outputShapeMesh);
        }

        // Enumerate shells that are not part of a solid
        for (TopExp_Explorer ex (shape, TopAbs_SHELL, TopAbs_SOLID); ex.More (); ex.Next ()) {
            const TopoDS_Shape& currentShape = ex.Current ();
            BrepShapeMesh outputShapeMesh (currentShape);
            onMesh (outputShapeMesh);
        }

        // Create a mesh from faces that are not part of a shell
        BrepStandaloneFacesMesh standaloneFacesMesh (shape);
        if (standaloneFacesMesh.HasFaces ()) {
            onMesh (standaloneFacesMesh);
        }
    }

private:
    const TopoDS_Shape& shape;
};

ImporterBrep::ImporterBrep () :
    Importer (),
    shape ()
{

}

Importer::Result ImporterBrep::LoadFile (const std::vector<std::uint8_t>& fileContent, const ImportParams& params)
{
    VectorBuffer inputBuffer (fileContent);
    std::istream inputStream (&inputBuffer);

    BRep_Builder builder;
    BRepTools::Read (shape, inputStream, builder);
    if (shape.IsNull ()) {
        return Importer::Result::ImportFailed;
    }

    TriangulateShape (shape, params);
    return Importer::Result::Success;
}

NodePtr ImporterBrep::GetRootNode () const
{
    return std::make_shared<BrepNode> (shape);
}
