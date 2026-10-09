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

std::vector<size_t> detectPreamble(const std::vector<double>& magnitudes) {
    std::vector<size_t> preambles;

    // compute score at every index and store them 
    std::vector<double> scores(magnitudes.size() - 234);
    double pulse_sum = 0;
    double non_pulse_sum = 0;
    for (size_t i = 0; i < magnitudes.size() - 234; i++) {
        // indices 0, 2, 7, 9 is the preamble pulse positions
        pulse_sum = magnitudes[i] + magnitudes[i+2] + magnitudes[i+7] + magnitudes[i+9];
        non_pulse_sum = magnitudes[i+1] + magnitudes[i+3] + magnitudes[i+4] + magnitudes[i+5] + magnitudes[i+6] + 
                        magnitudes[i+8] + magnitudes[i+10] + magnitudes[i+11] + magnitudes[i+12] + magnitudes[i+13] + magnitudes[i+14] + magnitudes[i+15];
        scores[i] = pulse_sum - (non_pulse_sum / 3);
    }

    // mean 
    double sum = 0;
    for (double s: scores) {
        sum += s;
    }
    double mean = sum / scores.size();

    // standard deviation
    double total = 0;
    for (double s: scores) {
        total += (s - mean) * (s - mean);
    }
    double stddev = std::sqrt(total / scores.size());

    double threshold = mean + (5 * stddev);

    for (size_t i = 0; i < scores.size(); i++) {
        if (scores[i] > threshold) {
            preambles.push_back(i);
            i += 240;
        }
    }

    return preambles;
}

std::vector<uint8_t> demodulateMessage(const std::vector<double>& magnitudes, size_t preamble_idx) {
    std::vector<uint8_t> packet(14, 0);

    uint8_t current_byte = 0;
    uint8_t bits_in_byte = 0;

    size_t start_idx = preamble_idx + 16;
    for (size_t i = start_idx; i < start_idx + 224; i+=2) {

        packet[current_byte] = packet[current_byte] << 1;
        packet[current_byte] += magnitudes[i] > magnitudes[i+1];

        bits_in_byte += 1;

        if (bits_in_byte == 8) {
            current_byte +=1;
            bits_in_byte = 0;
        }
    }
    
    return packet;
}