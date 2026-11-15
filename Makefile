CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -g
LDLIBS = -lrtlsdr

TARGET = output
TEST_TARGET = test_runner

SRC = src/main.cpp src/file_reader.cpp src/signal_processing.cpp src/crc.cpp src/decode.cpp
TEST_SRC = tests/test_runner.cpp src/crc.cpp src/decode.cpp

all: $(TARGET) $(TEST_TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

$(TEST_TARGET): $(TEST_SRC)
	$(CXX) $(CXXFLAGS) -Isrc $(TEST_SRC) -o $(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET)

.PHONY: all clean