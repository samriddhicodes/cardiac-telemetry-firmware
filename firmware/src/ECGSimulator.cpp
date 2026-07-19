#define _USE_MATH_DEFINES // <-- Add this at the very top line
#include <cmath>          // Make sure this is included right after
#include "../include/ECGSimulator.hpp"
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
            // 1. Core baseline rhythm adjustments based on heart state
            double frequency = (currentState == HeartState::Tachycardia) ? 2.5 : 1.2;
            voltage = 0.2 * std::sin(2 * M_PI * frequency * time);

            // 2. Adjust peak intervals
            double interval = (currentState == HeartState::Tachycardia) ? 0.35 : 0.8; 
            
            // 3. Generate an ultra-sharp QRS/R-Peak spike (Narrowed window to 6ms)
            double heartbeatTimer = std::fmod(time, interval);
            if (heartbeatTimer < 0.006) { 
                // Deliver a massive, narrow voltage spike
                voltage += (currentState == HeartState::Tachycardia) ? 2.5 : 2.0; 
            }
        } else {
            // Asystole: Complete flatline with tiny baseline noise
            voltage = 0.01 * std::sin(2 * M_PI * 10 * time); 
        }

        m_buffer.push(voltage);

        // Advance simulated time: 500Hz sampling rate
        time += 0.002; 
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}