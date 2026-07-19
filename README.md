# Real-Time Cardiac Telemetry System

A full-stack biomedical engineering project demonstrating low-latency telemetry processing. This system captures simulated real-time hardware data, processes it via a C++ firmware engine, streams it across a TCP socket to a Node.js gateway backend, and displays it dynamically via a React web application.

## System Architecture

* **Firmware Engine (C++17):** Simulates a 500Hz ECG signal generator using a thread-safe circular buffer, evaluates raw voltages for R-peaks to calculate dynamic BPM, detects clinical anomalies (Tachycardia & Asystole), and broadcasts JSON packets over WinSock TCP.
* **Ingestion Backend (Node.js):** Acts as a real-time data gateway by hosting a TCP socket listener for the firmware engine and a WebSocket server to pipe streams directly to the browser.
* **Web Dashboard (React + Vite):** Uses the HTML5 Canvas API to draw a live scrolling medical grid display with responsive glowing alerts if a critical patient state is triggered.

## Getting Started

### Prerequisites
* Windows OS with Microsoft C++ Build Tools installed (`cl.exe`)
* Node.js (v16+)

### Installation & Execution

1.  **Clone the Repository:**
    ```bash
    git clone <your-repo-url>
    cd cardiac-telemetry-firmware
    ```

2.  **Launch the Backend Gateway:**
    ```bash
    cd backend
    npm install
    node server.js
    ```

3.  **Launch the React Frontend:**
    Open a new terminal window:
    ```bash
    cd frontend
    npm install
    npm run dev
    ```

4.  **Compile and Run the Firmware:**
    Open a Developer PowerShell or command window:
    ```bash
    cl.exe /EHsc /std:c++17 /Ifirmware/include firmware/src/*.cpp ws2_32.lib /Fefirmware/src/main.exe
    ./firmware/src/main.exe
    ```

### Simulation Controls
When running the firmware binary terminal, press the following keys to dynamically change the heart's biological rhythm:
* `N` - Normal Rhythm (~75 BPM)
* `T` - Inject Pathological Tachycardia (>140 BPM)
* `A` - Inject Asystole (Cardiac Arrest Flatline)
* `Q` - Safe System Shutdown