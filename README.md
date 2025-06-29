# Soil Testing AI Model

This project implements a machine learning model for soil testing and fertilizer recommendation based on various soil parameters and crop types.

## Features

* Predicts suitable fertilizer based on:

  * Temperature
  * Humidity
  * Moisture
  * Soil Type
  * Crop Type
  * Nitrogen levels
  * Potassium levels
  * Phosphorous levels

## Setup Instructions

1. **Create a virtual environment:**

   ```bash
   python -m venv venv
   ```

2. **Activate the virtual environment:**

   * **Windows:**

     ```bash
     .\venv\Scripts\activate
     ```
   * **Linux/Mac:**

     ```bash
     source venv/bin/activate
     ```

3. **Install required packages:**

   ```bash
   pip install -r requirements.txt
   ```

4. **Train the machine learning model (if not already trained):**

   ```bash
   python soil_testing_model.py
   ```

5. **After successful training, follow these steps to run the complete system:**

   * **Run the ESP32 sensor simulation via Wokwi:**

     * Open your Wokwi project
     * Start the simulation to transmit data to the Thingspeak channel

   * **Run the server to fetch data from Thingspeak and provide fertilizer predictions:**

     ```bash
     cd esp32_sensor_interface
     python server2.py
     ```

   * **Launch the Flutter mobile application:**

     ```bash
     cd soil_quality_app
     flutter pub get
     flutter run
     ```

## Model Output

Upon training:

1. The dataset is loaded and preprocessed
2. A Random Forest Classifier is trained
3. The trained model and encoders are saved in the `models` directory
4. A confusion matrix visualization is generated
5. Model performance metrics are printed to the console

## Prediction Interface (for manual testing)

You can still use the CLI-based prediction interface:

```bash
python predict.py
```

This will allow you to enter soil parameters and get fertilizer recommendations interactively via the terminal.

## Model Files

After training, the following files will be available in the `models` directory:

* `soil_testing_model.joblib` — Trained ML model
* `soil_type_encoder.joblib` — Encoder for soil types
* `crop_type_encoder.joblib` — Encoder for crop types
* `fertilizer_encoder.joblib` — Encoder for fertilizer names
* `scaler.joblib` — Feature scaler
* `confusion_matrix.png` — Model performance visualization

## Dataset

The model uses `data_core.csv` located in the `dataset` directory, containing soil parameters and their corresponding fertilizer recommendations.
