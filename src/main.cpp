#include "file_reader.h"
#include "signal_processing.h"

#include <iostream>
#include <fstream>
#include <vector>

int main() {
    std::vector<uint8_t> raw = readRawFile("tests/modes1.bin");
    std::cout << "Read " << raw.size() << " bytes\n";

    std::vector<double> magnitudes = computeMagnitudes(raw);
    std::cout << "Computed " << magnitudes.size() << " magnitude samples\n";

    std::ofstream out("magnitudes.txt");
    for (double m : magnitudes) {
        out << m << "\n";
    }

    std::cout << "Wrote magnitudes.txt\n";
    return 0;
}