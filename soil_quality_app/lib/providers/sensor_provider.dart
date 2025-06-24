import 'package:flutter/foundation.dart';
import '../models/sensor_reading.dart';
import '../services/api_service.dart';

class SensorProvider with ChangeNotifier {
  SensorReading _sensorData = SensorReading.defaultValues();
  final ApiService _apiService = ApiService();
  bool _isLoading = false;
  String _error = '';

  SensorReading get sensorData => _sensorData;
  bool get isLoading => _isLoading;
  String get error => _error;

  // Soil types mapping
  final List<Map<String, dynamic>> soilTypes = [
    {'id': 0, 'name': 'Sandy'},
    {'id': 1, 'name': 'Loamy'},
    {'id': 2, 'name': 'Black'},
    {'id': 3, 'name': 'Red'},
    {'id': 4, 'name': 'Clayey'},
  ];

  // Crop types mapping
  final List<Map<String, dynamic>> cropTypes = [
    {'id': 0, 'name': 'Maize'},
    {'id': 1, 'name': 'Sugarcane'},
    {'id': 2, 'name': 'Cotton'},
    {'id': 3, 'name': 'Tobacco'},
    {'id': 4, 'name': 'Paddy'},
    {'id': 5, 'name': 'Barley'},
    {'id': 6, 'name': 'Wheat'},
    {'id': 7, 'name': 'Millets'},
    {'id': 8, 'name': 'Oil seeds'},
    {'id': 9, 'name': 'Pulses'},
    {'id': 10, 'name': 'Ground Nuts'},
  ];

  // Get soil name by id
  String getSoilName(int id) {
    final soil = soilTypes.firstWhere(
      (soil) => soil['id'] == id,
      orElse: () => {'id': id, 'name': 'Unknown'},
    );
    return soil['name'];
  }

  // Get crop name by id
  String getCropName(int id) {
    final crop = cropTypes.firstWhere(
      (crop) => crop['id'] == id,
      orElse: () => {'id': id, 'name': 'Unknown'},
    );
    return crop['name'];
  }

  // Fetch sensor data
  Future<void> fetchSensorData() async {
    _isLoading = true;
    _error = '';
    notifyListeners();

    try {
      final data = await _apiService.fetchSensorData();
      _sensorData = data;
      _isLoading = false;
      notifyListeners();
    } catch (e) {
      _error = 'Failed to fetch sensor data: $e';
      _isLoading = false;
      notifyListeners();
    }
  }

  // Update crop type
  Future<void> updateCropType(int cropType) async {
    _isLoading = true;
    notifyListeners();

    try {
      final recommendation = await _apiService.updateCropType(cropType);
      _sensorData = SensorReading(
        temperature: _sensorData.temperature,
        humidity: _sensorData.humidity,
        moisture: _sensorData.moisture,
        nitrogen: _sensorData.nitrogen,
        phosphorus: _sensorData.phosphorus,
        potassium: _sensorData.potassium,
        soilType: _sensorData.soilType,
        cropType: cropType,
        recommendedFertilizer: recommendation,
        timestamp: DateTime.now(),
      );
      _isLoading = false;
      notifyListeners();
    } catch (e) {
      _error = 'Failed to update crop type: $e';
      _isLoading = false;
      notifyListeners();
    }
  }

  // Update soil type
  Future<void> updateSoilType(int soilType) async {
    _isLoading = true;
    notifyListeners();

    try {
      final recommendation = await _apiService.updateSoilType(soilType);
      _sensorData = SensorReading(
        temperature: _sensorData.temperature,
        humidity: _sensorData.humidity,
        moisture: _sensorData.moisture,
        nitrogen: _sensorData.nitrogen,
        phosphorus: _sensorData.phosphorus,
        potassium: _sensorData.potassium,
        soilType: soilType,
        cropType: _sensorData.cropType,
        recommendedFertilizer: recommendation,
        timestamp: DateTime.now(),
      );
      _isLoading = false;
      notifyListeners();
    } catch (e) {
      _error = 'Failed to update soil type: $e';
      _isLoading = false;
      notifyListeners();
    }
  }
}