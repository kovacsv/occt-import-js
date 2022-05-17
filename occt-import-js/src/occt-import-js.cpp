#ifdef EMSCRIPTEN

#include "occt-import-js.hpp"
#include "importer.hpp"
#include <emscripten/bind.h>

static void EnumerateNodeMeshes (const NodePtr& node, const std::function<void (const Mesh&)>& onMesh)
{
	if (node->IsMeshNode ()) {
		node->EnumerateMeshes (onMesh);
	}
	std::vector<NodePtr> children = node->GetChildren ();
	for (const NodePtr& child : children) {
		EnumerateNodeMeshes (child, onMesh);
	}
}

emscripten::val ReadStepFile (const emscripten::val& content)
{
	emscripten::val resultObj (emscripten::val::object ());
	
	Importer importer;
	const std::vector<uint8_t>& contentArr = emscripten::vecFromJSArray<std::uint8_t> (content);
	Importer::Result importResult = importer.LoadStepFile (contentArr);
	resultObj.set ("success", importResult == Importer::Result::Success);
	if (importResult != Importer::Result::Success) {
		return resultObj;
	}

	int meshIndex = 0;
	emscripten::val meshesArr (emscripten::val::array ());
	NodePtr rootNode = importer.GetRootNode ();
	EnumerateNodeMeshes (rootNode, [&] (const Mesh& mesh) {
		int vertexCount = 0;
		int normalCount = 0;
		int triangleCount = 0;
		int faceColorCount = 0;

		emscripten::val positionArr (emscripten::val::array ());
		emscripten::val normalArr (emscripten::val::array ());
		emscripten::val indexArr (emscripten::val::array ());
		emscripten::val faceColorArr (emscripten::val::array ());

		mesh.EnumerateFaces ([&] (const Face& face) {
			int triangleOffset = triangleCount;
			int vertexOffset = vertexCount;
			face.EnumerateVertices ([&](double x, double y, double z) {
				positionArr.set (vertexCount * 3, x);
				positionArr.set (vertexCount * 3 + 1, y);
				positionArr.set (vertexCount * 3 + 2, z);
				vertexCount += 1;
			});
			face.EnumerateNormals ([&](double x, double y, double z) {
				normalArr.set (normalCount * 3, x);
				normalArr.set (normalCount * 3 + 1, y);
				normalArr.set (normalCount * 3 + 2, z);
				normalCount += 1;
			});
			face.EnumerateTriangles ([&](int v0, int v1, int v2) {
				indexArr.set (triangleCount * 3, vertexOffset + v0);
				indexArr.set (triangleCount * 3 + 1, vertexOffset + v1);
				indexArr.set (triangleCount * 3 + 2, vertexOffset + v2);
				triangleCount += 1;
			});
			Color faceColor = face.GetColor ();
			if (faceColor.hasValue) {
				emscripten::val faceColorObj (emscripten::val::object ());
				faceColorObj.set ("first", triangleOffset);
				faceColorObj.set ("last", triangleCount - 1);
				emscripten::val colorArr (emscripten::val::array ());
				colorArr.set (0, faceColor.r);
				colorArr.set (1, faceColor.g);
				colorArr.set (2, faceColor.b);
				faceColorObj.set ("color", colorArr);
				faceColorArr.set (faceColorCount, faceColorObj);
				faceColorCount += 1;
			}
		});

		emscripten::val meshObj (emscripten::val::object ());
		std::string name = mesh.GetName ();
		if (name.length () > 0) {
			meshObj.set ("name", mesh.GetName ());
		}

		Color color = mesh.GetColor ();
		if (color.hasValue) {
			emscripten::val colorArr (emscripten::val::array ());
			colorArr.set (0, color.r);
			colorArr.set (1, color.g);
			colorArr.set (2, color.b);
			meshObj.set ("color", colorArr);
		}

		if (faceColorCount > 0) {
			meshObj.set ("face_colors", faceColorArr);
		}

		emscripten::val attributesObj (emscripten::val::object ());

		emscripten::val positionObj (emscripten::val::object ());
		positionObj.set ("array", positionArr);
		attributesObj.set ("position", positionObj);

		if (vertexCount == normalCount) {
			emscripten::val normalObj (emscripten::val::object ());
			normalObj.set ("array", normalArr);
			attributesObj.set ("normal", normalObj);
		}

		emscripten::val indexObj (emscripten::val::object ());
		indexObj.set ("array", indexArr);

		meshObj.set ("attributes", attributesObj);
		meshObj.set ("index", indexObj);

		meshesArr.set (meshIndex, meshObj);
		meshIndex += 1;
	});

	resultObj.set ("meshes", meshesArr);
	return resultObj;
}

EMSCRIPTEN_BINDINGS (assimpjs)
{
	emscripten::function<emscripten::val, const emscripten::val&> ("ReadStepFile", &ReadStepFile);
}

#endif
