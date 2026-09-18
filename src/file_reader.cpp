#include "file_reader.h"
#include <fstream>
#include <stdexcept>

std::vector<uint8_t> readRawFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open file: " + path);
    }
    return std::vector<uint8_t>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}