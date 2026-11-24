#include "signal_processing.h"
#include "bits.h"
#include <cmath>

#include "decode.h"

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


static bool looksLikePreamble(const std::vector<double>& magnitudes, size_t idx) {

    // Gotten from dump1090
    if (!(magnitudes[idx]   > magnitudes[idx+1] &&
          magnitudes[idx+1] < magnitudes[idx+2] &&
          magnitudes[idx+2] > magnitudes[idx+3] &&
          magnitudes[idx+3] < magnitudes[idx]   &&
          magnitudes[idx+4] < magnitudes[idx]   &&
          magnitudes[idx+5] < magnitudes[idx]   &&
          magnitudes[idx+6] < magnitudes[idx]   &&
          magnitudes[idx+7] > magnitudes[idx+8] &&
          magnitudes[idx+8] < magnitudes[idx+9] &&
          magnitudes[idx+9] > magnitudes[idx+6])) {
        return false;
    }

    // Non-pulses must sit below this threshold
    double high = (magnitudes[idx] + magnitudes[idx+2] + magnitudes[idx+7] + magnitudes[idx+9]) / 6;

    // middle of the gap between the two pulse pairs
    if (magnitudes[idx+4] >= high || magnitudes[idx+5] >= high) {
        return false;
    }

    // quiet parts just before the data bits start
    if (magnitudes[idx+11] >= high || magnitudes[idx+12] >= high || magnitudes[idx+13] >= high || magnitudes[idx+14] >= high) {
        return false;
    }

    return true;
}

std::vector<size_t> detectPreambleShape(const std::vector<double>& magnitudes) {
    std::vector<size_t> preambles;

    for (size_t idx = 0; idx + 240 <= magnitudes.size(); idx++) {
        if (looksLikePreamble(magnitudes, idx)) {
            preambles.push_back(idx);
        }
    }

    return preambles;
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
        scores[i] = (pulse_sum*0.75) - (non_pulse_sum*0.25);
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
        }
    }

    return preambles;
}

std::array<uint8_t, 14> demodulateMessage(const std::vector<double>& magnitudes, size_t preamble_idx) {
    std::array<uint8_t, 14> packet{};

    size_t start_idx = preamble_idx + PREAMBLE_SAMPLES_LENGTH;

    // Checking first bit to see if DF < 16 (short, 56 bit message), or DF >= 16 (long, 224 bit message)
    bool is_long = magnitudes[start_idx] > magnitudes[start_idx + 1];
    size_t n_bits = is_long ? LONG_MSG_BIT_LENGTH : SHORT_MSG_BIT_LENGTH;
    size_t end_idx = start_idx + SAMPLES_PER_BIT * n_bits; 

    uint8_t current_byte = 0;
    uint8_t bits_in_byte = 0;

    for (size_t i = start_idx; i < end_idx; i+=SAMPLES_PER_BIT) {

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


std::vector<double> applyPhaseCorrection(const std::vector<double>& magnitudes, size_t preamble_idx, const std::array<uint8_t, 14>& packet) {

    double on_time = magnitudes[preamble_idx] + magnitudes[preamble_idx+2] + magnitudes[preamble_idx+7] + magnitudes[preamble_idx+9];

    size_t early_idx = (preamble_idx > 0) ? preamble_idx - 1 : 0;    // incase preamble_idx is 0
    double early = 2 * (magnitudes[early_idx] + magnitudes[preamble_idx+6]);

    double late = 2 * (magnitudes[preamble_idx+3] + magnitudes[preamble_idx+10]);

    const double UNITY = 16384.0;

    double alpha = 0.0;
    double scale_up;
    double scale_down;
    bool leak_direction;    // 0 = backwards, 1 = forwards
    if (early > late) {
        if ((early + on_time) > 0) {
            alpha = early / (early + on_time);
        }
        leak_direction = 0;
    }
    else {
        if ((late + on_time) > 0) {
            alpha = late / (late + on_time);
        }
        leak_direction = 1;
    }
    scale_up = UNITY + UNITY * alpha;
    scale_down = UNITY - UNITY * alpha;

    // extracting the magnitude values for this message
    size_t data_start = preamble_idx + 16;
    size_t n_data_samples = 112 * 2;
    std::vector<double> corrected(magnitudes.begin() + data_start, magnitudes.begin() + data_start + n_data_samples);

    for (size_t bit_idx = 0; bit_idx < 112; bit_idx++) {
        for (size_t half = 0; half < 2; half++) {
            size_t sample_idx = bit_idx * 2 + half;
            if (sample_idx >= corrected.size()) {
                break;
            }

            // Sampling is late, so energy leaks backwards
            if (leak_direction == 0) {
                if (half == 1) {
                    if ((bit_idx < (112 - 1)) && (getBits(packet, bit_idx + 1, 1) == 1)) {
                        corrected[sample_idx] = corrected[sample_idx] * scale_down / UNITY;
                    }
                    else {
                        corrected[sample_idx] = corrected[sample_idx] * scale_up / UNITY;
                    }
                }
            }
            // Sampling is early, so energy leaks forward
            else {
                if (half == 0) {
                    if ((bit_idx > 0) && (getBits(packet, bit_idx - 1, 1) == 0)) {
                        corrected[sample_idx] = corrected[sample_idx] * scale_down / UNITY;
                    }
                    else {
                        corrected[sample_idx] = corrected[sample_idx] * scale_up / UNITY;
                    }
                }
            }
        }
    }

    return corrected;
}

std::array<uint8_t, 14> demodulatePhaseCorrected(const std::vector<double>& magnitudes) {
    std::array<uint8_t, 14> packet{};

    uint8_t current_byte = 0;
    uint8_t bits_in_byte = 0;

    for (size_t i = 0; i < 224; i += 2) {

        packet[current_byte] = packet[current_byte] << 1;
        packet[current_byte] += magnitudes[i] > magnitudes[i+1];

        bits_in_byte += 1;

        if (bits_in_byte == 8) {
            current_byte += 1;
            bits_in_byte = 0;
        }
    }

    return packet;
}