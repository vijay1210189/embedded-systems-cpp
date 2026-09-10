#include <iostream>
#include "BatteryManagementSystem.hpp"
#include "SepsisRiskMonitor.hpp"

// Demonstrates both embedded systems re-expressed in C++:
//   1. BatteryManagementSystem — LUT-based SOC estimation, fault
//      diagnostics, and simulated PWM charging control across a
//      multi-cell pack.
//   2. SepsisRiskMonitor — trend-based (3-of-5 rule) vital-sign
//      classification via a Finite State Machine.
// Both share the same Sensor class hierarchy, demonstrating
// polymorphism and code reuse across two independent embedded
// systems projects.

void runBmsDemo() {
    std::cout << "=========================================\n";
    std::cout << " Battery Management System (C++) Demo\n";
    std::cout << "=========================================\n";

    BatteryManagementSystem bms(/*minSafeVoltage=*/3.0, /*maxSafeVoltage=*/4.2, /*maxSafeTempC=*/45.0);
    bms.addCell(1);
    bms.addCell(2);
    bms.addCell(3);

    // Simulated raw ADC readings over three time steps (mV, raw temp ADC).
    struct Reading { int cellId; double rawMv; double rawTempAdc; };
    std::vector<std::vector<Reading>> timeline = {
        { {1, 3200, 250}, {2, 3250, 260}, {3, 3180, 255} },   // charging, nominal
        { {1, 3900, 300}, {2, 3950, 310}, {3, 3880, 305} },   // approaching full
        { {1, 4300, 480}, {2, 4100, 320}, {3, 4050, 300} },   // cell 1 overvoltage + overtemp fault
    };

    for (size_t t = 0; t < timeline.size(); ++t) {
        std::cout << "\n[Time step " << t << "]";
        for (const auto& r : timeline[t]) {
            bms.updateCell(r.cellId, r.rawMv, r.rawTempAdc);
        }
        bms.printStatus();
    }
}

void runSepsisDemo() {
    std::cout << "\n=========================================\n";
    std::cout << " Sepsis Risk Monitor (C++) Demo\n";
    std::cout << "=========================================\n";

    SepsisRiskMonitor monitor(/*windowSize=*/5, /*voteThreshold=*/3);

    // Simulated raw sensor readings: (tempAdc, pulseIntervalMs, respIntervalMs)
    // Sequence drifts from normal -> moderate -> critical vitals.
    struct Reading { double tempAdc; double pulseMs; double respMs; };
    std::vector<Reading> stream = {
        {370, 800, 3500},  // normal
        {372, 780, 3400},  // normal
        {375, 700, 3000},  // borderline
        {380, 550, 2200},  // moderate trending
        {382, 500, 2000},  // moderate trending
        {392, 440, 1900},  // critical trending
        {395, 400, 1800},  // critical trending
        {398, 380, 1700},  // critical trending
    };

    for (size_t i = 0; i < stream.size(); ++i) {
        RiskState state = monitor.update(stream[i].tempAdc, stream[i].pulseMs, stream[i].respMs);
        std::cout << "  Sample " << i << " -> State: "
                  << SepsisRiskMonitor::stateToString(state) << "\n";
    }
}

int main() {
    runBmsDemo();
    runSepsisDemo();
    return 0;
}
