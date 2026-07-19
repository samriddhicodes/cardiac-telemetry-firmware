#define _USE_MATH_DEFINES // <-- Add this at the very top line
#include <cmath>          // Make sure this is included right after
#include "ECGSimulator.hpp"
#include <chrono>

ECGSimulator::ECGSimulator(CircularBuffer<double, 500>& buffer) : m_buffer(buffer) {}

ECGSimulator::~ECGSimulator() {
    stop();
}

void ECGSimulator::start() {
    m_running = true;
    m_workerThread = std::thread(&ECGSimulator::run, this);
}

void ECGSimulator::stop() {
    if (m_running) {
        m_running = false;
        if (m_workerThread.joinable()) {
            m_workerThread.join();
        }
    }
}

void ECGSimulator::setState(HeartState state) {
    m_state = state;
}

void ECGSimulator::run() {
    double time = 0.0;
    
    while (m_running) {
        HeartState currentState = m_state.load();
        double voltage = 0.0;

        if (currentState != HeartState::Asystole) {
            // 1. Simulate a basic baseline heart rhythm (P and T waves using sine waves)
            voltage = 0.2 * std::sin(2 * M_PI * 1.2 * time);

            // 2. Adjust heart rate interval based on condition state
            double interval = (currentState == HeartState::Tachycardia) ? 0.3 : 0.8; // 0.3s (~200 BPM) vs 0.8s (~75 BPM)
            
            // 3. Generate a sharp QRS/R-Peak
            double heartbeatTimer = std::fmod(time, interval);
            if (heartbeatTimer < 0.04) { 
                voltage += 2.0; // The R-Peak spike
            }
        } else {
            // Asystole: Flatline with micro-voltage noise
            voltage = 0.01 * std::sin(2 * M_PI * 50 * time); 
        }

        m_buffer.push(voltage);

        // Advance simulated time: 500Hz sampling rate means 2 milliseconds per sample
        time += 0.002; 
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}