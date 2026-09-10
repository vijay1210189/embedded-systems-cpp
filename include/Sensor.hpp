#ifndef SENSOR_HPP
#define SENSOR_HPP

#include <string>

// Abstract base class for all sensor types used across the embedded
// systems (BMS and vital-sign monitoring). Demonstrates polymorphism:
// every concrete sensor overrides read() to convert a raw ADC-style
// input into a calibrated physical value.
class Sensor {
public:
    Sensor(std::string name, double calibrationOffset = 0.0)
        : name_(std::move(name)), calibrationOffset_(calibrationOffset) {}

    virtual ~Sensor() = default;

    // Pure virtual: forces every derived sensor to define its own
    // conversion from raw reading to calibrated physical value.
    virtual double read(double rawValue) const = 0;

    const std::string& getName() const { return name_; }

protected:
    std::string name_;
    double calibrationOffset_;
};

// Voltage sensor for individual battery cells (ADC millivolt -> volts).
class VoltageSensor : public Sensor {
public:
    explicit VoltageSensor(std::string name, double calibrationOffset = 0.0)
        : Sensor(std::move(name), calibrationOffset) {}

    double read(double rawMillivolts) const override {
        return (rawMillivolts / 1000.0) + calibrationOffset_;
    }
};

// Temperature sensor (used by both BMS pack monitoring and the
// sepsis-risk vital-sign monitor) — simple linear ADC-to-Celsius map.
class TemperatureSensor : public Sensor {
public:
    explicit TemperatureSensor(std::string name, double calibrationOffset = 0.0)
        : Sensor(std::move(name), calibrationOffset) {}

    double read(double rawAdc) const override {
        // Example linear mapping for an LM35-style sensor: 10 mV/°C
        return (rawAdc * 0.1) + calibrationOffset_;
    }
};

// Pulse sensor — converts a raw photoplethysmogram peak-interval
// reading (ms between beats) into beats-per-minute.
class PulseSensor : public Sensor {
public:
    explicit PulseSensor(std::string name, double calibrationOffset = 0.0)
        : Sensor(std::move(name), calibrationOffset) {}

    double read(double beatIntervalMs) const override {
        if (beatIntervalMs <= 0.0) return 0.0;
        return (60000.0 / beatIntervalMs) + calibrationOffset_;
    }
};

// Respiration sensor — converts a raw strain-gauge reading into
// breaths per minute.
class RespirationSensor : public Sensor {
public:
    explicit RespirationSensor(std::string name, double calibrationOffset = 0.0)
        : Sensor(std::move(name), calibrationOffset) {}

    double read(double breathIntervalMs) const override {
        if (breathIntervalMs <= 0.0) return 0.0;
        return (60000.0 / breathIntervalMs) + calibrationOffset_;
    }
};

#endif // SENSOR_HPP
