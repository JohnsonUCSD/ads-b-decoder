#include "crc.h"

bool checkCRC(const std::array<uint8_t, 14>& message) {
    const uint32_t GENERATOR = 0x1FFF409;

    // valid messages are 14 bytes
    if (message.size() != 14) {
        return false;
    }

    uint32_t remainder = 0;
    uint32_t message_byte = 0;
    uint32_t shift_amount = 0;
    uint32_t bit = 0;

    for (size_t n = 0; n < 112; n++) {
        // extract message bit n
        message_byte = message[n / 8];
        shift_amount = 7 - (n % 8);
        bit = (message_byte >> shift_amount) & 1;

        // shift remainder left one and add new bit on right
        remainder = remainder << 1;
        remainder = remainder | bit;

        // XOR when generator fits (bit 24 is 1)
        if (remainder & 0x1000000) {
            remainder ^= GENERATOR;
        }

    }

    return remainder == 0;

}