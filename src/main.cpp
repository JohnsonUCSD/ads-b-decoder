#include "file_reader.h"
#include "signal_processing.h"

#include <iostream>
#include <fstream>
#include <vector>

#include <cstdio>

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

    std::ofstream out("preambles.txt");
    for (double p : preambles) {
        out << p << "\n";
    }
    std::cout << "Found " << preambles.size() << " candidates\n";

    std::vector<uint8_t> packet = demodulateMessage(magnitudes, 43474);
for (uint8_t byte : packet) {
    std::printf("%02X", byte);
}
    std::cout << "\n";


    std::cout << "done\n";
    return 0;
}