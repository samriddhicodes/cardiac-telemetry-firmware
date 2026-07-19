#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "TelemetryProcessor.hpp"
#include <iostream>
#include <winsock2.h>
#include <string>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

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

void TelemetryProcessor::triggerInterruptAlarm(const char* alertMessage) {
    std::cerr << "\n==================================================\n";
    std::cerr << " [HARDWARE INTERRUPT] CRITICAL ALARM TRIGGERED\n";
    std::cerr << " CONDITION: " << alertMessage << "\n";
    std::cerr << "==================================================\n\n";
}

void TelemetryProcessor::run() {
    // 1. Initialize Windows Sockets (Winsock)
    WSADATA wsaData;
    SOCKET connectSocket = INVALID_SOCKET;
    
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock initialization failed.\n";
        return;
    }

    // 2. Configure and establish TCP connection to Node.js server
    sockaddr_in clientService;
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr("127.0.0.1");
    clientService.sin_port = htons(5001);

    std::cout << "[Firmware] Connecting to Node.js ingestion server on 127.0.0.1:5001...\n";
    
    // Attempt connection loop
    while (m_running) {
        connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (connectSocket == INVALID_SOCKET) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        if (connect(connectSocket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
            closesocket(connectSocket);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        break; 
    }

    std::cout << "[Firmware] Network pipe established with backend gateway.\n";

    while (m_running) {
        auto dataPoint = m_buffer.pop();

        if (dataPoint.has_value()) {
            double voltage = dataPoint.value();
            auto now = std::chrono::steady_clock::now();
            auto timeSinceLastPeak = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastPeakTime).count();
            
            std::string alarmState = "NONE";
            int bpmCalculated = 0;

            // Asystole detection
            if (timeSinceLastPeak > 2500) {
                triggerInterruptAlarm("ASYSTOLE (CARDIAC ARREST DETECTED)");
                alarmState = "ASYSTOLE";
                m_lastPeakTime = now; 
            }

            // R-Peak detection
            if (voltage > m_peakThreshold && !m_peakDetected) {
                m_peakDetected = true;
                double bpm = 60000.0 / timeSinceLastPeak;
                bpmCalculated = static_cast<int>(bpm);

                if (bpm > 140.0 && timeSinceLastPeak > 50) {
                    triggerInterruptAlarm("TACHYCARDIA (CRITICAL HIGH HEART RATE)");
                    alarmState = "TACHYCARDIAC";
                }
                m_lastPeakTime = now;
            } 
            else if (voltage < m_peakThreshold) {
                m_peakDetected = false;
            }

            // 3. Serialize to JSON payload
            std::ostringstream jsonStream;
            jsonStream << "{\"voltage\":" << voltage 
                       << ",\"bpm\":" << bpmCalculated 
                       << ",\"alarm\":\"" << alarmState << "\"}\n";
            std::string jsonPayload = jsonStream.str();

            // 4. Stream over the TCP socket
            send(connectSocket, jsonPayload.c_str(), static_cast<int>(jsonPayload.length()), 0);
        }

        std::this_thread::sleep_for(std::chrono::microseconds(800));
    }

    closesocket(connectSocket);
    WSACleanup();
}