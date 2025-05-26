#pragma once

#include <filesystem>
#include <string>

namespace Utility
{
std::string loadFile(const std::filesystem::path &filename);
int randomInt(int exclusiveMax);

std::u32string decodeUTF8(std::string_view view);
}
