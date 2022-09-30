#pragma once

#include <TopoDS_Face.hxx>

#include <vector>
#include <fstream>

class VectorBuffer : public std::streambuf
{
public:
    VectorBuffer (const std::vector<uint8_t>& v);
};

bool TriangulateShape (TopoDS_Shape& shape);
