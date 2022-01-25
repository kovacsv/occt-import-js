#ifndef OCCTIMPORTJS_HPP
#define OCCTIMPORTJS_HPP

#include <string>
#include <vector>

const std::string StepToJson (const std::string& filePath);
const std::string StepToJson (const std::vector<std::uint8_t>& fileContent);

#endif
