import 'dart:convert';
import 'package:http/http.dart' as http;
import '../models/sensor_reading.dart';

class ApiService {
  // ThingSpeak API configuration
  static const String channelId = "2989083";
  static const String readApiKey = "R6LCX8EH46PUEC5Q";
  static const String thingspeakEndpoint = 
      "https://api.thingspeak.com/channels/$channelId/feeds/last.json?api_key=$readApiKey";
  
  // Server API endpoint (if you want to connect to your Flask server)
  static const String serverEndpoint = "http://192.168.79.100:5000"; // Update with your server's IP

  // Fetch data from ThingSpeak
  Future<SensorReading> fetchSensorData() async {
    try {
      final response = await http.get(Uri.parse(thingspeakEndpoint));
      
      if (response.statusCode == 200) {
        final Map<String, dynamic> thingspeakData = json.decode(response.body);
        
        // Map ThingSpeak fields to our application's data structure
        // Field1: Humidity, Field2: Temperature, Field3: Moisture, 
        // Field4: Nitrogen, Field5: Phosphorus, Field6: Potassium
        final data = {
          'humidity': double.tryParse(thingspeakData['field1'] ?? '0') ?? 60.0,
          'temperature': double.tryParse(thingspeakData['field2'] ?? '0') ?? 25.0,
          'moisture': double.tryParse(thingspeakData['field3'] ?? '0') ?? 35.0,
          'nitrogen': double.tryParse(thingspeakData['field4'] ?? '0') ?? 20.0,
          'phosphorus': double.tryParse(thingspeakData['field5'] ?? '0') ?? 15.0,
          'potassium': double.tryParse(thingspeakData['field6'] ?? '0') ?? 25.0,
          'soil_type': 1, // Default to Loamy
          'crop_type': 0, // Default to Maize
          'recommended_fertilizer': 'Fetching recommendation...',
          'timestamp': thingspeakData['created_at'] ?? DateTime.now().toIso8601String(),
        };
        
        // Get fertilizer recommendation from server
        await predictFertilizer(data);
        
        return SensorReading.fromJson(data);
      } else {
        throw Exception('Failed to load data from ThingSpeak');
      }
    } catch (e) {
      print('Error fetching sensor data: $e');
      return SensorReading.defaultValues();
    }
  }

  // Get fertilizer recommendation from server
  Future<void> predictFertilizer(Map<String, dynamic> data) async {
    try {
      // If you have a server running, you can use this to get recommendations
      // Otherwise, you can implement a simplified prediction algorithm in the app
      final response = await http.post(
        Uri.parse('$serverEndpoint/sensor-data'),
        headers: {'Content-Type': 'application/json'},
        body: json.encode(data),
      );
      
      if (response.statusCode == 200) {
        final responseData = json.decode(response.body);
        data['recommended_fertilizer'] = responseData['recommended_fertilizer'];
      } else {
        // Fallback to a simple recommendation logic
        simplePrediction(data);
      }
    } catch (e) {
      print('Error predicting fertilizer: $e');
      // Fallback to a simple recommendation logic
      simplePrediction(data);
    }
  }

  // Simple prediction logic as fallback
  void simplePrediction(Map<String, dynamic> data) {
    final soilType = data['soil_type'];
    final cropType = data['crop_type'];
    final nitrogen = data['nitrogen'];
    
    // Very simplified logic - in a real app, you would implement a more sophisticated algorithm
    if (nitrogen < 10) {
      data['recommended_fertilizer'] = 'Urea';
    } else if (soilType == 0) { // Sandy
      data['recommended_fertilizer'] = 'DAP';
    } else if (cropType == 0) { // Maize
      data['recommended_fertilizer'] = 'NPK';
    } else {
      data['recommended_fertilizer'] = 'Organic Fertilizer';
    }
  }

  // Update crop type
  Future<String> updateCropType(int cropType) async {
    try {
      final response = await http.post(
        Uri.parse('$serverEndpoint/update-crop'),
        headers: {'Content-Type': 'application/json'},
        body: json.encode({'crop_type': cropType}),
      );
      
      if (response.statusCode == 200) {
        final responseData = json.decode(response.body);
        return responseData['recommended_fertilizer'];
      } else {
        throw Exception('Failed to update crop type');
      }
    } catch (e) {
      print('Error updating crop type: $e');
      return 'Error updating crop type';
    }
  }

  // Update soil type
  Future<String> updateSoilType(int soilType) async {
    try {
      final response = await http.post(
        Uri.parse('$serverEndpoint/update-soil'),
        headers: {'Content-Type': 'application/json'},
        body: json.encode({'soil_type': soilType}),
      );
      
      if (response.statusCode == 200) {
        final responseData = json.decode(response.body);
        return responseData['recommended_fertilizer'];
      } else {
        throw Exception('Failed to update soil type');
      }
    } catch (e) {
      print('Error updating soil type: $e');
      return 'Error updating soil type';
    }
  }
}