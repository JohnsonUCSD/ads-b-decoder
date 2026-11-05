#include <array>
#include <string>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "crc.h"
#include "decode.h"


std::array<uint8_t, 14> fromHex(const std::string& hex) {
    std::array<uint8_t, 14> p{};
    for (size_t i = 0; i < 14; i++) {
        p[i] = std::stoi(hex.substr(i * 2, 2), nullptr, 16);
    }
    return p;
}

TEST_CASE("CRC Parity Validation") {

    // No bit error
    auto packet1 = fromHex("8F4D20235877A0BBBF997CDB827B");
    CHECK(checkCRC(packet1) == true);

    // 1 Bit error
    auto packet2 = fromHex("8F4D20235877A0BCBF997CDB827B"); 
    CHECK(checkCRC(packet2) == false);

}

TEST_CASE("Airborne Position Messages (TC 9-18)") {

    auto packet = fromHex("8F4D20235877A0BBBF997CDB827B");

    AdsbMessage msg = decode(packet);

    CHECK(checkCRC(packet) == true); 
    
    CHECK(msg.df == 17);
    CHECK(msg.ca == 7);
    CHECK(msg.icao == 0x4D2023);
    CHECK(msg.tc == 11);
}