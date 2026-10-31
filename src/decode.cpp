#include "decode.h"

const char ADSB_CHARSET[] = "#ABCDEFGHIJKLMNOPQRSTUVWXYZ##### ###############0123456789######";

AdsbMessage decode(const std::array<uint8_t, 14>& packet) {
    AdsbMessage result;

    if (packet.size() != 14) {
        return result;
    }

    // Downlink Format (bits 1-5)
    result.df = packet[0] >> 3;

    // Transponder capability (bits 6-8)
    result.ca = packet[0] & 0x07;

    // ICAO address (bits 9-32)
    result.icao = (packet[1] << 16) | (packet[2] << 8) | packet[3];

    // Type code (bits 33-37)
    result.tc = packet[4] >> 3;

    if (result.df != 17) {
        return result;
    }

    // Aircraft identification
    if (result.tc >= 1 && result.tc <= 4) {
        Identification ident;
        ident.category = packet[4] & 0x07;

        uint8_t index = 0;
        std::string callsign;
        // After Type code and Aircraft category the rest of the message bits are characters 6 bits each
        uint64_t message =  static_cast<uint64_t>(packet[5]) << 40 | 
                            static_cast<uint64_t>(packet[6]) << 32 | 
                            static_cast<uint64_t>(packet[7]) << 24 | 
                            static_cast<uint64_t>(packet[8]) << 16 | 
                            static_cast<uint64_t>(packet[9]) << 8 | 
                            static_cast<uint64_t>(packet[10]);
        for (size_t i = 0; i < 8; i++) {
            index = (message >> (6 * (7 - i))) & 0x3f;
            callsign += ADSB_CHARSET[index];
        }
        ident.callsign = callsign;

        result.payload = ident;
    }
    // Surface position
    else if (result.tc >= 5 && result.tc <= 8) { 

    } 
    // Airborne position (w/Baro Altitude)
    else if (result.tc >= 9 && result.tc <= 18) {
        AirbornePosition airpos;

        uint16_t alt12 =    static_cast<uint32_t>(packet[5]) << 4 |
                            static_cast<uint32_t>(packet[6] >> 4);
        bool q_bit = (alt12 >> 4) & 0x1;
        if (q_bit) {
            // remove q_bit
             uint16_t altitude = ((alt12 & 0xfe0) >> 1) | (alt12 & 0xf);
             airpos.altitude = (altitude * 25) - 1000;
        }
        else {
            // alternative encoding with 100 feet increments (do later cause can't find sec:alt_code)
        }

        airpos.cpr_format = (packet[6] >> 2) & 0x1;
        airpos.lat_cpr =    static_cast<uint32_t>(packet[6] & 0x3) << 15 | 
                            static_cast<uint32_t>(packet[7]) << 7 | 
                            static_cast<uint32_t>((packet[8] >> 1) & 0x7f);

        airpos.lon_cpr =    static_cast<uint32_t>(packet[8] & 0x1) << 16 | 
                            static_cast<uint32_t>(packet[9]) << 8 | 
                            static_cast<uint32_t>(packet[10]);

        result.payload = airpos;
    }
    // Airborne velocities
    else if (result.tc == 19) {
        AirborneVelocity airvelo;
        airvelo.subtype = (packet[4] & 0x7);
        airvelo.vr_sign = (packet[8] >> 3) & 0x1;
        // airvelo.vertical_rate = (packet[8] & 0x7) | 
    }
    // Airborne position (w/GNSS Height)
    else if (result.tc >= 20 && result.tc <= 22) {

    }
    // // Reserved
    // else if (result.tc >= 23 && result.tc <= 27) {

    // }
    // Aircraft status
    else if (result.tc == 28) {

    } 
    // Target State and status information
    else if (result.tc == 29) {

    }
    // Aircraft operation status
    else if (result.tc == 31) {

    }
    else {
        
    }
    return result;
}