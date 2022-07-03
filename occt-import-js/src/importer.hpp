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

class Node;
using NodePtr = std::shared_ptr<const Node>;

class Node
{
public:
	virtual std::string GetName () const = 0;
	virtual std::vector<NodePtr> GetChildren () const = 0;

	virtual bool IsMeshNode () const = 0;
	virtual void EnumerateMeshes (const std::function<void (const Mesh&)>& onMesh) const = 0;
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
	enum class Format
	{
		Step = 0,
		Iges = 1
	};

	enum class Result
	{
		Success = 0,
		FileNotFound = 1,
		ImportFailed = 2
	};

	Importer ();
	~Importer ();

	Result		LoadFile (Format format, const std::string& filePath);
	Result		LoadFile (Format format, const std::vector<std::uint8_t>& fileContent);

	NodePtr		GetRootNode () const;
	void		DumpHierarchy () const;

private:
	ImporterImpl* impl;
};

#endif
