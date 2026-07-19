#ifndef TELEMETRY_PROCESSOR_HPP
#define TELEMETRY_PROCESSOR_HPP

#include "CircularBuffer.hpp"
#include <thread>
#include <atomic>
#include <chrono>

class TelemetryProcessor {
private:
    CircularBuffer<double, 500>& m_buffer;
    std::thread m_workerThread;
    std::atomic<bool> m_running{false};

    // Tracking algorithm state variables variables
    std::chrono::steady_clock::time_point m_lastPeakTime;
    bool m_peakDetected = false;
    double m_peakThreshold = 1.5; // Voltage threshold to clear an R-peak

    void run();
    void triggerInterruptAlarm(const char* alertMessage);

public:
    TelemetryProcessor(CircularBuffer<double, 500>& buffer);
    ~TelemetryProcessor();

    void start();
    void stop();
};

#endif // TELEMETRY_PROCESSOR_HPP