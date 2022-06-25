#include <iostream>
#include <fstream>

#include "importer.hpp"

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
		mesh.EnumerateFaces ([&] (const Face& face) {
			std::uint32_t faceVertexCount = 0;
			std::cout << "  Face Start" << std::endl;
			face.EnumerateVertices ([&] (double x, double y, double z) {
				std::cout << "    Vertex: " << x << ", " << y << ", " << z << std::endl;
				objFile << "v " << x << " " << y << " " << z << std::endl;
				faceVertexCount += 1;
			});
			face.EnumerateNormals ([&] (double x, double y, double z) {
				std::cout << "    Normal: " << x << ", " << y << ", " << z << std::endl;
				objFile << "vn " << x << " " << y << " " << z << std::endl;
			});
			face.EnumerateTriangles ([&] (int v0, int v1, int v2) {
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

	std::ofstream		objFile;
	std::uint32_t		vertexCount;
	std::uint32_t		meshCount;
};

static void WriteNode (const NodePtr& node, ObjWriter& writer)
{
	std::string name = node->GetName ();
	if (node->IsMeshNode ()) {
		node->EnumerateMeshes ([&] (const Mesh& mesh) {
			writer.OnMesh (mesh);
		});
	}
	std::vector<NodePtr> children = node->GetChildren ();
	for (const NodePtr& child : children) {
		WriteNode (child, writer);
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

	Importer::Format format = Importer::Format::Step;
	if (extension == ".stp" || extension == ".step") {
		format = Importer::Format::Step;
	} else if (extension == ".igs" || extension == ".iges") {
		format = Importer::Format::Iges;
	} else {
		return 1;
	}

	Importer importer;
	Importer::Result result = importer.LoadFile (format, argv[1]);
	if (result != Importer::Result::Success) {
		return 1;
	}

	importer.DumpHierarchy ();

	ObjWriter writer;
	WriteNode (importer.GetRootNode (), writer);

	return 0;
}
