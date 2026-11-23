#include <vector>
#include <array>
#include <cstdint>
#include <unordered_set>

uint32_t remainderCRC(const std::array<uint8_t, 14>& message);

bool checkCRC(const std::array<uint8_t, 14>& message, std::unordered_set<uint32_t>& known_icaos);