#ifdef EMSCRIPTEN

#include "js-interface.hpp"
#include "importer-step.hpp"
#include "importer-iges.hpp"
#include "importer-brep.hpp"
#include <emscripten/bind.h>

class HierarchyWriter
{
public:
    HierarchyWriter (emscripten::val& meshesArr) :
        meshesArr (meshesArr),
        meshCount (0)
    {
    }

    void WriteNode (const NodePtr& node, emscripten::val& nodeObj)
    {
        nodeObj.set ("name", node->GetName ());

        emscripten::val nodeMeshesArr (emscripten::val::array ());
        WriteMeshes (node, nodeMeshesArr);
        nodeObj.set ("meshes", nodeMeshesArr);

        std::vector<NodePtr> children = node->GetChildren ();
        emscripten::val childrenArr (emscripten::val::array ());
        for (int childIndex = 0; childIndex < children.size (); childIndex++) {
            const NodePtr& child = children[childIndex];
            emscripten::val childNodeObj (emscripten::val::object ());
            WriteNode (child, childNodeObj);
            childrenArr.set (childIndex, childNodeObj);
        }
        nodeObj.set ("children", childrenArr);
    }

    void WriteMeshes (const NodePtr& node, emscripten::val& nodeMeshesArr)
    {
        if (!node->IsMeshNode ()) {
            return;
        }

        int nodeMeshCount = 0;
        node->EnumerateMeshes ([&](const Mesh& mesh) {
            int vertexCount = 0;
            int normalCount = 0;
            int triangleCount = 0;
            int faceColorCount = 0;

            emscripten::val positionArr (emscripten::val::array ());
            emscripten::val normalArr (emscripten::val::array ());
            emscripten::val indexArr (emscripten::val::array ());
            emscripten::val faceColorArr (emscripten::val::array ());

            mesh.EnumerateFaces ([&](const Face& face) {
                int triangleOffset = triangleCount;
                int vertexOffset = vertexCount;
                face.EnumerateVertices ([&](double x, double y, double z) {
                    positionArr.set (vertexCount * 3, x);
                    positionArr.set (vertexCount * 3 + 1, y);
                    positionArr.set (vertexCount * 3 + 2, z);
                    vertexCount += 1;
                });
                face.EnumerateNormals ([&](double x, double y, double z) {
                    normalArr.set (normalCount * 3, x);
                    normalArr.set (normalCount * 3 + 1, y);
                    normalArr.set (normalCount * 3 + 2, z);
                    normalCount += 1;
                });
                face.EnumerateTriangles ([&](int v0, int v1, int v2) {
                    indexArr.set (triangleCount * 3, vertexOffset + v0);
                    indexArr.set (triangleCount * 3 + 1, vertexOffset + v1);
                    indexArr.set (triangleCount * 3 + 2, vertexOffset + v2);
                    triangleCount += 1;
                });
                Color faceColor;
                if (face.GetColor (faceColor)) {
                    emscripten::val faceColorObj (emscripten::val::object ());
                    faceColorObj.set ("first", triangleOffset);
                    faceColorObj.set ("last", triangleCount - 1);
                    emscripten::val colorArr (emscripten::val::array ());
                    colorArr.set (0, faceColor.r);
                    colorArr.set (1, faceColor.g);
                    colorArr.set (2, faceColor.b);
                    faceColorObj.set ("color", colorArr);
                    faceColorArr.set (faceColorCount, faceColorObj);
                    faceColorCount += 1;
                }
            });

            emscripten::val meshObj (emscripten::val::object ());
            meshObj.set ("name", mesh.GetName ());
            Color meshColor;
            if (mesh.GetColor (meshColor)) {
                emscripten::val colorArr (emscripten::val::array ());
                colorArr.set (0, meshColor.r);
                colorArr.set (1, meshColor.g);
                colorArr.set (2, meshColor.b);
                meshObj.set ("color", colorArr);
            }

            if (faceColorCount > 0) {
                meshObj.set ("face_colors", faceColorArr);
            }

            emscripten::val attributesObj (emscripten::val::object ());

            emscripten::val positionObj (emscripten::val::object ());
            positionObj.set ("array", positionArr);
            attributesObj.set ("position", positionObj);

            if (vertexCount == normalCount) {
                emscripten::val normalObj (emscripten::val::object ());
                normalObj.set ("array", normalArr);
                attributesObj.set ("normal", normalObj);
            }

            emscripten::val indexObj (emscripten::val::object ());
            indexObj.set ("array", indexArr);

            meshObj.set ("attributes", attributesObj);
            meshObj.set ("index", indexObj);

            meshesArr.set (meshCount, meshObj);
            nodeMeshesArr.set (nodeMeshCount, meshCount);
            meshCount += 1;
            nodeMeshCount += 1;
        });
    }

    emscripten::val& meshesArr;
    int meshCount;
};

static void EnumerateNodeMeshes (const NodePtr& node, const std::function<void (const Mesh&)>& onMesh)
{
    if (node->IsMeshNode ()) {
        node->EnumerateMeshes (onMesh);
    }
    std::vector<NodePtr> children = node->GetChildren ();
    for (const NodePtr& child : children) {
        EnumerateNodeMeshes (child, onMesh);
    }
}

static emscripten::val ReadFile (ImporterPtr importer, const emscripten::val& content)
{
    emscripten::val resultObj (emscripten::val::object ());

    const std::vector<uint8_t>& contentArr = emscripten::vecFromJSArray<std::uint8_t> (content);
    Importer::Result importResult = importer->LoadFile (contentArr);
    resultObj.set ("success", importResult == Importer::Result::Success);
    if (importResult != Importer::Result::Success) {
        return resultObj;
    }

    int meshIndex = 0;
    emscripten::val rootNodeObj (emscripten::val::object ());
    emscripten::val meshesArr (emscripten::val::array ());
    NodePtr rootNode = importer->GetRootNode ();

    HierarchyWriter hierarchyWriter (meshesArr);
    hierarchyWriter.WriteNode (rootNode, rootNodeObj);

    resultObj.set ("root", rootNodeObj);
    resultObj.set ("meshes", meshesArr);
    return resultObj;
}

emscripten::val ReadStepFile (const emscripten::val& content)
{
    ImporterPtr importer = std::make_shared<ImporterStep> ();
    return ReadFile (importer, content);
}

emscripten::val ReadIgesFile (const emscripten::val& content)
{
    ImporterPtr importer = std::make_shared<ImporterIges> ();
    return ReadFile (importer, content);
}

emscripten::val ReadBrepFile (const emscripten::val& content)
{
    ImporterPtr importer = std::make_shared<ImporterBrep> ();
    return ReadFile (importer, content);
}

EMSCRIPTEN_BINDINGS (assimpjs)
{
    emscripten::function<emscripten::val, const emscripten::val&> ("ReadStepFile", &ReadStepFile);
    emscripten::function<emscripten::val, const emscripten::val&> ("ReadIgesFile", &ReadIgesFile);
    emscripten::function<emscripten::val, const emscripten::val&> ("ReadBrepFile", &ReadBrepFile);
}

#endif
