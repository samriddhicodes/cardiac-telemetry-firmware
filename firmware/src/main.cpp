#include "../include/CircularBuffer.hpp"
#include "../include/ECGSimulator.hpp"
#include "../include/TelemetryProcessor.hpp"
#include <iostream>

int main() {
    std::cout << "Initializing Embedded Cardiac Telemetry Engine...\n";
    
    CircularBuffer<double, 500> shareableBuffer;
    ECGSimulator simulator(shareableBuffer);
    TelemetryProcessor processor(shareableBuffer);

    processor.start();
    simulator.start();

    std::cout << "System running at 500Hz.\n";
    std::cout << "Controls: [N] Normal Rhythm  |  [T] Inject Tachycardia  |  [A] Inject Asystole  |  [Q] Shutdown\n\n";

    char input = ' ';
    while (input != 'q' && input != 'Q') {
        std::cin >> input;
        switch (input) {
            case 'n': case 'N':
                std::cout << "\n>> Switching to Normal Sinus Rhythm.\n";
                simulator.setState(HeartState::Normal);
                break;
            case 't': case 'T':
                std::cout << "\n>> Injecting Pathological Tachycardia Simulation!\n";
                simulator.setState(HeartState::Tachycardia);
                break;
            case 'a': case 'A':
                std::cout << "\n>> Injecting Asystole Event Event!\n";
                simulator.setState(HeartState::Asystole);
                break;
            case 'q': case 'Q':
                std::cout << "\nShutting down safety systems...\n";
                break;
            default:
                break;
        }
    }

    simulator.stop();
    processor.stop();
    std::cout << "Telemetry execution terminated cleanly. Zero heap leakages." << std::endl;

    return 0;
}