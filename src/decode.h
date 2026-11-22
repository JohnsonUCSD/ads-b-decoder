#include <vector>
#include <cstdint>
#include <string>
#include <array>

#include <cmath>
#include <numbers>

#include <variant>
#include <optional>

// Aircraft identification (tc 1-4)
struct Identification {
    uint8_t category;
    std::string callsign;
};

// Airborne position (tc 9-18)
struct AirbornePosition {
    int altitude;           // 
    bool cpr_format;        // 0 = even frame, 1 = odd frame
    uint32_t lat_cpr;
    uint32_t lon_cpr;    
};

// Airborne velocities (tc 19)
struct AirborneVelocity {
    uint8_t subtype;
    bool source_vr;     // 0 = GNSS, 1 = Barometer
    bool vr_sign;       // 0 = up, 1 = down
    
    // optional because 0 means no information 
    std::optional<double> speed;
    std::optional<double> heading;
    std::optional<int> vertical_rate;

    // only for subtype 3 and 4
    std::optional<bool> airspeed_type;
};

struct AdsbMessage {
    uint8_t df = 0;     // Downlink Format (bits 1-5)
    uint8_t ca = 0;     // Transponder capability (bits 6-8)
    uint32_t icao = 0;  // ICAO address (bits 9-32)
    uint8_t tc = 0;     // Type code (bits 33-37)

    std::variant<std::monostate, Identification, AirbornePosition, AirborneVelocity> payload;
};

AdsbMessage decode(const std::array<uint8_t, 14>& message);