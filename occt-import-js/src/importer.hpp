#ifndef IMPORTER_HPP
#define IMPORTER_HPP

#include <string>
#include <vector>
#include <functional>

class Face
{
public:
	virtual void EnumerateVertices (const std::function<void (double, double, double)>& onVertex) const = 0;
	virtual void EnumerateTriangles (const std::function<void (int, int, int)>& onTriangle) const = 0;
};

class Shape
{
public:
	virtual void EnumerateFaces (const std::function<void (const Face& face)>& onFace) const = 0;
};

class Output
{
public:
	virtual void OnShape (const Shape& shape) = 0;
};

enum class Result
{
	Success = 0,
	FileNotFound = 1,
	ImportFailed = 2
};

Result ReadStepFile (const std::string& filePath, Output& output);
Result ReadStepFile (const std::vector<std::uint8_t>& fileContent, Output& output);

#endif
