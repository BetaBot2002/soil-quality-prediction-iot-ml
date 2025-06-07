# Soil Testing AI Model

This project implements a machine learning model for soil testing and fertilizer recommendation based on various soil parameters and crop types.

## Features

- Predicts suitable fertilizer based on:
  - Temperature
  - Humidity
  - Moisture
  - Soil Type
  - Crop Type
  - Nitrogen levels
  - Potassium levels
  - Phosphorous levels

## Setup Instructions

1. Create a virtual environment:
```bash
python -m venv venv
```

2. Activate the virtual environment:
- Windows:
```bash
.\venv\Scripts\activate
```
- Linux/Mac:
```bash
source venv/bin/activate
```

3. Install required packages:
```bash
pip install -r requirements.txt
```

4. Train the model (if not already trained):
```bash
python soil_testing_model.py
```

5. Make predictions using the interface:
```bash
python predict.py
```

## Model Output

The model will:
1. Load and preprocess the data
2. Train a Random Forest Classifier
3. Save the trained model and necessary encoders in the `models` directory
4. Generate a confusion matrix visualization
5. Print model performance metrics

## Prediction Interface

The `predict.py` script provides an interactive interface to:
1. Input soil parameters (temperature, humidity, moisture)
2. Input soil composition (nitrogen, potassium, phosphorous)
3. Select soil type and crop type from available options
4. Get fertilizer recommendations based on the input parameters

Example usage:
```
=== Soil Testing Prediction Interface ===

Enter Temperature (°C): 28
Enter Humidity (%): 54
Enter Moisture (%): 46
Enter Nitrogen content (mg/kg): 35
Enter Potassium content (mg/kg): 0
Enter Phosphorous content (mg/kg): 0

Available Soil Types: Clayey, Loamy, Red, Sandy
Enter Soil Type: Clayey

Available Crop Types: Barley, Cotton, Maize, Paddy, Sugarcane, Tobacco, Wheat
Enter Crop Type: Paddy

=== Prediction Results ===
Recommended Fertilizer: Urea
```

## Model Files

After training, the following files will be created in the `models` directory:
- `soil_testing_model.joblib`: The trained model
- `soil_type_encoder.joblib`: Encoder for soil types
- `crop_type_encoder.joblib`: Encoder for crop types
- `fertilizer_encoder.joblib`: Encoder for fertilizer names
- `scaler.joblib`: Feature scaler
- `confusion_matrix.png`: Visualization of model performance

## Dataset

The model uses the `data_core.csv` dataset located in the `dataset` directory, which contains soil parameters and corresponding fertilizer recommendations. 