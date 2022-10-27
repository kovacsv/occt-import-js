#include "importer.hpp"

#include <fstream>

Color::Color () :
    r (0),
    g (0),
    b (0)
{

}

Color::Color (double r, double g, double b) :
    r (r),
    g (g),
    b (b)
{

}

Face::Face ()
{

}

Face::~Face ()
{

}

Mesh::Mesh ()
{

}

Mesh::~Mesh ()
{

}

Node::Node ()
{

}

Node::~Node ()
{

}

TriangulationParams::TriangulationParams () :
    automatic (true),
    linearDeflection (1.0),
    angularDeflection (0.5)
{

}

TriangulationParams::TriangulationParams (double linearDeflection, double angularDeflection) :
    automatic (false),
    linearDeflection (linearDeflection),
    angularDeflection (angularDeflection)
{

}

Importer::Importer ()
{

}

Importer::~Importer ()
{

}

Importer::Result Importer::LoadFile (const std::string& filePath, const TriangulationParams& params)
{
    std::ifstream inputStream (filePath, std::ios::binary);
    if (!inputStream.is_open ()) {
        return Importer::Result::FileNotFound;
    }

    std::vector<std::uint8_t> fileContent (std::istreambuf_iterator<char> (inputStream), {});
    Importer::Result result = LoadFile (fileContent, params);
    inputStream.close ();
    return result;
}
