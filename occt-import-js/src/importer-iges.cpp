#include "importer-iges.hpp"
#include "importer-utils.hpp"

#include <TDocStd_Document.hxx>
#include <IGESCAFControl_Reader.hxx>

ImporterIges::ImporterIges () :
    ImporterXcaf ()
{

}

bool ImporterIges::TransferToDocument (const std::vector<std::uint8_t>& fileContent)
{
    // IGESCAFControl_Reader::ReadStream is not implemented, so the stream
    // should be written to a temporary file to import the content from
    std::string dummyFileName = "temp.igs";
    std::ofstream dummyFile;
    dummyFile.open (dummyFileName, std::ios::binary);
    dummyFile.write ((char*) fileContent.data (), fileContent.size ());
    dummyFile.close ();

    IGESCAFControl_Reader igesCafReader;
    igesCafReader.SetColorMode (true);
    igesCafReader.SetNameMode (true);

    IFSelect_ReturnStatus readStatus = igesCafReader.ReadFile (dummyFileName.c_str ());
    if (readStatus != IFSelect_RetDone) {
        std::remove (dummyFileName.c_str ());
        return false;
    }

    if (!igesCafReader.Transfer (document)) {
        std::remove (dummyFileName.c_str ());
        return false;
    }

    std::remove (dummyFileName.c_str ());
    return true;
}
