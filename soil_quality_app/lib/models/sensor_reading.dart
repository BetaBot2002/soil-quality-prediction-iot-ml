import 'package:intl/intl.dart';

class SensorReading {
  final double temperature;
  final double humidity;
  final double moisture;
  final double nitrogen;
  final double phosphorus;
  final double potassium;
  final int soilType;
  final int cropType;
  final String recommendedFertilizer;
  final DateTime timestamp;

  SensorReading({
    required this.temperature,
    required this.humidity,
    required this.moisture,
    required this.nitrogen,
    required this.phosphorus,
    required this.potassium,
    required this.soilType,
    required this.cropType,
    required this.recommendedFertilizer,
    required this.timestamp,
  });

  factory SensorReading.fromJson(Map<String, dynamic> json) {
    return SensorReading(
      temperature: json['temperature']?.toDouble() ?? 25.0,
      humidity: json['humidity']?.toDouble() ?? 60.0,
      moisture: json['moisture']?.toDouble() ?? 35.0,
      nitrogen: json['nitrogen']?.toDouble() ?? 20.0,
      phosphorus: json['phosphorus']?.toDouble() ?? 15.0,
      potassium: json['potassium']?.toDouble() ?? 25.0,
      soilType: json['soil_type'] ?? 1,
      cropType: json['crop_type'] ?? 0,
      recommendedFertilizer: json['recommended_fertilizer'] ?? 'No recommendation yet',
      timestamp: json['timestamp'] != null
          ? DateTime.parse(json['timestamp'])
          : DateTime.now(),
    );
  }

  factory SensorReading.defaultValues() {
    return SensorReading(
      temperature: 25.0,
      humidity: 60.0,
      moisture: 35.0,
      nitrogen: 20.0,
      phosphorus: 15.0,
      potassium: 25.0,
      soilType: 1,
      cropType: 0,
      recommendedFertilizer: 'No recommendation yet',
      timestamp: DateTime.now(),
    );
  }

  String get formattedTimestamp {
    return DateFormat('yyyy-MM-dd HH:mm:ss').format(timestamp);
  }

  Map<String, dynamic> toJson() {
    return {
      'temperature': temperature,
      'humidity': humidity,
      'moisture': moisture,
      'nitrogen': nitrogen,
      'phosphorus': phosphorus,
      'potassium': potassium,
      'soil_type': soilType,
      'crop_type': cropType,
      'recommended_fertilizer': recommendedFertilizer,
      'timestamp': formattedTimestamp,
    };
  }
}