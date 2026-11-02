#include "decode.h"

const char ADSB_CHARSET[] = "#ABCDEFGHIJKLMNOPQRSTUVWXYZ##### ###############0123456789######";

uint32_t getBits(const std::array<uint8_t, 14>& packet, size_t start, size_t length) {
    uint32_t value = 0;

    for (size_t i = start; i < start + length; i++) {
        value = (value << 1) | ((packet[i / 8] >> (7 - i % 8)) & 0x1);
    }

    return value;
}

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
        airvelo.subtype = getBits(packet, 37, 3);
        // Ground speed decoding
        if (airvelo.subtype == 1 || airvelo.subtype == 2) {
            bool ew = getBits(packet, 45, 1);       // 0 = east, 1 = west
            uint32_t ew_velo =  getBits(packet, 46, 10);
            bool ns =  getBits(packet, 56, 1);      // 0 = north, 1 = south
            uint32_t ns_velo =  getBits(packet, 57, 10);

            // 0 means no information, so skip those
            if (ew_velo != 0 && ns_velo != 0) {
                int ew_speed;
                int ns_speed;
                // subsonic
                if (airvelo.subtype == 1) {
                    ew_speed = ew_velo - 1;
                    ns_speed = ns_velo - 1;
                }
                // supersonic
                else {
                    ew_speed = 4*(ew_velo - 1);
                    ns_speed = 4*(ns_velo - 1);
                }
                // applying negatives to ew and ns
                if (ew == 1) {
                    ew_speed = -ew_speed;
                }
                if (ns == 1) {
                    ns_speed = -ns_speed;
                }

                airvelo.speed = std::sqrt((ew_speed*ew_speed) + (ns_speed*ns_speed));

                double angle = std::fmod((std::atan2(ew_speed, ns_speed) * (360 / (2* std::numbers::pi))), 360);
                if (angle < 0) {
                    angle += 360.0;
                }
                airvelo.heading = angle;

            }
        }
        else if (airvelo.subtype == 3 || airvelo.subtype == 4) {
            bool maghead_status = getBits(packet, 45, 1);       // 0 = not available, 1 = available
            uint16_t maghead = getBits(packet, 46, 10);
            airvelo.airspeed_type = getBits(packet, 56, 1);               // 0 = Indicated airspeed (IAS), 1 = True airspeed (TAS)
            uint16_t airspeed = getBits(packet, 57, 10);

            if (maghead_status == 1) {
                airvelo.heading = maghead * (360.0 / 1024.0);
            }

            // 0 means no information available
            if (airspeed != 0) {
                // subsonic
                if (airvelo.subtype == 3) {
                    airvelo.speed = airspeed - 1;
                }
                // supersonic
                else {
                    airvelo.speed = 4 * (airspeed - 1);
                }
            }
        }

        airvelo.vr_sign = getBits(packet, 68, 1);               // 0 = up, 1 = down
        airvelo.source_vr = getBits(packet, 67, 1);             // 0 = GNSS, 1 = baro
        uint16_t vertical_rate = getBits(packet, 69, 9);
        // vertical rate of 0 means information not available, so skip
        if (vertical_rate != 0) {
            if (airvelo.vr_sign == 0){
                airvelo.vertical_rate = 64 * (vertical_rate - 1);
            }
            else{
                airvelo.vertical_rate = -64 * (vertical_rate - 1);
            }
        }

        result.payload = airvelo;
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