#ifdef EMSCRIPTEN

#include "occt-import-js.hpp"
#include "importer.hpp"
#include <emscripten/bind.h>

class EmscriptenOutput : public Output
{
public:
	EmscriptenOutput () :
		resultObj (emscripten::val::object ()),
		meshesArr (emscripten::val::array ()),
		meshIndex (0)
	{
		
	}

	virtual void OnBegin () override
	{

	}

	virtual void OnEnd () override
	{
		resultObj.set ("meshes", meshesArr);
	}

	virtual void OnShape (const Shape& shape) override
	{
		int vertexCount = 0;
		int triangleCount = 0;

		emscripten::val positionArr (emscripten::val::array ());
		emscripten::val indexArr (emscripten::val::array ());
		
		shape.EnumerateFaces ([&] (const Face& face) {
			int vertexOffset = vertexCount;
			face.EnumerateVertices ([&] (double x, double y, double z) {
				positionArr.set (vertexCount * 3, x);
				positionArr.set (vertexCount * 3 + 1, y);
				positionArr.set (vertexCount * 3 + 2, z);
				vertexCount += 1;
			});
			face.EnumerateTriangles ([&] (int v0, int v1, int v2) {
				indexArr.set (triangleCount * 3, vertexOffset + v0 - 1);
				indexArr.set (triangleCount * 3 + 1, vertexOffset + v1 - 1);
				indexArr.set (triangleCount * 3 + 2, vertexOffset + v2 - 1);
				triangleCount += 1;
			});
		});
		
		emscripten::val positionObj (emscripten::val::object ());
		positionObj.set ("array", positionArr);

		emscripten::val indexObj (emscripten::val::object ());
		indexObj.set ("array", indexArr);

		emscripten::val attributesObj (emscripten::val::object ());
		attributesObj.set ("position", positionObj);

		emscripten::val meshObj (emscripten::val::object ());
		meshObj.set ("attributes", attributesObj);
		meshObj.set ("index", indexObj);

		meshesArr.set (meshIndex, meshObj);
		meshIndex += 1;
	}

	emscripten::val		resultObj;
	emscripten::val		meshesArr;
	int					meshIndex;
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
