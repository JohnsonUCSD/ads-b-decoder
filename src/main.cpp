#include "file_reader.h"
#include "signal_processing.h"
#include "crc.h"
#include "decode.h"
#include "bits.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <array>

#include <cstdio>
#include <iomanip>

#include <csignal>
#include <algorithm>

#include <rtl-sdr.h>

volatile std::sig_atomic_t g_running = 1;
void onSigint(int) { g_running = 0; }

int runFile(const std::string& path) {
    std::vector<uint8_t> raw = readRawFile(path);
    std::vector<double> magnitudes = computeMagnitudes(raw);

    // std::vector<size_t> preambles = detectPreamble(magnitudes);
    std::vector<size_t> preambles = detectPreambleShape(magnitudes);

    std::array<uint8_t, 14> message;
    std::unordered_set<uint32_t> known_icaos;
    size_t crc_total = 0;
    std::ofstream outs("crc_checked.txt");
    for (size_t p : preambles) {
        message = demodulateMessage(magnitudes, p);
        if (checkCRC(message, known_icaos)) {

            outs << "Packet at " << p << ":\t";
            for (size_t i = 0; i < getBitLength(message) / 8; i++) {
                outs << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(message[i]);
            }
            outs << std::dec << "\n";
            crc_total++;
        }
        else {
            std::vector<double> corrected1 = applyPhaseCorrection_Gentile(magnitudes, p, message);
            std::vector<double> corrected2 = applyPhaseCorrection_dump(magnitudes, p, message);

            std::array<uint8_t, 14> corrected_msg1 = demodulatePhaseCorrected(corrected1);
            std::array<uint8_t, 14> corrected_msg2 = demodulatePhaseCorrected(corrected2);

            if (checkCRC(corrected_msg1, known_icaos)) {
                std::cout << "Gentile Packet at " << p << ":\t";
                for (size_t i = 0; i < getBitLength(corrected_msg1) / 8; i++) {
                    std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(corrected_msg1[i]);
                }
                std::cout << std::dec << "\n";

            }
            if (checkCRC(corrected_msg2, known_icaos)) {
                std::cout << "Dump Packet at " << p << ":\t";
                for (size_t i = 0; i < getBitLength(corrected_msg2) / 8; i++) {
                    std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(corrected_msg2[i]);
                }
                std::cout << std::dec << "\n";

            }

        }
    }
    std::cout << crc_total << "/" << preambles.size() << std::endl;

    std::cout << "Known ICAOs (" << known_icaos.size() << "):\n";
    for (uint32_t icao : known_icaos) {
        std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << icao << "\n";
    }
    std::cout << std::dec << std::nouppercase;


    return 0;
}


int runLive(){
    std::signal(SIGINT, onSigint);

    std::cerr << "opening...\n";
    rtlsdr_dev_t* dev = nullptr;
    if (rtlsdr_open(&dev, 0) != 0) {
        std::cerr << "Could not open RTL-SDR device\n";
        return 1;
    }

    std::cerr << "opened, configuring...\n";
    rtlsdr_set_center_freq(dev, 1090000000);    // 1090 Mhz frequency
    rtlsdr_set_sample_rate(dev, 2000000);       // 2 Mhz sample rate
    rtlsdr_set_tuner_gain_mode(dev, 1);         // manual gain
    rtlsdr_set_tuner_gain(dev, 402);            // 40.2 dB
    rtlsdr_reset_buffer(dev);

    std::vector<uint8_t> buf(262144);        

    std::cerr << "waiting for first read...\n";

    std::unordered_set<uint32_t> known_icaos;

    while (g_running) {
        int n_read = 0;
        if (rtlsdr_read_sync(dev, buf.data(), static_cast<int>(buf.size()), &n_read) != 0) {
            break;
        }

        std::vector<uint8_t> raw(buf.begin(), buf.begin() + n_read);
        std::vector<double> magnitudes = computeMagnitudes(raw);
        std::vector<size_t> preambles = detectPreamble(magnitudes);



        // TODO: ADD PHASE CORRECTION 
        std::array<uint8_t, 14> message;
        
        for (size_t p : preambles) {
            message = demodulateMessage(magnitudes, p);
            if (checkCRC(message, known_icaos)) {
                for (uint8_t byte : message) {
                    std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(byte);
                }
                std::cout << std::dec << std::endl;
            }
        }
    }

    rtlsdr_close(dev);

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file.bin> | --live\n";
        return 1;
    }

    std::string arg = argv[1];

    if (arg == "--live") {
        return runLive();
    }
    return runFile(arg);
}