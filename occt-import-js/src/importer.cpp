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

Importer::Importer ()
{

}

Importer::~Importer ()
{

}

Importer::Result Importer::LoadFile (const std::string& filePath)
{
    std::ifstream inputStream (filePath, std::ios::binary);
    if (!inputStream.is_open ()) {
        return Importer::Result::FileNotFound;
    }

    std::vector<std::uint8_t> fileContent (std::istreambuf_iterator<char> (inputStream), {});
    Importer::Result result = LoadFile (fileContent);
    inputStream.close ();
    return result;
}
