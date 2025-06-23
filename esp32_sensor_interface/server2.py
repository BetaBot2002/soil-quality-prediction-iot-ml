from flask import Flask, render_template, request, jsonify
import json
import os
import datetime
import joblib
import numpy as np
import requests

app = Flask(__name__)

# ThingSpeak API configuration
# To set up ThingSpeak:
# 1. Create a free account at https://thingspeak.com
# 2. Create a new channel with the following fields:
#    - Field1: Humidity
#    - Field2: Temperature
#    - Field3: Moisture
#    - Field4: Nitrogen
#    - Field5: Phosphorus
#    - Field6: Potassium
# 3. Get your Channel ID and Read API Key from the API Keys tab
# 4. Replace the placeholder values below with your actual ThingSpeak credentials
THINGSPEAK_CHANNEL_ID = "2989083"  # Replace with your ThingSpeak channel ID
THINGSPEAK_READ_API_KEY = "R6LCX8EH46PUEC5Q"  # Replace with your ThingSpeak Read API Key
THINGSPEAK_API_ENDPOINT = f"https://api.thingspeak.com/channels/{THINGSPEAK_CHANNEL_ID}/feeds/last.json?api_key={THINGSPEAK_READ_API_KEY}"

# Load models and encoders
def load_models():
    # Get the absolute path to the models directory
    current_dir = os.path.dirname(os.path.abspath(__file__))
    models_dir = os.path.join(os.path.dirname(current_dir), 'models')
    
    # Create models directory if it doesn't exist
    if not os.path.exists(models_dir):
        os.makedirs(models_dir)
        print(f"Created models directory at {models_dir}")
        print("Please run soil_testing_model.py first to train the model.")
        return None
    
    try:
        models = {
            'model': joblib.load(os.path.join(models_dir, 'soil_testing_model.joblib')),
            'scaler': joblib.load(os.path.join(models_dir, 'scaler.joblib')),
            'soil_encoder': joblib.load(os.path.join(models_dir, 'soil_type_encoder.joblib')),
            'crop_encoder': joblib.load(os.path.join(models_dir, 'crop_type_encoder.joblib')),
            'fertilizer_encoder': joblib.load(os.path.join(models_dir, 'fertilizer_encoder.joblib'))
        }
        return models
    except FileNotFoundError as e:
        print(f"Error loading model files: {e}")
        print("Please run soil_testing_model.py first to train the model.")
        return None

# Initialize models
models = load_models()

# Initialize latest readings
latest_readings = {
    'temperature': 25.0,
    'humidity': 60.0,
    'moisture': 35.0,
    'nitrogen': 20.0,
    'phosphorus': 15.0,
    'potassium': 25.0,
    'soil_type': 1,  # Default to Loamy
    'crop_type': 0,  # Default to Maize
    'recommended_fertilizer': 'No recommendation yet',
    'timestamp': datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
}

# Function to fetch data from ThingSpeak API
def fetch_sensor_data():
    try:
        response = requests.get(THINGSPEAK_API_ENDPOINT)
        if response.status_code == 200:
            thingspeak_data = response.json()
            
            # Map ThingSpeak fields to our application's data structure
            # Field1: Humidity, Field2: Temperature, Field3: Moisture, 
            # Field4: Nitrogen, Field5: Phosphorus, Field6: Potassium
            data = {
                'humidity': float(thingspeak_data.get('field1', 0)) or latest_readings['humidity'],
                'temperature': float(thingspeak_data.get('field2', 0)) or latest_readings['temperature'],
                'moisture': float(thingspeak_data.get('field3', 0)) or latest_readings['moisture'],
                'nitrogen': float(thingspeak_data.get('field4', 0)) or latest_readings['nitrogen'],
                'phosphorus': float(thingspeak_data.get('field5', 0)) or latest_readings['phosphorus'],
                'potassium': float(thingspeak_data.get('field6', 0)) or latest_readings['potassium']
            }
            
            # Add timestamp from ThingSpeak
            data['created_at'] = thingspeak_data.get('created_at', datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
            
            print("Successfully fetched data from ThingSpeak")
            return data
        else:
            print(f"Error fetching data from ThingSpeak: {response.status_code}")
            return None
    except Exception as e:
        print(f"Exception while fetching data from ThingSpeak: {e}")
        return None

# Add this after the models initialization and before the predict_fertilizer function
# Cache to store soil-crop-fertilizer mappings
fertilizer_cache = {}

def predict_fertilizer(data):
    """Make prediction using the AI model."""
    if models is None:
        return "Model not loaded. Please train the model first."

    try:
        # Prepare input data
        features = np.array([
            data['temperature'],
            data['humidity'],
            data['moisture'],
            data['soil_type'],
            data.get('crop_type', 0),
            data['nitrogen'],
            data['phosphorus'],
            data['potassium']
        ]).reshape(1, -1)

        # Scale features
        features_scaled = models['scaler'].transform(features)

        # Make prediction
        prediction = models['model'].predict(features_scaled)[0]
        fertilizer = models['fertilizer_encoder'].inverse_transform([prediction])[0]

        # Get soil type and crop type
        soil_type = data['soil_type']
        crop_type = data.get('crop_type', 0)

        # Check if this soil type has any fertilizer recommendations already
        if soil_type in fertilizer_cache:
            existing_fertilizers = set(fertilizer_cache[soil_type].values())

            # If recommended fertilizer is already assigned to another crop on same soil
            if fertilizer in existing_fertilizers:
                # Get all possible fertilizers
                all_fertilizers = set(models['fertilizer_encoder'].classes_)

                # Remove fertilizers already assigned for this soil type
                alternative_fertilizers = list(all_fertilizers - existing_fertilizers)

                if alternative_fertilizers:
                    # Pick first available alternative
                    fertilizer = alternative_fertilizers[0]
                    print(f"Changed fertilizer recommendation for soil {soil_type}, crop {crop_type} "
                          f"to avoid conflicts with existing assignments.")
                else:
                    print(f"No alternative fertilizer available for soil {soil_type}. Keeping original: {fertilizer}")

        # Store this recommendation in the cache
        if soil_type not in fertilizer_cache:
            fertilizer_cache[soil_type] = {}
        fertilizer_cache[soil_type][crop_type] = fertilizer

        return fertilizer

    except Exception as e:
        print(f"Error in predict_fertilizer: {e}")
        return "Prediction error"


@app.route('/')
def home():
    """Render the web interface."""
    # Fetch latest data from the API
    api_data = fetch_sensor_data()
    
    if api_data:
        # Update latest readings with data from API
        latest_readings.update({
            'temperature': api_data.get('temperature', latest_readings['temperature']),
            'humidity': api_data.get('humidity', latest_readings['humidity']),
            'moisture': api_data.get('moisture', latest_readings['moisture']),
            'nitrogen': api_data.get('nitrogen', latest_readings['nitrogen']),
            'phosphorus': api_data.get('phosphorus', latest_readings['phosphorus']),
            'potassium': api_data.get('potassium', latest_readings['potassium']),
            'timestamp': datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        })
        
        # Get fertilizer recommendation based on updated data
        fertilizer = predict_fertilizer(latest_readings)
        latest_readings['recommended_fertilizer'] = fertilizer
    
    # Define soil types and crop types
    soil_types = [
        (0, "Sandy"),
        (1, "Loamy"),
        (2, "Black"),
        (3, "Red"),
        (4, "Clayey")
    ]
    
    crop_types = [
        (0, "Maize"),
        (1, "Sugarcane"),
        (2, "Cotton"),
        (3, "Tobacco"),
        (4, "Paddy"),
        (5, "Barley"),
        (6, "Wheat"),
        (7, "Millets"),
        (8, "Oil seeds"),
        (9, "Pulses"),
        (10, "Ground Nuts")
    ]
    
    return render_template('index2.html', 
                          readings=latest_readings, 
                          soil_types=soil_types,
                          crop_types=crop_types)

@app.route('/sensor-data', methods=['POST', 'GET'])
def receive_data():
    """Receive and process sensor data."""
    try:
        if request.method == 'GET':
            # For GET requests, fetch fresh data from ThingSpeak
            thingspeak_data = fetch_sensor_data()
            
            if thingspeak_data:
                # Update latest readings with ThingSpeak data
                latest_readings.update(thingspeak_data)
                timestamp = thingspeak_data.get('created_at', datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
                latest_readings['timestamp'] = timestamp
                
                # Get fertilizer recommendation based on updated readings
                fertilizer = predict_fertilizer(fertilizer = predict_fertilizer(latest_readings)
)
                latest_readings['recommended_fertilizer'] = fertilizer
                
                return jsonify({
                    'status': 'success',
                    'data': latest_readings,
                    'recommended_fertilizer': fertilizer,
                    'timestamp': timestamp
                })
            else:
                # If ThingSpeak fetch fails, return cached data with warning
                fertilizer = latest_readings.get('recommended_fertilizer', 'Unknown')
                return jsonify({
                    'status': 'success',
                    'data': latest_readings,
                    'recommended_fertilizer': fertilizer,
                    'timestamp': latest_readings.get('timestamp', datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")),
                    'warning': 'Using cached data. Could not fetch new data from ThingSpeak.'
                })
        else:  # POST request
            # For POST requests, use the data sent in the request
            data = request.json
            
            # Update latest readings with POST data
            latest_readings.update({
                'temperature': data.get('temperature', latest_readings['temperature']),
                'humidity': data.get('humidity', latest_readings['humidity']),
                'moisture': data.get('moisture', latest_readings['moisture']),
                'nitrogen': data.get('nitrogen', latest_readings['nitrogen']),
                'phosphorus': data.get('phosphorus', latest_readings['phosphorus']),
                'potassium': data.get('potassium', latest_readings['potassium']),
                'timestamp': datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            })
        
        # Get fertilizer recommendation
        fertilizer = predict_fertilizer(latest_readings)
        latest_readings['recommended_fertilizer'] = fertilizer
        
        # Log data to file
        log_data = {
            'timestamp': latest_readings['timestamp'],
            'temperature': latest_readings['temperature'],
            'humidity': latest_readings['humidity'],
            'moisture': latest_readings['moisture'],
            'soil_type': latest_readings['soil_type'],
            'nitrogen': latest_readings['nitrogen'],
            'phosphorus': latest_readings['phosphorus'],
            'potassium': latest_readings['potassium'],
            'crop_type': latest_readings['crop_type'],
            'fertilizer': fertilizer,
            'source': 'API' if 'api_data' in locals() and api_data else 'Direct POST'
        }
        
        # Save to log file
        log_file = os.path.join('data', 'sensor_log.json')
        try:
            if os.path.exists(log_file):
                with open(log_file, 'r') as f:
                    logs = json.load(f)
            else:
                logs = []
            
            logs.append(log_data)
            
            with open(log_file, 'w') as f:
                json.dump(logs, f, indent=4)
        except Exception as e:
            print(f"Error saving log: {e}")
            
            # Also save to logs directory as backup
            logs_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'logs')
            if not os.path.exists(logs_dir):
                os.makedirs(logs_dir)
            
            with open(os.path.join(logs_dir, 'sensor_log.json'), 'a') as f:
                json.dump(log_data, f)
                f.write('\n')
        
        return jsonify({
            'status': 'success',
            'data': latest_readings,
            'recommended_fertilizer': fertilizer
        })
        
    except Exception as e:
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 400

@app.route('/update-crop', methods=['POST'])
def update_crop():
    """Update crop type and get new recommendation."""
    try:
        crop_type = request.json['crop_type']
        latest_readings['crop_type'] = int(crop_type)
        
        # Get fertilizer recommendation with updated crop type
        fertilizer = predict_fertilizer(latest_readings)
        latest_readings['recommended_fertilizer'] = fertilizer
        
        return jsonify({
            'status': 'success',
            'recommended_fertilizer': fertilizer
        })
        
    except Exception as e:
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 400

@app.route('/update-soil', methods=['POST'])
def update_soil():
    """Update soil type and get new recommendation."""
    try:
        soil_type = request.json['soil_type']
        latest_readings['soil_type'] = int(soil_type)
        
        # Get fertilizer recommendation with updated soil type
        fertilizer = predict_fertilizer(latest_readings)
        latest_readings['recommended_fertilizer'] = fertilizer
        
        return jsonify({
            'status': 'success',
            'recommended_fertilizer': fertilizer
        })
        
    except Exception as e:
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 400

# Removed automatic refresh on every request to make refreshing manual only
# The refresh now happens only when explicitly requested via the refresh button

if __name__ == '__main__':
    # Create templates directory if it doesn't exist
    templates_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'templates')
    if not os.path.exists(templates_dir):
        os.makedirs(templates_dir)
    
    # Create logs directory if it doesn't exist
    logs_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'logs')
    if not os.path.exists(logs_dir):
        os.makedirs(logs_dir)
    
    # Create data directory if it doesn't exist
    os.makedirs("data", exist_ok=True)
    
    # Fetch initial data from ThingSpeak
    print("Attempting to fetch initial data from ThingSpeak...")
    initial_data = fetch_sensor_data()
    if initial_data:
        latest_readings.update(initial_data)
        print("Initial data loaded successfully from ThingSpeak")
    else:
        print("Could not fetch initial data from ThingSpeak. Using default values.")
        
    app.run(debug=True, host='0.0.0.0', port=5000)