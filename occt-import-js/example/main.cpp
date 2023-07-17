#include <iostream>
#include <fstream>

#include "importer-step.hpp"
#include "importer-iges.hpp"
#include "importer-brep.hpp"

class ObjWriter
{
public:
    ObjWriter () :
        objFile ("result.obj"),
        vertexCount (0),
        meshCount (0)
    {

    }

    ~ObjWriter ()
    {
        objFile.close ();
    }

    void OnMesh (const Mesh& mesh)
    {
        std::cout << "Mesh Start" << std::endl;
        objFile << "g " << meshCount << std::endl;
        mesh.EnumerateFaces ([&](const Face& face) {
            std::uint32_t faceVertexCount = 0;
            std::cout << "  Face Start" << std::endl;
            face.EnumerateVertices ([&](double x, double y, double z) {
                std::cout << "    Vertex: " << x << ", " << y << ", " << z << std::endl;
                objFile << "v " << x << " " << y << " " << z << std::endl;
                faceVertexCount += 1;
            });
            face.EnumerateNormals ([&](double x, double y, double z) {
                std::cout << "    Normal: " << x << ", " << y << ", " << z << std::endl;
                objFile << "vn " << x << " " << y << " " << z << std::endl;
            });
            face.EnumerateTriangles ([&](int v0, int v1, int v2) {
                std::cout << "    Triangle: " << v0 << ", " << v1 << ", " << v2 << std::endl;
                objFile << "f ";
                objFile << (vertexCount + v0 + 1) << "//" << (vertexCount + v0 + 1) << " ";
                objFile << (vertexCount + v1 + 1) << "//" << (vertexCount + v1 + 1) << " ";
                objFile << (vertexCount + v2 + 1) << "//" << (vertexCount + v2 + 1) << " ";
                objFile << std::endl;
            });
            std::cout << "  Face End" << std::endl;
            vertexCount += faceVertexCount;
        });
        std::cout << "Mesh End " << std::endl;
        meshCount += 1;
    }

    std::ofstream objFile;
    std::uint32_t vertexCount;
    std::uint32_t meshCount;
};

static void WriteNode (const NodePtr& node, ObjWriter& writer)
{
    std::string name = node->GetName ();
    if (node->IsMeshNode ()) {
        node->EnumerateMeshes ([&](const Mesh& mesh) {
            writer.OnMesh (mesh);
        });
    }
    std::vector<NodePtr> children = node->GetChildren ();
    for (const NodePtr& child : children) {
        WriteNode (child, writer);
    }
}

static void DumpNode (const NodePtr& node, int level)
{
    auto writeIndent = [](int indentLevel) {
        for (int i = 0; i < indentLevel; i++) {
            std::cout << "  ";
        }
    };

    writeIndent (level);
    std::cout << "-> Node: " << node->GetName ();
    std::cout << std::endl;

    node->EnumerateMeshes ([&](const Mesh& mesh) {
        writeIndent (level);
        std::cout << "   Mesh: " << mesh.GetName ();
        Color color;
        if (mesh.GetColor (color)) {
            std::cout << " (" << color.r << ", " << color.g << ", " << color.b << ")";
        }
        std::cout << std::endl;
    });

    std::vector<NodePtr> children = node->GetChildren ();
    for (const NodePtr& child : children) {
        DumpNode (child, level + 1);
    }
}

int main (int argc, const char* argv[])
{
    if (argc < 2) {
        return 1;
    }

    std::string fileName = argv[1];
    size_t extensionStart = fileName.find_last_of ('.');
    if (extensionStart == std::string::npos) {
        return 1;
    }
    std::string extension = fileName.substr (extensionStart);
    for (size_t i = 0; i < extension.length (); i++) {
        extension[i] = std::tolower (extension[i]);
    }

    ImporterPtr importer = nullptr;
    if (extension == ".stp" || extension == ".step") {
        importer = std::make_shared<ImporterStep> ();
    } else if (extension == ".igs" || extension == ".iges") {
        importer = std::make_shared<ImporterIges> ();
    } else if (extension == ".brp" || extension == ".brep") {
        importer = std::make_shared<ImporterBrep> ();
    } else {
        return 1;
    }

    ImportParams params;
    params.linearUnit = ImportParams::LinearUnit::Meter;
    Importer::Result result = importer->LoadFile (argv[1], params);
    if (result != Importer::Result::Success) {
        return 1;
    }

    ObjWriter writer;
    WriteNode (importer->GetRootNode (), writer);

    DumpNode (importer->GetRootNode (), 0);

    system ("PAUSE");
    return 0;
}
