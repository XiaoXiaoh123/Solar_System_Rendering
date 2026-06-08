#include "Paths.h"

#include <array>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#else
#include <limits.h>
#include <unistd.h>
#endif

namespace Paths {

namespace fs = std::filesystem;

std::string executableDirectory() {
#ifdef _WIN32
    std::array<char, MAX_PATH> buffer{};
    DWORD length = GetModuleFileNameA(nullptr, buffer.data(),
                                      static_cast<DWORD>(buffer.size()));
    if (length == 0 || length >= buffer.size()) {
        return fs::current_path().string();
    }
    return fs::path(buffer.data()).parent_path().string();
#else
    std::array<char, PATH_MAX> buffer{};
    ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    if (length <= 0) {
        return fs::current_path().string();
    }
    buffer[static_cast<std::size_t>(length)] = '\0';
    return fs::path(buffer.data()).parent_path().string();
#endif
}

std::string currentWorkingDirectory() {
    return fs::current_path().string();
}

std::string resolve(const std::string& path) {
    fs::path input(path);
    if (input.is_absolute()) {
        return fs::exists(input) ? fs::weakly_canonical(input).string() : input.string();
    }

    fs::path exeDir = executableDirectory();
    std::array<fs::path, 5> bases = {
        fs::current_path(),
        exeDir,
        exeDir / "..",
        exeDir / ".." / "..",
        exeDir / ".." / ".." / ".."
    };

    for (const fs::path& base : bases) {
        fs::path candidate = base / input;
        if (fs::exists(candidate)) {
            return fs::weakly_canonical(candidate).string();
        }
    }

    return path;
}

std::string writableLogPath(const std::string& filename) {
    fs::path exePath = fs::path(executableDirectory()) / filename;
    return exePath.string();
}

} // namespace Paths
