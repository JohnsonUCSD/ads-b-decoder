#include "signal_processing.h"
#include <cmath>

std::vector<double> computeMagnitudes(const std::vector<uint8_t>& raw) {
    std::vector<double> magnitudes;
    magnitudes.reserve(raw.size() / 2);

    for (size_t i = 0; i + 1 < raw.size(); i += 2) {
        double i_data = raw[i] - 127.5;
        double q_data = raw[i + 1] - 127.5;
        magnitudes.push_back(std::sqrt((i_data * i_data) + (q_data * q_data)));
    }

    return magnitudes;
}