#pragma once

#include "importer.hpp"

#include <TDocStd_Document.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>

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
    NodePtr rootNode;
};
