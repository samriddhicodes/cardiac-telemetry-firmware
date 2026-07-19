const net = require('net');
const { WebSocketServer } = require('ws');

// 1. Initialize the WebSocket Server for the Frontend Dashboard
const wss = new WebSocketServer({ port: 8080 });
console.log('WebSocket server listening for frontend on port 8080');

// Keep track of connected frontend clients
let connectedClients = [];
wss.on('connection', (ws) => {
    console.log('Frontend dashboard client connected');
    connectedClients.push(ws);

    ws.on('close', () => {
        connectedClients = connectedClients.filter(client => client !== ws);
        console.log('Frontend dashboard client disconnected');
    });
});

// Broadcast helper to send telemetry packets to all open browser tabs
function broadcastToDashboard(data) {
    const payload = JSON.stringify(data);
    connectedClients.forEach(client => {
        if (client.readyState === 1) { // 1 means OPEN
            client.send(payload);
        }
    });
}

// 2. Initialize the TCP Socket Server to ingest data from C++ Firmware
const tcpServer = net.createServer((socket) => {
    console.log('C++ Telemetry Firmware Node linked successfully via TCP');

    let buffer = '';
    socket.on('data', (data) => {
        buffer += data.toString();
        
        // Handle stream buffering: break incoming packets by newline
        let boundary = buffer.indexOf('\n');
        while (boundary !== -1) {
            const rawPacket = buffer.substring(0, boundary).trim();
            buffer = buffer.substring(boundary + 1);
            
            if (rawPacket) {
                try {
                    const telemetryData = JSON.parse(rawPacket);
                    
                    // Add a server-side timestamp
                    telemetryData.receivedAt = new Date().toISOString();
                    
                    // Forward straight to the frontend dashboard
                    broadcastToDashboard(telemetryData);
                } catch (err) {
                    // Ignore partial/corrupted packets during initial connection handshake
                }
            }
            boundary = buffer.indexOf('\n');
        }
    });

    socket.on('end', () => {
        console.log('C++ Telemetry Firmware Node disconnected');
    });
});

tcpServer.listen(5001, '127.0.0.1', () => {
    console.log('TCP Ingestion Server listening for firmware data on 127.0.0.1:5001');
});