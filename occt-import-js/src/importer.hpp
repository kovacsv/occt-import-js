#ifndef IMPORTER_HPP
#define IMPORTER_HPP

#include <string>
#include <vector>
#include <memory>
#include <functional>

class Color
{
public:
	Color ();
	Color (double r, double g, double b);

	bool hasValue;
	double r;
	double g;
	double b;
};

class Face
{
public:
	virtual bool HasNormals () const = 0;
	virtual Color GetColor () const = 0;

	virtual void EnumerateVertices (const std::function<void (double, double, double)>& onVertex) const = 0;
	virtual void EnumerateNormals (const std::function<void (double, double, double)>& onNormal) const = 0;
	virtual void EnumerateTriangles (const std::function<void (int, int, int)>& onTriangle) const = 0;
};

class Mesh
{
public:
	virtual std::string		GetName () const = 0;
	virtual Color			GetColor () const = 0;
	virtual void			EnumerateFaces (const std::function<void (const Face& face)>& onFace) const = 0;
};

enum class Result
{
	Success = 0,
	FileNotFound = 1,
	ImportFailed = 2
};

class ImporterImpl;

class Importer
{
public:
	enum class Result
	{
		Success = 0,
		FileNotFound = 1,
		ImportFailed = 2
	};

	Importer ();
	~Importer ();

	Result		LoadStepFile (const std::string& filePath);
	Result		LoadStepFile (const std::vector<std::uint8_t>& fileContent);
	Result		LoadStepFile (std::istream& inputStream);

	void		EnumerateMeshes (const std::function<void (const Mesh&)>& onMesh) const;
	void		DumpHierarchy () const;

private:
	ImporterImpl* impl;
};

#endif
