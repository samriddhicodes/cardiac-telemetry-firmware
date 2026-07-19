#include "TelemetryProcessor.hpp"
#include <iostream>

TelemetryProcessor::TelemetryProcessor(CircularBuffer<double, 500>& buffer) 
    : m_buffer(buffer), m_lastPeakTime(std::chrono::steady_clock::now()) {}

TelemetryProcessor::~TelemetryProcessor() {
    stop();
}

void TelemetryProcessor::start() {
    m_running = true;
    m_workerThread = std::thread(&TelemetryProcessor::run, this);
}

void TelemetryProcessor::stop() {
    if (m_running) {
        m_running = false;
        if (m_workerThread.joinable()) {
            m_workerThread.join();
        }
    }
}

// Simulates a Hardware Interrupt / High Priority Task Execution
void TelemetryProcessor::triggerInterruptAlarm(const char* alertMessage) {
    std::cerr << "\n==================================================\n";
    std::cerr << " [HARDWARE INTERRUPT] CRITICAL ALARM TRIGGERED\n";
    std::cerr << " CONDITION: " << alertMessage << "\n";
    std::cerr << "==================================================\n\n";
}

void TelemetryProcessor::run() {
    while (m_running) {
        auto dataPoint = m_buffer.pop();

        if (dataPoint.has_value()) {
            double voltage = dataPoint.value();
            auto now = std::chrono::steady_clock::now();
            auto timeSinceLastPeak = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastPeakTime).count();

            // 1. Asystole Check: If no peak is registered within 2.5 seconds
            if (timeSinceLastPeak > 2500) {
                triggerInterruptAlarm("ASYSTOLE (CARDIAC ARREST DETECTED)");
                // Reset clock so we don't spam the stdout continuously every millisecond
                m_lastPeakTime = now; 
            }

            // 2. R-Peak Threshold Detection Engine
            if (voltage > m_peakThreshold && !m_peakDetected) {
                m_peakDetected = true; // Debounce to prevent multiple counts on a single broad peak

                double bpm = 60000.0 / timeSinceLastPeak;
                
                // Print standard real-time telemetry log
                std::cout << "[Telemetry Data] Sample: " << voltage << " V | Calculated HR: " << static_cast<int>(bpm) << " BPM" << std::endl;

                // 3. Tachycardia Check
                if (bpm > 140.0 && timeSinceLastPeak > 50) { // check lower limit to avoid initial calculation noise
                    triggerInterruptAlarm("TACHYCARDIA (CRITICAL HIGH HEART RATE)");
                }

                m_lastPeakTime = now;
            } 
            else if (voltage < m_peakThreshold) {
                m_peakDetected = false; // Reset the debounce state when voltage returns below threshold
            }
        }

        // Run analysis loop slightly faster than producer frequency to stay cleared out
        std::this_thread::sleep_for(std::chrono::microseconds(800));
    }
}