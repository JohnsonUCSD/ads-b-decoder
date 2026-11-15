#include "file_reader.h"
#include "signal_processing.h"
#include "crc.h"
#include "decode.h"

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

    std::vector<size_t> preambles = detectPreamble(magnitudes);

    std::array<uint8_t, 14> message;
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

    while (g_running) {
        int n_read = 0;
        if (rtlsdr_read_sync(dev, buf.data(), static_cast<int>(buf.size()), &n_read) != 0) {
            break;
        }

        std::vector<uint8_t> raw(buf.begin(), buf.begin() + n_read);
        std::vector<double> magnitudes = computeMagnitudes(raw);
        std::vector<size_t> preambles = detectPreamble(magnitudes);


        std::array<uint8_t, 14> message;
        for (size_t p : preambles) {
            message = demodulateMessage(magnitudes, p);
            if (checkCRC(message)) {
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

// int main() {
//     std::vector<uint8_t> raw = readRawFile("tests/modes1.bin");
//     std::cout << "Read " << raw.size() << " bytes\n";

//     std::vector<double> magnitudes = computeMagnitudes(raw);
//     std::cout << "Computed " << magnitudes.size() << " magnitude samples\n";

//     // std::ofstream out("magnitudes.txt");
//     // for (double m : magnitudes) {
//     //     out << m << "\n";
//     // }

//     std::vector<size_t> preambles = detectPreamble(magnitudes);

//     // std::ofstream out("preambles.txt");
//     // for (size_t p : preambles) {
//     //     out << p << "\n";
//     // }
//     std::cout << "Found " << preambles.size() << " candidates\n";

//     std::array<uint8_t, 14> packet = demodulateMessage(magnitudes, 43474);
//     for (uint8_t byte : packet) {
//         std::printf("%02X", byte);
//     }
//     std::cout << "\n";

//     std::cout << "CRC Check on 43474: " << checkCRC(packet) << std::endl;


//     std::array<uint8_t, 14> message;
//     size_t crc_total = 0;
//     std::ofstream outs("crc_checked.txt");
//     for (size_t p : preambles) {
//         message = demodulateMessage(magnitudes, p);
//         if (checkCRC(message)) {

//             outs << "Packet at " << p << ":\t";
//             for (uint8_t byte : message) {
//                 outs << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(byte);
//             }
//             outs << std::dec << "\n";
//             crc_total++;
//         }
//     }
//     // std::cout << crc_total << "CRC Pass, " << preambles.size() - crc_total <<" CRC Fail out of" <<  <<"\n";
//     std::printf("%zu CRC Pass, %zu CRC Fail out of %zu candidates\n", crc_total, preambles.size() - crc_total, preambles.size());


//     std::array<uint8_t, 14> packet_amc = demodulateMessage(magnitudes, 120675);

//     std::cout << checkCRC(packet_amc) << std::endl;
//     AdsbMessage AMC421 = decode(packet_amc);

//     std::cout << std::get<Identification>(AMC421.payload).callsign << std::endl;

//     std::cout << "done\n";
    
//     return 0;
// }