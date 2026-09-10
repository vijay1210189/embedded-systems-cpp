#ifndef SEPSIS_RISK_MONITOR_HPP
#define SEPSIS_RISK_MONITOR_HPP

#include <deque>
#include <string>
#include "Sensor.hpp"

// Finite State Machine states for sepsis risk classification —
// mirrors the NORMAL / MODERATE / CRITICAL states used in the
// embedded C / ESP32 Sepsis Risk Detection project.
enum class RiskState {
    NORMAL,
    MODERATE,
    CRITICAL
};

// A single vital-sign reading snapshot.
struct VitalSample {
    double temperatureC;
    double pulseBpm;
    double respirationBpm;
};

// Re-expresses the trend-based (3-of-5 rule) vital-sign analysis and
// Finite State Machine from the embedded C Sepsis Risk Detection
// project as a C++ class: a sliding window of the last five samples
// is evaluated, and a state transition only occurs once at least
// three of the five most recent samples agree it is warranted. This
// suppresses single-sample sensor noise from causing false alarms.
class SepsisRiskMonitor {
public:
    explicit SepsisRiskMonitor(size_t windowSize = 5, size_t voteThreshold = 3);

    // Feeds one new raw sensor reading through calibration and the
    // trend-based FSM, returning the resulting (possibly unchanged) state.
    RiskState update(double rawTempAdc, double rawPulseIntervalMs, double rawRespIntervalMs);

    RiskState getCurrentState() const { return currentState_; }
    static std::string stateToString(RiskState state);

private:
    bool sampleIndicatesCritical(const VitalSample& s) const;
    bool sampleIndicatesModerate(const VitalSample& s) const;

    TemperatureSensor tempSensor_;
    PulseSensor pulseSensor_;
    RespirationSensor respSensor_;

    std::deque<VitalSample> window_;
    size_t windowSize_;
    size_t voteThreshold_;
    RiskState currentState_;
};

#endif // SEPSIS_RISK_MONITOR_HPP
