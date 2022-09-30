#pragma once

#include "importer-xcaf.hpp"

class ImporterIges : public ImporterXcaf
{
public:
    ImporterIges ();

private:
    virtual bool TransferToDocument (const std::vector<std::uint8_t>& fileContent) override;
};
