from flask import Flask, request, jsonify, render_template
import joblib
import numpy as np
import json
from datetime import datetime
import os

app = Flask(__name__)

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

# Store latest readings
latest_readings = {
    'temperature': 0,
    'humidity': 0,
    'moisture': 0,
    'soil_type': 0,
    'nitrogen': 0,
    'phosphorous': 0,
    'potassium': 0,
    'timestamp': None,
    'recommended_fertilizer': None
}

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
        0,  # Default crop type (can be modified through web interface)
        data['nitrogen'],
        data['phosphorous'],
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
    return render_template('index.html', readings=latest_readings)

@app.route('/sensor-data', methods=['POST'])
def receive_data():
    """Receive and process sensor data."""
    try:
        data = request.json
        
        # Update latest readings
        latest_readings.update({
            'temperature': data['temperature'],
            'humidity': data['humidity'],
            'moisture': data['moisture'],
            'soil_type': data['soil_type'],
            'nitrogen': data['nitrogen'],
            'phosphorous': data['phosphorous'],
            'potassium': data['potassium'],
            'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        })
        
        # Get fertilizer recommendation
        fertilizer = predict_fertilizer(data)
        latest_readings['recommended_fertilizer'] = fertilizer
        
        # Save to log file
        log_data = {
            **data,
            'timestamp': latest_readings['timestamp'],
            'recommended_fertilizer': fertilizer
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
        if models is None:
            return jsonify({
                'status': 'error',
                'message': 'Model not loaded. Please train the model first.'
            }), 400
            
        crop_type = request.json['crop_type']
        
        # Prepare input data with new crop type
        features = np.array([
            latest_readings['temperature'],
            latest_readings['humidity'],
            latest_readings['moisture'],
            latest_readings['soil_type'],
            crop_type,
            latest_readings['nitrogen'],
            latest_readings['phosphorous'],
            latest_readings['potassium']
        ]).reshape(1, -1)
        
        # Scale features
        features_scaled = models['scaler'].transform(features)
        
        # Make prediction
        prediction = models['model'].predict(features_scaled)[0]
        fertilizer = models['fertilizer_encoder'].inverse_transform([prediction])[0]
        
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

if __name__ == '__main__':
    # Create templates directory if it doesn't exist
    templates_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'templates')
    if not os.path.exists(templates_dir):
        os.makedirs(templates_dir)
    
    # Check if models are loaded
    if models is None:
        print("WARNING: Models not loaded. The server will start but won't be able to make predictions.")
        print("Please run soil_testing_model.py first to train the model.")
    
    app.run(host='0.0.0.0', port=5000, debug=True) 