# Real-Time Cardiac Telemetry and Pulse Classifier

A simulated real-time embedded firmware module built in modern C++ that processes streamable ECG voltage data, monitors a patient's heart rate, and triggers low-latency, firmware-level hardware interrupt alerts for critical cardiac anomalies.

## Embedded and Architectural Highlights

This project intentionally rejects desktop application patterns in favor of safety-critical, deterministic firmware constraints:

* **Zero Heap Allocation:** Dynamic memory allocation (new, delete, std::vector resizing) is strictly forbidden to guarantee absolute runtime determinism and eliminate the possibility of heap fragmentation or memory leaks in a life-critical environment.
* **Thread-Safe Circular Buffer:** Built a custom, fixed-size CircularBuffer using std::array wrapped in a mutex barrier to pass high-frequency data safely between producer and consumer threads without race conditions.
* **Multi-Threaded Sensor Simulation:** * **Sensor Thread:** Simulates a 500Hz physical ADC hardware sampling rate, generating mock ECG waveforms (P-wave, T-wave, and sharp QRS complexes).
    * **Analytics Thread:** Consumes the stream, tracking real-time intervals between R-peaks to calculate instantaneous Beats Per Minute (BPM).

## Firmware Safety Responses and Fault Conditions

The Telemetry Processor evaluates incoming data frame-by-frame and implements specific state-machine overrides when clinical thresholds are breached:

* **Normal Sinus Rhythm Handling:** Under nominal operation (60 to 100 BPM), the processor logs standard telemetry metrics including the current voltage vector and a rolling calculation of the heart rate.
* **Tachycardia Safety Mitigation:** If the calculated heart rate crosses the threshold of 140 BPM, the system bypasses standard diagnostic logging. It immediately calls an out-of-band high-priority routine simulating a hardware interrupt warning log to alert the medical monitoring network.
* **Asystole (Cardiac Arrest) Fault Mitigation:** The system tracks the elapsed time since the last verified R-peak. If no valid peak passes the threshold within 2500 milliseconds, the processor declares a critical device fault (Flatline condition). The engine triggers a continuous high-priority hardware interrupt loop to bypass normal operation and signal a life-threatening emergency.
* **Debouncing and Noise Rejection:** The processing engine relies on a hardware-level voltage threshold (1.5 V) combined with a digital debounce toggle. This ensures that broad voltage peaks or electromagnetic interference do not cause duplicate peak registration or false heart rate warnings.

## How to Build and Run

### Prerequisites
* A compiler supporting C++17 or C++20 (GCC, Clang, or MSVC)

### Build Instructions via Terminal
Navigate to the root directory and compile using:
```bash
g++ -std=c++17 -O2 src/*.cpp -Iinclude -lpthread -o telemetry_node