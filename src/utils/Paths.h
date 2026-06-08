#pragma once

#include <string>

namespace Paths {

std::string executableDirectory();
std::string currentWorkingDirectory();
std::string resolve(const std::string& path);
std::string writableLogPath(const std::string& filename);

} // namespace Paths
