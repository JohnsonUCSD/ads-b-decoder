#include <vector>
#include <cstdint>
#include <string>

#include <optional>

struct AdsbMessage {
    uint8_t df = 0;     // Downlink Format (bits 1-5)
    uint8_t ca = 0;     // Transponder capability (bits 6-8)
    uint32_t icao = 0;  // ICAO address (bits 9-32)
    uint8_t tc = 0;     // Type code (bits 33-37)

    // Aircraft identification (TC 1-4)
    std::optional<uint8_t> category;
    std::optional<std::string> callsign; 

    // Airborne Position (TC 9-8 and 20-22)
    std::optional<int> altitude;            // in feet
    std::optional<bool> format_flag;        // true = odd frame, false = even frame
    std::optional<bool> time_flag           // 
    std::optional<uint32_t> lat_cpr;
    std::optional<uint32_t> lon_cpr;

};

AdsbMessage decode(const std::vector<uint8_t>& message);