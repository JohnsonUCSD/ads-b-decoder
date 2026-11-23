#include "crc.h"
#include "bits.h"



static constexpr uint32_t GENERATOR = 0x1FFF409;

/**
 * @brief Computes the Mode-S CRC remainder for a message.
 *
 * Runs the generator over every bit of the message, including the parity field.
 *
 * @param message A 14-byte array containing the raw message data.
 * @return uint32_t The 24-bit CRC remainder
 */
uint32_t remainderCRC(const std::array<uint8_t, 14>& message) {

    uint32_t remainder = 0;
    uint32_t message_byte = 0;
    uint32_t shift_amount = 0;
    uint32_t bit = 0;

    size_t message_length = getBitLength(message);
    for (size_t n = 0; n < message_length; n++) {
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
    return remainder;
}

/**
 * @brief Checks whether a message is valid for its Downlink Format.
 *
 * - DF 17/18: the remainder must be 0
 * - DF 11: the top 17 bits of the remainder must be 0.
 * - DF 0, 4, 5, 16, 20, 21, 24+: the remainder is the aircraft address AND has been seen in known_icaos.
 * - Other DFs: rejected
 *
 * @param message A 14-byte array containing the raw message data.
 * @param known_icaos Set of confirmed aircraft addresses, updated when DF 11, 17, or 18 message is confirmed.
 * @return bool true if the message passes the check for its format, false otherwise
 */
bool checkCRC(const std::array<uint8_t, 14>& message, std::unordered_set<uint32_t>& known_icaos) {
    uint8_t df = getDF(message);
    uint32_t remainder = remainderCRC(message);
    uint32_t icao = (message[1] << 16) | (message[2] << 8) | message[3];

    if (df == 17 || df == 18) {
        if (remainder == 0) {
            known_icaos.insert(icao);
            return true;
        }
        else {
            return false;
        }
    }

    if (df == 11) {
        // the top 17 bits of the remainder need to be 0
        if ((remainder & 0xFFFF80) != 0) {
            return false;
        }
        known_icaos.insert(icao);
        return true;
    }

    // The rest of the used DF values
    if (df == 0 || df == 4 || df == 5 || df == 16 || df == 20 || df == 21 || df >= 24) {
        return known_icaos.contains(remainder);
    }
    return false;
}