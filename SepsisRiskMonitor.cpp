#include "SepsisRiskMonitor.hpp"
#include <algorithm>

SepsisRiskMonitor::SepsisRiskMonitor(size_t windowSize, size_t voteThreshold)
    : tempSensor_("VitalTempSensor"), pulseSensor_("VitalPulseSensor"),
      respSensor_("VitalRespSensor"), windowSize_(windowSize),
      voteThreshold_(voteThreshold), currentState_(RiskState::NORMAL) {}

// Clinically simplified thresholds for demonstration purposes.
bool SepsisRiskMonitor::sampleIndicatesCritical(const VitalSample& s) const {
    return (s.temperatureC >= 39.0 || s.temperatureC <= 35.0) &&
           (s.pulseBpm >= 130.0) &&
           (s.respirationBpm >= 30.0);
}

bool SepsisRiskMonitor::sampleIndicatesModerate(const VitalSample& s) const {
    return (s.temperatureC >= 38.0 || s.temperatureC <= 36.0) ||
           (s.pulseBpm >= 100.0) ||
           (s.respirationBpm >= 22.0);
}

RiskState SepsisRiskMonitor::update(double rawTempAdc, double rawPulseIntervalMs, double rawRespIntervalMs) {
    VitalSample sample{
        tempSensor_.read(rawTempAdc),
        pulseSensor_.read(rawPulseIntervalMs),
        respSensor_.read(rawRespIntervalMs)
    };

    window_.push_back(sample);
    if (window_.size() > windowSize_) {
        window_.pop_front();
    }

    // Only apply the 3-of-5 vote once the window is full — avoids
    // premature state changes during system warm-up.
    if (window_.size() < windowSize_) {
        return currentState_;
    }

    size_t criticalVotes = std::count_if(window_.begin(), window_.end(),
        [this](const VitalSample& s) { return sampleIndicatesCritical(s); });
    size_t moderateVotes = std::count_if(window_.begin(), window_.end(),
        [this](const VitalSample& s) { return sampleIndicatesModerate(s); });

    if (criticalVotes >= voteThreshold_) {
        currentState_ = RiskState::CRITICAL;
    } else if (moderateVotes >= voteThreshold_) {
        currentState_ = RiskState::MODERATE;
    } else {
        currentState_ = RiskState::NORMAL;
    }

    return currentState_;
}

std::string SepsisRiskMonitor::stateToString(RiskState state) {
    switch (state) {
        case RiskState::NORMAL:   return "NORMAL";
        case RiskState::MODERATE: return "MODERATE";
        case RiskState::CRITICAL: return "CRITICAL";
    }
    return "UNKNOWN";
}
