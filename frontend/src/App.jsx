import React, { useEffect, useState, useRef } from 'react';
import './App.css';

function App() {
  const [bpm, setBpm] = useState(0);
  const [alarm, setAlarm] = useState('NONE');
  const canvasRef = useRef(null);
  const dataPointsRef = useRef([]);
  const maxPoints = 300; // Total horizontal resolution for the scrolling frame

  useEffect(() => {
    // Connect to the Node.js WebSocket gateway
    const ws = new WebSocket('ws://localhost:8080');

    ws.onmessage = (event) => {
      try {
        const packet = JSON.parse(event.data);
        
        // Update BPM if a new peak calculation arrived
        if (packet.bpm > 0) {
          setBpm(packet.bpm);
        }
        
        // Update global alarm status state
        if (packet.alarm && packet.alarm !== 'NONE') {
          setAlarm(packet.alarm);
        } else if (packet.bpm > 0) {
          setAlarm('NONE'); // Reset alarm state smoothly when nominal BPM returns
        }

        // Push new voltage point to scrolling window buffer
        dataPointsRef.current.push(packet.voltage);
        if (dataPointsRef.current.length > maxPoints) {
          dataPointsRef.current.shift();
        }
      } catch (err) {
        console.error('Error parsing incoming telemetry packet:', err);
      }
    };

    return () => ws.close();
  }, []);

  // Animation loop to draw the scrolling ECG vector smoothly
  useEffect(() => {
    let animationFrameId;
    const canvas = canvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext('2d');

    const renderWave = () => {
      ctx.clearRect(0, 0, canvas.width, canvas.height);

      // Draw standard clinical grid background
      ctx.strokeStyle = '#1e293b';
      ctx.lineWidth = 0.5;
      const gridSize = 20;
      for (let x = 0; x < canvas.width; x += gridSize) {
        ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, canvas.height); ctx.stroke();
      }
      for (let y = 0; y < canvas.height; y += gridSize) {
        ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(canvas.width, y); ctx.stroke();
      }

      // Draw the ECG Waveform trace line
      const points = dataPointsRef.current;
      if (points.length > 1) {
        ctx.strokeStyle = alarm !== 'NONE' ? '#ef4444' : '#10b981'; // Red on alarm, green on normal
        ctx.lineWidth = 2.5;
        ctx.shadowBlur = 8;
        ctx.shadowColor = alarm !== 'NONE' ? '#ef4444' : '#10b981';
        ctx.beginPath();

        for (let i = 0; i < points.length; i++) {
          const x = (canvas.width / maxPoints) * i;
          // Invert and map the voltage values nicely into canvas pixel limits
          const y = (canvas.height / 2) - (points[i] * 40); 

          if (i === 0) ctx.moveTo(x, y);
          else ctx.lineTo(x, y);
        }
        ctx.stroke();
        ctx.shadowBlur = 0; // Reset shadow configuration
      }

      animationFrameId = requestAnimationFrame(renderWave);
    };

    renderWave();
    return () => cancelAnimationFrame(animationFrameId);
  }, [alarm]);

  return (
    <div className={`app-container ${alarm !== 'NONE' ? 'panic-mode' : ''}`}>
      <header className="dashboard-header">
        <h1>ICU CENTRAL STATION – TELEMETRY GATEWAY</h1>
        <div className="status-badge">SYSTEM OPERATIONAL</div>
      </header>

      <main className="dashboard-grid">
        <div className="card vitals-card">
          <h2>Heart Rate</h2>
          <div className="metric-display">
            <span className="value">{bpm}</span>
            <span className="unit">BPM</span>
          </div>
        </div>

        <div className="card alarm-card">
          <h2>System Alerts</h2>
          <div className={`alarm-banner ${alarm !== 'NONE' ? 'active' : 'nominal'}`}>
            {alarm !== 'NONE' ? `⚠️ CRITICAL: ${alarm}` : '🟢 NOMINAL SYSTEM STATUS'}
          </div>
        </div>

        <div className="card chart-card">
          <h2>Real-Time Live Electrocardiogram (ECG) Vector</h2>
          <canvas ref={canvasRef} width={800} height={300} className="ecg-canvas" />
        </div>
      </main>
    </div>
  );
}

export default App;
