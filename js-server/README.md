# Soil Sensor Data Server

A simple Node.js HTTP server for handling soil sensor data. This server provides two endpoints:

1. `GET /get_data` - Retrieves the current sensor data
2. `POST /write_data` - Updates the sensor data

## Setup

```bash
# Install dependencies
npm install

# Start the server
npm start
```

The server will run on port 3000 by default. You can change this by setting the PORT environment variable.

## API Usage

### Get Sensor Data

```bash
curl http://localhost:3000/get_data
```

Response:
```json
{
  "moisture": 45.2,
  "temperature": 25.3,
  "humidity": 68.7,
  "nitrogen": 42,
  "phosphorus": 35,
  "potassium": 28,
  "timestamp": "2023-06-15T12:34:56.789Z"
}
```

### Write Sensor Data

```bash
curl -X POST -H "Content-Type: application/json" -d '{"moisture":45.2,"temperature":25.3,"humidity":68.7,"nitrogen":42,"phosphorus":35,"potassium":28}' http://localhost:3000/write_data
```

Response:
```json
{
  "status": "success",
  "message": "Data saved successfully"
}
```

## Data Structure

The sensor data is stored in a file named `data.json` with the following structure:

```json
{
  "moisture": 45.2,      // Soil moisture percentage
  "temperature": 25.3,   // Temperature in Celsius
  "humidity": 68.7,      // Air humidity percentage
  "nitrogen": 42,        // Nitrogen level in mg/kg
  "phosphorus": 35,      // Phosphorus level in mg/kg
  "potassium": 28,       // Potassium level in mg/kg
  "timestamp": "2023-06-15T12:34:56.789Z"  // Last update time
}
```