CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude
SRCS := src/main.cpp src/BatteryManagementSystem.cpp src/SepsisRiskMonitor.cpp
TARGET := embedded_demo

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
