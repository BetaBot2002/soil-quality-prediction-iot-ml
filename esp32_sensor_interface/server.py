from flask import Flask, render_template, request, jsonify
import json
import os
import datetime
import joblib
import numpy as np
import requests

app = Flask(__name__)

# API endpoint for fetching sensor data
API_ENDPOINT = "https://soil-quality-prediction-iot-ml.onrender.com/get_data"

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

# Function to fetch data from the external API
def fetch_sensor_data():
    try:
        response = requests.get(API_ENDPOINT)
        if response.status_code == 200:
            data = response.json()
            return data
        else:
            print(f"Error fetching data: {response.status_code}")
            return None
    except Exception as e:
        print(f"Exception while fetching data: {e}")
        return None

def predict_fertilizer(data):
    """Make prediction using the AI model."""
    if models is None:
        return "Model not loaded. Please train the model first."
    
    # Prepare input data
    features = np.array([
        data['temperature'],
        data['humidity'],
        data['moisture'],
        data['soil_type'],
        data.get('crop_type', 0),  # Default crop type (can be modified through web interface)
        data['nitrogen'],
        data['phosphorus'],
        data['potassium']
    ]).reshape(1, -1)
    
    # Scale features
    features_scaled = models['scaler'].transform(features)
    
    # Make prediction
    prediction = models['model'].predict(features_scaled)[0]
    fertilizer = models['fertilizer_encoder'].inverse_transform([prediction])[0]
    
    return fertilizer

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
        (6, "Wheat")
    ]
    
    return render_template('index.html', 
                          readings=latest_readings, 
                          soil_types=soil_types,
                          crop_types=crop_types)

@app.route('/sensor-data', methods=['POST'])
def receive_data():
    """Receive and process sensor data."""
    try:
        # First try to get data from the API
        api_data = fetch_sensor_data()
        
        if api_data:
            # Use API data
            data = api_data
        else:
            # Fallback to request data if API fails
            data = request.json
        
        # Update latest readings
        latest_readings.update({
            'temperature': data.get('temperature', latest_readings['temperature']),
            'humidity': data.get('humidity', latest_readings['humidity']),
            'moisture': data.get('moisture', latest_readings['moisture']),
            'soil_type': data.get('soil_type', latest_readings['soil_type']),
            'nitrogen': data.get('nitrogen', latest_readings['nitrogen']),
            'phosphorus': data.get('phosphorus', latest_readings['phosphorus']),
            'potassium': data.get('potassium', latest_readings['potassium']),
            'timestamp': datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        })
        
        # Get fertilizer recommendation
        fertilizer = predict_fertilizer(latest_readings)
        latest_readings['recommended_fertilizer'] = fertilizer
        
        # Save to log file
        log_data = {
            **latest_readings,
            'source': 'API' if api_data else 'Direct POST'
        }
        
        # Create logs directory if it doesn't exist
        logs_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'logs')
        if not os.path.exists(logs_dir):
            os.makedirs(logs_dir)
        
        with open(os.path.join(logs_dir, 'sensor_log.json'), 'a') as f:
            json.dump(log_data, f)
            f.write('\n')
        
        return jsonify({
            'status': 'success',
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

# Function to refresh data from API periodically
@app.before_request
def refresh_data():
    """Refresh data from API before processing requests."""
    # Only refresh data for GET requests to avoid unnecessary API calls
    if request.method == 'GET':
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

if __name__ == '__main__':
    # Create templates directory if it doesn't exist
    templates_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'templates')
    if not os.path.exists(templates_dir):
        os.makedirs(templates_dir)
    
    # Create logs directory if it doesn't exist
    logs_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'logs')
    if not os.path.exists(logs_dir):
        os.makedirs(logs_dir)
    
    # Initial data fetch from API
    print("Fetching initial data from API...")
    api_data = fetch_sensor_data()
    if api_data:
        print("Successfully fetched initial data from API")
    else:
        print("WARNING: Could not fetch initial data from API. Using default values.")
    
    app.run(host='0.0.0.0', port=5000, debug=True)