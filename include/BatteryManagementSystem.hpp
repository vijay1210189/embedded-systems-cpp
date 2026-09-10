#ifndef BATTERY_MANAGEMENT_SYSTEM_HPP
#define BATTERY_MANAGEMENT_SYSTEM_HPP

#include <vector>
#include <map>
#include "Sensor.hpp"

// Fault flags raised by the BMS during real-time safety monitoring.
enum class BmsFault {
    NONE,
    OVERVOLTAGE,
    UNDERVOLTAGE,
    OVERTEMPERATURE
};

// Encapsulates a single battery cell: its live readings, a
// lookup-table (LUT) based State-of-Charge estimate, and its
// current fault status. This mirrors the ADC-based voltage
// monitoring and LUT-based SOC estimation used in the embedded C
// BMS project, re-expressed with C++ classes and encapsulation.
class BatteryCell {
public:
    BatteryCell(int id, double minSafeVoltage, double maxSafeVoltage, double maxSafeTempC);

    void updateReadings(double voltage, double temperatureC);

    int getId() const { return id_; }
    double getVoltage() const { return voltage_; }
    double getTemperature() const { return temperatureC_; }
    double getStateOfCharge() const { return stateOfCharge_; }
    BmsFault getFault() const { return fault_; }

private:
    // LUT-based SOC estimation: maps a cell voltage to an
    // approximate State-of-Charge percentage via linear
    // interpolation between calibration points.
    double estimateStateOfCharge(double voltage) const;
    void runFaultDiagnostics();

    int id_;
    double voltage_;
    double temperatureC_;
    double stateOfCharge_;
    double minSafeVoltage_;
    double maxSafeVoltage_;
    double maxSafeTempC_;
    BmsFault fault_;

    // Calibration LUT: {voltage, SOC%} pairs, ascending by voltage.
    static const std::vector<std::pair<double, double>> socLookupTable_;
};

// Manages a pack of BatteryCell objects: aggregates pack-level
// State-of-Charge, runs PWM-style charging-duty-cycle control, and
// reports any fault raised by an individual cell.
class BatteryManagementSystem {
public:
    BatteryManagementSystem(double minSafeVoltage, double maxSafeVoltage, double maxSafeTempC);

    void addCell(int id);
    void updateCell(int id, double rawVoltageMv, double rawTempAdc);

    double getPackStateOfCharge() const;
    double computeChargingDutyCycle() const; // simulated PWM duty cycle (0-100%)
    bool hasFault() const;
    void printStatus() const;

private:
    std::vector<BatteryCell> cells_;
    VoltageSensor voltageSensor_;
    TemperatureSensor tempSensor_;
    double minSafeVoltage_;
    double maxSafeVoltage_;
    double maxSafeTempC_;
};

#endif // BATTERY_MANAGEMENT_SYSTEM_HPP
