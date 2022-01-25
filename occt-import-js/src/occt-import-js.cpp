#include "occt-import-js.hpp"

#include <STEPControl_Reader.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <BRepMesh_IncrementalMesh.hxx>

const std::string StepToJson (const std::string& filePath)
{
	STEPControl_Reader reader;
 	IFSelect_ReturnStatus readStatus = reader.ReadFile (filePath.c_str ());
	if (readStatus != IFSelect_RetDone) {
		return ""; // TODO
	}

	reader.TransferRoots ();
	
	for (Standard_Integer rank = 1; rank <= reader.NbShapes (); rank++) {
		TopoDS_Shape shape = reader.Shape (rank);
		
		// TODO: calculate accuracy based on bounding box
		double accuracy = 1.0;

		// Calculate triangulation
		BRepMesh_IncrementalMesh mesh (shape, accuracy);

		for (TopExp_Explorer explorer (shape, TopAbs_FACE); explorer.More (); explorer.Next ()) {
			const TopoDS_Face& face = TopoDS::Face (explorer.Current ());

			TopLoc_Location location;
			Handle (Poly_Triangulation) triangulation = BRep_Tool::Triangulation (face, location);
			if (triangulation.IsNull () || triangulation->NbNodes () == 0 || triangulation->NbTriangles () == 0) {
				continue;
			}
			// TODO: get transformation from location
			for (Standard_Integer nodeIndex = 1; nodeIndex <= triangulation->NbNodes (); nodeIndex++) {
				gp_Pnt aPnt = triangulation->Node (nodeIndex);
				continue;
			}
			for (Standard_Integer triangleIndex = 1; triangleIndex <= triangulation->NbTriangles (); triangleIndex++) {
				Poly_Triangle aTri = triangulation->Triangle (triangleIndex);
				continue;
			}
		}
	}

	return "";
}

const std::string StepToJson (const std::vector<std::uint8_t>& fileContent)
{
	return "";
}
