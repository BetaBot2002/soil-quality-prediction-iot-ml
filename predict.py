import joblib
import numpy as np
from typing import Dict, Any
import pandas as pd

def load_models() -> Dict[str, Any]:
    """Load all necessary models and encoders."""
    models = {
        'model': joblib.load('models/soil_testing_model.joblib'),
        'scaler': joblib.load('models/scaler.joblib'),
        'soil_encoder': joblib.load('models/soil_type_encoder.joblib'),
        'crop_encoder': joblib.load('models/crop_type_encoder.joblib'),
        'fertilizer_encoder': joblib.load('models/fertilizer_encoder.joblib')
    }
    return models

def get_user_input() -> Dict[str, Any]:
    """Get input parameters from user."""
    print("\n=== Soil Testing Prediction Interface ===\n")
    
    # Get numerical inputs
    temperature = float(input("Enter Temperature (°C): "))
    humidity = float(input("Enter Humidity (%): "))
    moisture = float(input("Enter Moisture (%): "))
    nitrogen = float(input("Enter Nitrogen content (mg/kg): "))
    potassium = float(input("Enter Potassium content (mg/kg): "))
    phosphorous = float(input("Enter Phosphorous content (mg/kg): "))
    
    # Get categorical inputs
    print("\nAvailable Soil Types:", ', '.join(models['soil_encoder'].classes_))
    soil_type = input("Enter Soil Type: ")
    
    print("\nAvailable Crop Types:", ', '.join(models['crop_encoder'].classes_))
    crop_type = input("Enter Crop Type: ")
    
    return {
        'Temperature': temperature,
        'Humidity': humidity,
        'Moisture': moisture,
        'Nitrogen': nitrogen,
        'Potassium': potassium,
        'Phosphorous': phosphorous,
        'Soil Type': soil_type,
        'Crop Type': crop_type
    }

def prepare_input(user_input: Dict[str, Any], models: Dict[str, Any]) -> np.ndarray:
    """Prepare user input for model prediction."""
    # Encode categorical variables
    soil_encoded = models['soil_encoder'].transform([user_input['Soil Type']])[0]
    crop_encoded = models['crop_encoder'].transform([user_input['Crop Type']])[0]
    
    # Create feature array with proper feature names
    features_df = pd.DataFrame({
        'Temparature': [user_input['Temperature']],
        'Humidity': [user_input['Humidity']],
        'Moisture': [user_input['Moisture']],
        'Soil Type': [soil_encoded],
        'Crop Type': [crop_encoded],
        'Nitrogen': [user_input['Nitrogen']],
        'Potassium': [user_input['Potassium']],
        'Phosphorous': [user_input['Phosphorous']]
    })
    
    # Scale features
    features_scaled = models['scaler'].transform(features_df)
    return features_scaled

def main():
    global models
    try:
        # Load models and encoders
        print("Loading models...")
        models = load_models()
        
        while True:
            # Get user input
            user_input = get_user_input()
            
            # Prepare input for prediction
            features = prepare_input(user_input, models)
            
            # Make prediction
            prediction = models['model'].predict(features)[0]
            fertilizer = models['fertilizer_encoder'].inverse_transform([prediction])[0]
            
            # Display results
            print("\n=== Prediction Results ===")
            print(f"Recommended Fertilizer: {fertilizer}")
            
            # Ask if user wants to make another prediction
            again = input("\nWould you like to make another prediction? (yes/no): ")
            if again.lower() != 'yes':
                break
                
    except FileNotFoundError:
        print("Error: Model files not found. Please run 'soil_testing_model.py' first to train the model.")
    except Exception as e:
        print(f"An error occurred: {str(e)}")

if __name__ == "__main__":
    main() 