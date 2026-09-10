# Embedded Systems in C++: BMS & Sepsis Risk Monitor

A C++ re-implementation of core logic from two embedded systems projects
(Automotive Battery Management System and ESP32-based Sepsis Risk Detection),
restructured around a shared, polymorphic sensor hierarchy to demonstrate
object-oriented C++ design applied to real embedded/firmware logic.

## What this demonstrates

- **Abstract base classes & polymorphism** — `Sensor` defines a pure virtual
  `read()` interface; `VoltageSensor`, `TemperatureSensor`, `PulseSensor`, and
  `RespirationSensor` each override it with their own calibration logic.
- **Encapsulation** — `BatteryCell` and `SepsisRiskMonitor` hide their
  internal state (raw readings, lookup tables, sliding windows) behind a
  clean public interface.
- **STL usage** — `std::vector`, `std::deque`, `std::map`, `std::any_of`,
  `std::count_if`, and lambda expressions.
- **Real embedded logic, not a toy example**:
  - LUT-based State-of-Charge estimation via linear interpolation
  - Real-time fault diagnostics (overvoltage / undervoltage / overtemperature)
  - Simulated PWM charging duty-cycle control that responds to pack state
  - Trend-based (3-of-5 rule) vital-sign classification via a Finite State
    Machine, suppressing single-sample sensor noise — same design as the
    ESP32 Sepsis Risk Detection project's embedded C implementation

## Structure

```
include/
  Sensor.hpp                     # abstract Sensor + 4 concrete sensor types
  BatteryManagementSystem.hpp    # BatteryCell + BatteryManagementSystem
  SepsisRiskMonitor.hpp          # RiskState FSM + SepsisRiskMonitor
src/
  BatteryManagementSystem.cpp
  SepsisRiskMonitor.cpp
  main.cpp                       # demo driver exercising both systems
Makefile
```

## Build & run

```bash
make run
```

or manually:

```bash
g++ -std=c++17 -Wall -Wextra -Iinclude src/main.cpp src/BatteryManagementSystem.cpp src/SepsisRiskMonitor.cpp -o embedded_demo
./embedded_demo
```

## Sample output

The demo simulates a 3-cell battery pack across three time steps (charging →
approaching full → a cell fault), and an 8-sample vital-sign stream drifting
from normal to critical, showing the FSM's state transitions in real time.

## Origin

This project re-expresses logic first built in Embedded C for two hardware
projects (an automotive BMS running ADC-based voltage monitoring, and an
ESP32-based sepsis risk detector using multi-sensor trend analysis), applying
C++ class design, inheritance, and the STL on top of the same underlying
embedded logic.
