# ADS-B Decoder
An ADS-B (Mode S 1090 MHz) decoder in C++ that works from raw I/Q samples

## Current Features
- Reading raw I/Q files (2 MHz sample rate)
- Preamble detection with adaptive thresholding threshold (mean + N*stddev)
- Demodulating 112-bit messages
- CRC checking
- Decoding for:
  - Aircraft identification (TC 1-4)
  - Airborne position (TC 9-18)
  - Airborne velocity (TC 19)

## WIP
- Decoding remaining type codes
- Printing decoded messages

## Future Plans
- Phase correction for failed CRC packetss
- Short messages (56-bit) such as DF11
- Live input from RTL-SDR

## Build and Run
In Progress of Changing

## Project Layout
| File | Purpose |
|---|---|
| `file_reader` | Loads raw bytes from file |
| `signal_processing` | Magnitudes, preamble detection, demodulation |
| `crc` | CRC check |
| `decode` | Message field decoding |

## Testing
- [https://github.com/doctest/doctest](https://github.com/doctest/doctest)

## References
- [The 1090 Megahertz Riddle](https://mode-s.org/1090mhz/) (Mode S reference)
- [John Gentile](https://john-gentile.com/kb/dsp/ADS-B.html)
- [dump1090](https://github.com/antirez/dump1090)