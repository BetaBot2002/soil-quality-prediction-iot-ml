import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder, StandardScaler
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import classification_report, confusion_matrix
import joblib
import seaborn as sns
import matplotlib.pyplot as plt

# Load the dataset
def load_data():
    df = pd.read_csv('dataset/data_core.csv')
    return df

# Preprocess the data
def preprocess_data(df):
    # Create label encoders for categorical variables
    le_soil = LabelEncoder()
    le_crop = LabelEncoder()
    le_fertilizer = LabelEncoder()
    
    # Encode categorical variables
    df['Soil Type'] = le_soil.fit_transform(df['Soil Type'])
    df['Crop Type'] = le_crop.fit_transform(df['Crop Type'])
    df['Fertilizer Name'] = le_fertilizer.fit_transform(df['Fertilizer Name'])
    
    # Save label encoders for future use
    joblib.dump(le_soil, 'models/soil_type_encoder.joblib')
    joblib.dump(le_crop, 'models/crop_type_encoder.joblib')
    joblib.dump(le_fertilizer, 'models/fertilizer_encoder.joblib')
    
    return df

def train_model(X, y):
    # Split the data
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
    
    # Scale the features
    scaler = StandardScaler()
    X_train_scaled = scaler.fit_transform(X_train)
    X_test_scaled = scaler.transform(X_test)
    
    # Save the scaler
    joblib.dump(scaler, 'models/scaler.joblib')
    
    # Create and train the model
    model = RandomForestClassifier(n_estimators=100, random_state=42)
    model.fit(X_train_scaled, y_train)
    
    # Make predictions
    y_pred = model.predict(X_test_scaled)
    
    # Print model performance
    print("\nModel Performance:")
    print(classification_report(y_test, y_pred))
    
    # Create confusion matrix
    cm = confusion_matrix(y_test, y_pred)
    plt.figure(figsize=(10, 8))
    sns.heatmap(cm, annot=True, fmt='d', cmap='Blues')
    plt.title('Confusion Matrix')
    plt.ylabel('True Label')
    plt.xlabel('Predicted Label')
    plt.savefig('models/confusion_matrix.png')
    plt.close()
    
    return model

def main():
    # Create models directory if it doesn't exist
    import os
    if not os.path.exists('models'):
        os.makedirs('models')
    
    # Load and preprocess data
    print("Loading and preprocessing data...")
    df = load_data()
    df_processed = preprocess_data(df)
    
    # Prepare features and target
    X = df_processed.drop('Fertilizer Name', axis=1)
    y = df_processed['Fertilizer Name']
    
    # Train the model
    print("Training the model...")
    model = train_model(X, y)
    
    # Save the model
    print("Saving the model...")
    joblib.dump(model, 'models/soil_testing_model.joblib')
    print("Model saved successfully!")

if __name__ == "__main__":
    main() 