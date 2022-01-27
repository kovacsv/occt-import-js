#ifdef EMSCRIPTEN

#include "occt-import-js.hpp"
#include "importer.hpp"
#include <emscripten/bind.h>

class EmscriptenOutput : public Output
{
public:
	EmscriptenOutput () :
		resultObj (emscripten::val::object ()),
		shapesArr (emscripten::val::array ()),
		shapeIndex (0)
	{
		
	}

	virtual void OnBegin () override
	{

	}

	virtual void OnEnd () override
	{
		resultObj.set ("shapes", shapesArr);
	}

	virtual void OnShape (const Shape& shape) override
	{
		int vertexCount = 0;
		int triangleCount = 0;
		emscripten::val vertices (emscripten::val::array ());
		emscripten::val triangles (emscripten::val::array ());
		shape.EnumerateFaces ([&] (const Face& face) {
			int vertexOffset = vertexCount;
			face.EnumerateVertices ([&] (double x, double y, double z) {
				vertices.set (vertexCount * 3, x);
				vertices.set (vertexCount * 3 + 1, y);
				vertices.set (vertexCount * 3 + 2, z);
				vertexCount += 1;
			});
			face.EnumerateTriangles ([&] (int v0, int v1, int v2) {
				triangles.set (triangleCount * 3, vertexOffset + v0 - 1);
				triangles.set (triangleCount * 3 + 1, vertexOffset + v1 - 1);
				triangles.set (triangleCount * 3 + 2, vertexOffset + v2 - 1);
				triangleCount += 1;
			});
		});
		emscripten::val shapeObj (emscripten::val::object ());
		shapeObj.set ("vertices", vertices);
		shapeObj.set ("triangles", triangles);
		shapesArr.set (shapeIndex, shapeObj);
		shapeIndex += 1;
	}

	emscripten::val		resultObj;
	emscripten::val		shapesArr;
	int					shapeIndex;
};

emscripten::val ReadStepFile (const emscripten::val& content)
{
	const std::vector<uint8_t>& contentArr = emscripten::vecFromJSArray<std::uint8_t> (content);
	EmscriptenOutput output;
	Result result = ReadStepFile (contentArr, output);
	output.resultObj.set ("success", result == Result::Success);
	return output.resultObj;
}

EMSCRIPTEN_BINDINGS (assimpjs)
{
	emscripten::function<emscripten::val, const emscripten::val&> ("ReadStepFile", &ReadStepFile);
}

#endif
