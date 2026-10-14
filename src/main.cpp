#include "file_reader.h"
#include "signal_processing.h"
#include "crc.h"
#include "decode.h"

#include <iostream>
#include <fstream>
#include <vector>

#include <cstdio>
#include <iomanip>

int main() {
    std::vector<uint8_t> raw = readRawFile("tests/modes1.bin");
    std::cout << "Read " << raw.size() << " bytes\n";

    std::vector<double> magnitudes = computeMagnitudes(raw);
    std::cout << "Computed " << magnitudes.size() << " magnitude samples\n";

    // std::ofstream out("magnitudes.txt");
    // for (double m : magnitudes) {
    //     out << m << "\n";
    // }

    std::vector<size_t> preambles = detectPreamble(magnitudes);

    // std::ofstream out("preambles.txt");
    // for (size_t p : preambles) {
    //     out << p << "\n";
    // }
    std::cout << "Found " << preambles.size() << " candidates\n";

    std::vector<uint8_t> packet = demodulateMessage(magnitudes, 43474);
    for (uint8_t byte : packet) {
        std::printf("%02X", byte);
    }
    std::cout << "\n";

    std::cout << "CRC Check on 43474: " << checkCRC(packet) << std::endl;


    std::vector<uint8_t> message;
    size_t crc_total = 0;
    std::ofstream outs("crc_checked.txt");
    for (size_t p : preambles) {
        message = demodulateMessage(magnitudes, p);
        if (checkCRC(message)) {

            outs << "Packet at " << p << ":\t";
            for (uint8_t byte : message) {
                outs << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(byte);
            }
            outs << std::dec << "\n";
            crc_total++;
        }
    }
    // std::cout << crc_total << "CRC Pass, " << preambles.size() - crc_total <<" CRC Fail out of" <<  <<"\n";
    std::printf("%zu CRC Pass, %zu CRC Fail out of %zu candidates\n", crc_total, preambles.size() - crc_total, preambles.size());


    std::vector<uint8_t> packet_amc = demodulateMessage(magnitudes, 120675);

    std::cout << checkCRC(packet_amc) << std::endl;
    AdsbMessage AMC421 = decode(packet_amc);

    std::cout << *AMC421.callsign << std::endl;

    std::cout << "done\n";
    
    return 0;
}