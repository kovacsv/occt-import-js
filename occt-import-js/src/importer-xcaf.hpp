#pragma once

#include "importer.hpp"

#include <TDocStd_Document.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <NCollection_DataMap.hxx>
#include <TopTools_ShapeMapHasher.hxx>
#include <TopoDS_Face.hxx>

// Built once per document, so XCAFDoc_ShapeTool::Search does not walk the document for every face
class XcafLookup
{
public:
    XcafLookup (const Handle (XCAFDoc_ShapeTool)& shapeTool, const Handle (XCAFDoc_ColorTool)& colorTool);

    bool Search (const TopoDS_Shape& shape, TDF_Label& label) const;
    bool GetFaceColor (const TopoDS_Face& face, Color& color) const;

private:
    void CollectComponents ();
    void CollectFaceColors (const Handle (XCAFDoc_ColorTool)& colorTool);

    Handle (XCAFDoc_ShapeTool) shapeTool;
    NCollection_DataMap<TopoDS_Shape, TDF_Label, TopTools_ShapeMapHasher> componentLabels;
    NCollection_DataMap<TopoDS_Shape, Color, TopTools_ShapeMapHasher> faceColors;
};

class ImporterXcaf : public Importer
{
public:
    ImporterXcaf ();

    virtual Result LoadFile (const std::vector<std::uint8_t>& fileContent, const ImportParams& params) override;
    virtual NodePtr GetRootNode () const override;

protected:
    virtual bool TransferToDocument (const std::vector<std::uint8_t>& fileContent) = 0;

    Handle (TDocStd_Document) document;
    Handle (XCAFDoc_ShapeTool) shapeTool;
    Handle (XCAFDoc_ColorTool) colorTool;
    std::unique_ptr<const XcafLookup> lookup;
    NodePtr rootNode;
};
