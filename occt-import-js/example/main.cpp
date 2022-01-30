#include <iostream>
#include <fstream>

#include "importer.hpp"

class ConsoleOutput : public Output
{
public:
	ConsoleOutput () :
		objFile ("result.obj"),
		vertexCount (0)
	{
		
	}

	~ConsoleOutput ()
	{
		objFile.close ();
	}

	virtual void OnBegin () override
	{

	}

	virtual void OnEnd () override
	{
	
	}

	virtual void OnShape (const Shape& shape) override
	{
		std::cout << "Shape Start" << std::endl;
		shape.EnumerateFaces ([&] (const Face& face) {
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
		std::cout << "Shape End " << std::endl;
	}

	std::ofstream		objFile;
	std::uint32_t		vertexCount;
};

int main (int argc, const char* argv[])
{
	if (argc < 2) {
		return 1;
	}

	ConsoleOutput output;
	ReadStepFile (argv[1], output);

	return 0;
}
