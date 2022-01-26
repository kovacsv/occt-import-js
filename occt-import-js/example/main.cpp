#include <iostream>

#include "importer.hpp"

class ConsoleOutput : public Output
{
public:
	ConsoleOutput ()
	{
	
	}

	virtual void OnShape (const Shape& shape) override
	{
		std::cout << "Shape Start" << std::endl;
		shape.EnumerateFaces ([] (const Face& face) {
			std::cout << "  Face Start" << std::endl;
			face.EnumerateVertices ([] (double x, double y, double z) {
				std::cout << "    Vertex: " << x << ", " << y << ", " << z << std::endl;
			});
			face.EnumerateTriangles ([] (int v0, int v1, int v2) {
				std::cout << "    Trianlge: " << v0 << ", " << v1 << ", " << v2 << std::endl;
			});
			std::cout << "  Face End" << std::endl;
		});
		std::cout << "Shape End" << std::endl;
	}
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
