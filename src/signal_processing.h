#include <vector>
#include <cstdint>

std::vector<double> computeMagnitudes(const std::vector<uint8_t>& raw);
std::vector<size_t> detectPreamble(const std::vector<double>& magnitudes);
std::vector<uint8_t> demodulateBits(const std::vector<double>& magnitudes, size_t preamble_idx);