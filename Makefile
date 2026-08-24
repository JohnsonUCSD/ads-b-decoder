CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

TARGET = output

SRC = src/main.cpp

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)