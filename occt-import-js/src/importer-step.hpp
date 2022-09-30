#pragma once

#include "importer-xcaf.hpp"

class ImporterStep : public ImporterXcaf
{
public:
    ImporterStep ();

private:
    virtual bool TransferToDocument (const std::vector<std::uint8_t>& fileContent) override;
};
