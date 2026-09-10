#include "BatteryManagementSystem.hpp"
#include <iostream>
#include <algorithm>
#include <iomanip>

// ---------------------- BatteryCell ----------------------

const std::vector<std::pair<double, double>> BatteryCell::socLookupTable_ = {
    {3.00, 0.0},
    {3.30, 10.0},
    {3.50, 25.0},
    {3.70, 50.0},
    {3.90, 75.0},
    {4.10, 90.0},
    {4.20, 100.0}
};

BatteryCell::BatteryCell(int id, double minSafeVoltage, double maxSafeVoltage, double maxSafeTempC)
    : id_(id), voltage_(0.0), temperatureC_(0.0), stateOfCharge_(0.0),
      minSafeVoltage_(minSafeVoltage), maxSafeVoltage_(maxSafeVoltage),
      maxSafeTempC_(maxSafeTempC), fault_(BmsFault::NONE) {}

double BatteryCell::estimateStateOfCharge(double voltage) const {
    const auto& lut = socLookupTable_;

    if (voltage <= lut.front().first) return lut.front().second;
    if (voltage >= lut.back().first) return lut.back().second;

    for (size_t i = 0; i + 1 < lut.size(); ++i) {
        double v0 = lut[i].first, v1 = lut[i + 1].first;
        if (voltage >= v0 && voltage <= v1) {
            double soc0 = lut[i].second, soc1 = lut[i + 1].second;
            double t = (voltage - v0) / (v1 - v0); // linear interpolation
            return soc0 + t * (soc1 - soc0);
        }
    }
    return 0.0;
}

void BatteryCell::runFaultDiagnostics() {
    if (voltage_ > maxSafeVoltage_) {
        fault_ = BmsFault::OVERVOLTAGE;
    } else if (voltage_ < minSafeVoltage_) {
        fault_ = BmsFault::UNDERVOLTAGE;
    } else if (temperatureC_ > maxSafeTempC_) {
        fault_ = BmsFault::OVERTEMPERATURE;
    } else {
        fault_ = BmsFault::NONE;
    }
}

void BatteryCell::updateReadings(double voltage, double temperatureC) {
    voltage_ = voltage;
    temperatureC_ = temperatureC;
    stateOfCharge_ = estimateStateOfCharge(voltage_);
    runFaultDiagnostics();
}

// ---------------------- BatteryManagementSystem ----------------------

BatteryManagementSystem::BatteryManagementSystem(double minSafeVoltage, double maxSafeVoltage, double maxSafeTempC)
    : voltageSensor_("PackVoltageSensor"), tempSensor_("PackTempSensor"),
      minSafeVoltage_(minSafeVoltage), maxSafeVoltage_(maxSafeVoltage), maxSafeTempC_(maxSafeTempC) {}

void BatteryManagementSystem::addCell(int id) {
    cells_.emplace_back(id, minSafeVoltage_, maxSafeVoltage_, maxSafeTempC_);
}

void BatteryManagementSystem::updateCell(int id, double rawVoltageMv, double rawTempAdc) {
    double calibratedVoltage = voltageSensor_.read(rawVoltageMv);
    double calibratedTemp = tempSensor_.read(rawTempAdc);

    for (auto& cell : cells_) {
        if (cell.getId() == id) {
            cell.updateReadings(calibratedVoltage, calibratedTemp);
            return;
        }
    }
}

double BatteryManagementSystem::getPackStateOfCharge() const {
    if (cells_.empty()) return 0.0;
    double total = 0.0;
    for (const auto& cell : cells_) total += cell.getStateOfCharge();
    return total / static_cast<double>(cells_.size());
}

// Simulated PWM charging duty cycle: tapers as the pack approaches
// full charge, and drops to 0% immediately if any cell has a fault
// (mirrors the real-time safety cutoff behavior of the embedded C BMS).
double BatteryManagementSystem::computeChargingDutyCycle() const {
    if (hasFault()) return 0.0;

    double soc = getPackStateOfCharge();
    if (soc >= 100.0) return 0.0;
    if (soc >= 90.0) return 20.0;   // trickle-charge taper near full
    if (soc >= 75.0) return 50.0;
    return 100.0;                  // full duty cycle for bulk charging
}

bool BatteryManagementSystem::hasFault() const {
    return std::any_of(cells_.begin(), cells_.end(),
                        [](const BatteryCell& c) { return c.getFault() != BmsFault::NONE; });
}

void BatteryManagementSystem::printStatus() const {
    static const std::map<BmsFault, std::string> faultNames = {
        {BmsFault::NONE, "OK"},
        {BmsFault::OVERVOLTAGE, "OVERVOLTAGE"},
        {BmsFault::UNDERVOLTAGE, "UNDERVOLTAGE"},
        {BmsFault::OVERTEMPERATURE, "OVERTEMPERATURE"}
    };

    std::cout << "\n--- Battery Management System Status ---\n";
    for (const auto& cell : cells_) {
        std::cout << "  Cell " << cell.getId()
                   << " | V=" << std::fixed << std::setprecision(2) << cell.getVoltage() << "V"
                   << " | T=" << cell.getTemperature() << "C"
                   << " | SOC=" << cell.getStateOfCharge() << "%"
                   << " | Fault=" << faultNames.at(cell.getFault()) << "\n";
    }
    std::cout << "  Pack SOC: " << getPackStateOfCharge() << "%\n";
    std::cout << "  Charging duty cycle (PWM): " << computeChargingDutyCycle() << "%\n";
    std::cout << "  Pack fault state: " << (hasFault() ? "FAULT PRESENT" : "NOMINAL") << "\n";
}
