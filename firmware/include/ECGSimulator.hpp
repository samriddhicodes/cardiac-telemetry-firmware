#ifndef ECG_SIMULATOR_HPP
#define ECG_SIMULATOR_HPP

#include "CircularBuffer.hpp"
#include <thread>
#include <atomic>

enum class HeartState {
    Normal,
    Tachycardia,
    Asystole
};

class ECGSimulator {
private:
    CircularBuffer<double, 500>& m_buffer;
    std::thread m_workerThread;
    std::atomic<bool> m_running{false};
    std::atomic<HeartState> m_state{HeartState::Normal};
    
    void run();

public:
    ECGSimulator(CircularBuffer<double, 500>& buffer);
    ~ECGSimulator();

    void start();
    void stop();
    void setState(HeartState state);
};

#endif // ECG_SIMULATOR_HPP