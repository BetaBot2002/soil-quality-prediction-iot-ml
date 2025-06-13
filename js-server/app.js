const http = require('http');
const fs = require('fs');
const path = require('path');
const url = require('url');

// Define the data file path
const dataFilePath = path.join(__dirname, 'data.json');

// Initialize data structure if file doesn't exist
if (!fs.existsSync(dataFilePath)) {
    const initialData = {
        moisture: 0,
        temperature: 0,
        humidity: 0,
        nitrogen: 0,
        phosphorus: 0,
        potassium: 0,
        timestamp: new Date().toISOString()
    };
    
    fs.writeFileSync(dataFilePath, JSON.stringify(initialData, null, 2));
    console.log('Created initial data.json file');
}

// Create HTTP server
const server = http.createServer((req, res) => {
    // Set CORS headers to allow requests from any origin
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
    res.setHeader('Access-Control-Allow-Headers', 'Content-Type');
    
    // Handle preflight requests
    if (req.method === 'OPTIONS') {
        res.writeHead(204);
        res.end();
        return;
    }
    
    // Parse the URL
    const parsedUrl = url.parse(req.url, true);
    const pathname = parsedUrl.pathname;
    
    // Route: GET /get_data
    if (pathname === '/get_data' && req.method === 'GET') {
        try {
            const data = fs.readFileSync(dataFilePath, 'utf8');
            res.writeHead(200, { 'Content-Type': 'application/json' });
            res.end(data);
        } catch (error) {
            console.error('Error reading data file:', error);
            res.writeHead(500, { 'Content-Type': 'application/json' });
            res.end(JSON.stringify({ error: 'Failed to read sensor data' }));
        }
    }
    // Route: POST /write_data
    else if (pathname === '/write_data' && req.method === 'POST') {
        let body = '';
        
        req.on('data', (chunk) => {
            body += chunk.toString();
        });
        
        req.on('end', () => {
            try {
                const sensorData = JSON.parse(body);
                
                // Validate required fields
                const requiredFields = ['moisture', 'temperature', 'humidity', 'nitrogen', 'phosphorus', 'potassium'];
                const missingFields = requiredFields.filter(field => sensorData[field] === undefined);
                
                if (missingFields.length > 0) {
                    res.writeHead(400, { 'Content-Type': 'application/json' });
                    res.end(JSON.stringify({ error: `Missing required fields: ${missingFields.join(', ')}` }));
                    return;
                }
                
                // Add timestamp
                sensorData.timestamp = new Date().toISOString();
                
                // Write to file
                fs.writeFileSync(dataFilePath, JSON.stringify(sensorData, null, 2));
                
                res.writeHead(200, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify({ status: 'success', message: 'Data saved successfully' }));
            } catch (error) {
                console.error('Error processing data:', error);
                res.writeHead(400, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify({ error: 'Invalid JSON data' }));
            }
        });
    }
    // Not found
    else {
        res.writeHead(404, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ error: 'Route not found' }));
    }
});

// Start server
const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
    console.log(`Server running at http://localhost:${PORT}/`);
    console.log(`Available endpoints:\n- GET /get_data\n- POST /write_data`);
});