#include "occt-import-js.hpp"
#include "importer.hpp"

#ifdef EMSCRIPTEN
#include <emscripten/bind.h>
#endif

class EmscriptenOutput : public Output
{
public:
	EmscriptenOutput ()
	{
	
	}

	virtual void OnShape (const Shape& shape) override
	{
		shape.EnumerateFaces ([] (const Face& face) {
			face.EnumerateVertices ([] (double x, double y, double z) {
				return;
			});
			face.EnumerateTriangles ([] (int v0, int v1, int v2) {
				return;
			});
		});
	}
};

#ifdef EMSCRIPTEN

int ReadStepFile (const emscripten::val& content)
{
	const std::vector<uint8_t>& contentArr = emscripten::vecFromJSArray<std::uint8_t> (content);
	EmscriptenOutput output;
	return (int) ReadStepFile (contentArr, output);
}

EMSCRIPTEN_BINDINGS (assimpjs)
{
	emscripten::function<int, const emscripten::val&> ("ReadStepFile", &ReadStepFile);
}

#endif
