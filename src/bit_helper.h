#pragma once
#include <array>
#include <cstddef>
#include <cstdint>


inline uint32_t getBits(const std::array<uint8_t, 14>& packet, size_t start, size_t length) {
    uint32_t value = 0;

    for (size_t i = start; i < start + length; i++) {
        value = (value << 1) | ((packet[i / 8] >> (7 - i % 8)) & 0x1);
    }

    return value;
}

/**
 * @brief Extracts the Downlink Format (DF) from a Mode-S / ADS-B packet.
 *
 * Reads the upper 5 bits of the first byte in the packet buffer.
 *
 * @param packet A 14-byte array containing the raw packet data.
 * @return uint8_t The 5-bit Downlink Format value (0–31).
 */
inline uint8_t getDF(const std::array<uint8_t, 14>& packet) {
    return packet[0] >> 3;
}

/**
 * @brief Determines bit length of a Mode-S / ADS-B packet.
 *
 * Looks at MSB of first byte, which corresponds to the Downlink Format (DF). 
 * A value of 1 indicates DF 16-31 (112-bit long message), while 0 indicates DF 0-15 (56-bit short message).
 *
 * @param packet A 14-byte array containing the raw packet data.
 * @return size_t The message length in bits: 112 for extended messages or 56 for short messages.
 */
inline size_t getBitLength(const std::array<uint8_t, 14>& packet) {
    return (packet[0] >> 7) ? 112 : 56;
}