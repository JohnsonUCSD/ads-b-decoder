#include <vector>
#include <array>
#include <cstdint>

std::vector<double> computeMagnitudes(const std::vector<uint8_t>& raw);
std::vector<size_t> detectPreamble(const std::vector<double>& magnitudes);
std::array<uint8_t, 14> demodulateMessage(const std::vector<double>& magnitudes, size_t preamble_idx);