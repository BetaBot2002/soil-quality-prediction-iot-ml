import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../providers/sensor_provider.dart';
import '../widgets/sensor_card.dart';

class DashboardScreen extends StatefulWidget {
  const DashboardScreen({Key? key}) : super(key: key);

  @override
  State<DashboardScreen> createState() => _DashboardScreenState();
}

class _DashboardScreenState extends State<DashboardScreen> {
  @override
  void initState() {
    super.initState();
    // Fetch data when screen loads
    Future.delayed(Duration.zero, () {
      Provider.of<SensorProvider>(context, listen: false).fetchSensorData();
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: SafeArea(
        child: Consumer<SensorProvider>(
          builder: (ctx, sensorProvider, _) {
            final sensorData = sensorProvider.sensorData;
            
            return RefreshIndicator(
              onRefresh: () => sensorProvider.fetchSensorData(),
              child: CustomScrollView(
                slivers: [
                  SliverAppBar(
                    floating: true,
                    pinned: true,
                    title: ShaderMask(
                      shaderCallback: (bounds) => LinearGradient(
                        colors: [Theme.of(context).primaryColor, Colors.teal],
                      ).createShader(bounds),
                      child: const Text(
                        'Soil Quality Dashboard',  // Shortened title
                        style: TextStyle(fontWeight: FontWeight.bold),
                      ),
                    ),
                    actions: [
                      IconButton(
                        icon: const Icon(Icons.refresh),
                        onPressed: () => sensorProvider.fetchSensorData(),
                      ),
                    ],
                  ),
                  SliverPadding(
                    padding: const EdgeInsets.all(16.0),
                    sliver: SliverToBoxAdapter(
                      child: sensorProvider.isLoading
                          ? const Center(child: CircularProgressIndicator())
                          : Column(
                              children: [
                                // Temperature and Humidity Row
                                Row(
                                  children: [
                                    Expanded(
                                      child: SensorCard(
                                        title: 'Temperature',
                                        value: sensorData.temperature.toString(),
                                        unit: '°C',
                                        icon: Icons.thermostat,
                                        isAlert: sensorData.temperature > 35,
                                      ),
                                    ),
                                    const SizedBox(width: 8),
                                    Expanded(
                                      child: SensorCard(
                                        title: 'Humidity',
                                        value: sensorData.humidity.toString(),
                                        unit: '%',
                                        icon: Icons.water_drop,
                                        isAlert: sensorData.humidity < 30,
                                      ),
                                    ),
                                  ],
                                ),
                                const SizedBox(height: 8),
                                
                                // Soil Moisture and Soil Type Row
                                Row(
                                  children: [
                                    Expanded(
                                      child: SensorCard(
                                        title: 'Soil Moisture',
                                        value: sensorData.moisture.toString(),
                                        unit: '%',
                                        icon: Icons.water,
                                        isAlert: sensorData.moisture < 20,
                                      ),
                                    ),
                                    const SizedBox(width: 8),
                                    Expanded(
                                      child: GestureDetector(
                                        onTap: () => _showSoilTypeModal(context),
                                        child: Card(
                                          elevation: 4,
                                          shape: RoundedRectangleBorder(
                                            borderRadius: BorderRadius.circular(12),
                                          ),
                                          child: Container(
                                            decoration: BoxDecoration(
                                              borderRadius: BorderRadius.circular(12),
                                              gradient: LinearGradient(
                                                begin: Alignment.topLeft,
                                                end: Alignment.bottomRight,
                                                colors: [Colors.grey[900]!, Colors.grey[850]!],
                                              ),
                                            ),
                                            child: Padding(
                                              padding: const EdgeInsets.all(16.0),
                                              child: Column(
                                                crossAxisAlignment: CrossAxisAlignment.start,
                                                children: [
                                                  Row(
                                                    children: [
                                                      Icon(
                                                        Icons.layers,
                                                        color: Theme.of(context).primaryColor,
                                                        size: 20,
                                                      ),
                                                      const SizedBox(width: 8),
                                                      const Text(
                                                        'Soil Type',
                                                        style: TextStyle(
                                                          color: Colors.white,
                                                          fontWeight: FontWeight.bold,
                                                          fontSize: 14,
                                                        ),
                                                      ),
                                                    ],
                                                  ),
                                                  const SizedBox(height: 12),
                                                  Row(
                                                    mainAxisAlignment: MainAxisAlignment.spaceBetween,
                                                    children: [
                                                      Text(
                                                        sensorProvider.getSoilName(sensorData.soilType),
                                                        style: const TextStyle(
                                                          color: Colors.white,
                                                          fontWeight: FontWeight.bold,
                                                          fontSize: 20,
                                                        ),
                                                      ),
                                                      const Icon(
                                                        Icons.edit,
                                                        color: Colors.white54,
                                                        size: 16,
                                                      ),
                                                    ],
                                                  ),
                                                ],
                                              ),
                                            ),
                                          ),
                                        ),
                                      ),
                                    ),
                                  ],
                                ),
                                const SizedBox(height: 8),
                                
                                // NPK Row
                                Column(
                                  children: [
                                    Row(
                                      children: [
                                        Expanded(
                                          flex: 1,
                                          child: SensorCard(
                                            title: 'Nitrogen',
                                            value: sensorData.nitrogen.toString(),
                                            unit: 'mg/kg',
                                            icon: Icons.science,
                                            isAlert: sensorData.nitrogen < 10,
                                          ),
                                        ),
                                        const SizedBox(width: 8),
                                        Expanded(
                                          flex: 1,
                                          child: SensorCard(
                                            title: 'Phosphorus',
                                            value: sensorData.phosphorus.toString(),
                                            unit: 'mg/kg',
                                            icon: Icons.science,
                                            isAlert: sensorData.phosphorus < 10,
                                          ),
                                        ),
                                      ],
                                    ),
                                    const SizedBox(height: 8),
                                    SensorCard(
                                      title: 'Potassium',
                                      value: sensorData.potassium.toString(),
                                      unit: 'mg/kg',
                                      icon: Icons.science,
                                      isAlert: sensorData.potassium < 10,
                                    ),
                                  ],
                                ),
                                const SizedBox(height: 16),
                                
                                // Fertilizer Recommendation Card
                                Card(
                                  elevation: 4,
                                  shape: RoundedRectangleBorder(
                                    borderRadius: BorderRadius.circular(12),
                                  ),
                                  child: Container(
                                    width: double.infinity,
                                    decoration: BoxDecoration(
                                      borderRadius: BorderRadius.circular(12),
                                      gradient: LinearGradient(
                                        begin: Alignment.topLeft,
                                        end: Alignment.bottomRight,
                                        colors: [Colors.grey[900]!, Colors.grey[850]!],
                                      ),
                                    ),
                                    child: Padding(
                                      padding: const EdgeInsets.all(16.0),
                                      child: Column(
                                        crossAxisAlignment: CrossAxisAlignment.start,
                                        children: [
                                          Row(
                                            children: [
                                              Icon(
                                                Icons.eco,
                                                color: Theme.of(context).primaryColor,
                                                size: 20,
                                              ),
                                              const SizedBox(width: 8),
                                              const Text(
                                                'Recommended Fertilizer',
                                                style: TextStyle(
                                                  color: Colors.white,
                                                  fontWeight: FontWeight.bold,
                                                  fontSize: 14,
                                                ),
                                              ),
                                            ],
                                          ),
                                          const SizedBox(height: 12),
                                          Text(
                                            sensorData.recommendedFertilizer,
                                            style: TextStyle(
                                              color: Theme.of(context).primaryColor,
                                              fontWeight: FontWeight.bold,
                                              fontSize: 24,
                                            ),
                                          ),
                                        ],
                                      ),
                                    ),
                                  ),
                                ),
                                const SizedBox(height: 16),
                                
                                // Last Updated and Crop Type Row
                                Row(
                                  children: [
                                    // Last Updated
                                    Expanded(
                                      flex: 3,
                                      child: Card(
                                        elevation: 4,
                                        shape: RoundedRectangleBorder(
                                          borderRadius: BorderRadius.circular(12),
                                        ),
                                        child: Container(
                                          decoration: BoxDecoration(
                                            borderRadius: BorderRadius.circular(12),
                                            gradient: LinearGradient(
                                              begin: Alignment.topLeft,
                                              end: Alignment.bottomRight,
                                              colors: [Colors.grey[900]!, Colors.grey[850]!],
                                            ),
                                          ),
                                          child: Padding(
                                            padding: const EdgeInsets.all(16.0),
                                            child: Column(
                                              crossAxisAlignment: CrossAxisAlignment.start,
                                              children: [
                                                Row(
                                                  children: [
                                                    Icon(
                                                      Icons.access_time,
                                                      color: Theme.of(context).primaryColor,
                                                      size: 20,
                                                    ),
                                                    const SizedBox(width: 8),
                                                    const Text(
                                                      'Last Updated',
                                                      style: TextStyle(
                                                        color: Colors.white,
                                                        fontWeight: FontWeight.bold,
                                                        fontSize: 14,
                                                      ),
                                                    ),
                                                  ],
                                                ),
                                                const SizedBox(height: 12),
                                                Text(
                                                  sensorData.formattedTimestamp,
                                                  style: const TextStyle(
                                                    color: Colors.white,
                                                    fontSize: 16,
                                                  ),
                                                ),
                                              ],
                                            ),
                                          ),
                                        ),
                                      ),
                                    ),
                                    const SizedBox(width: 8),
                                    
                                    // Crop Type
                                    Expanded(
                                      flex: 4,
                                      child: Card(
                                        elevation: 4,
                                        shape: RoundedRectangleBorder(
                                          borderRadius: BorderRadius.circular(12),
                                        ),
                                        child: Container(
                                          decoration: BoxDecoration(
                                            borderRadius: BorderRadius.circular(12),
                                            gradient: LinearGradient(
                                              begin: Alignment.topLeft,
                                              end: Alignment.bottomRight,
                                              colors: [Colors.grey[900]!, Colors.grey[850]!],
                                            ),
                                          ),
                                          child: Padding(
                                            padding: const EdgeInsets.all(16.0),
                                            child: Column(
                                              crossAxisAlignment: CrossAxisAlignment.start,
                                              children: [
                                                Row(
                                                  children: [
                                                    Icon(
                                                      Icons.grass,
                                                      color: Theme.of(context).primaryColor,
                                                      size: 20,
                                                    ),
                                                    const SizedBox(width: 8),
                                                    const Text(
                                                      'Crop Type',
                                                      style: TextStyle(
                                                        color: Colors.white,
                                                        fontWeight: FontWeight.bold,
                                                        fontSize: 14,
                                                      ),
                                                    ),
                                                  ],
                                                ),
                                                const SizedBox(height: 12),
                                                Row(
                                                  children: [
                                                    Expanded(
                                                      child: DropdownButton<int>(
                                                        isExpanded: true,
                                                        dropdownColor: Colors.grey[850],
                                                        value: sensorData.cropType,
                                                        items: sensorProvider.cropTypes
                                                            .map((crop) => DropdownMenuItem<int>(
                                                                  value: crop['id'],
                                                                  child: Text(
                                                                    crop['name'],
                                                                    style: const TextStyle(color: Colors.white),
                                                                    overflow: TextOverflow.ellipsis,
                                                                  ),
                                                                ))
                                                            .toList(),
                                                        onChanged: (value) {},
                                                        style: const TextStyle(color: Colors.white),
                                                        underline: Container(
                                                          height: 1,
                                                          color: Colors.white24,
                                                        ),
                                                      ),
                                                    ),
                                                    const SizedBox(width: 8),
                                                    SizedBox(
                                                      width: 80, // Fixed width for the button
                                                      child: ElevatedButton(
                                                        onPressed: () => _showCropTypeModal(context),
                                                        style: ElevatedButton.styleFrom(
                                                          backgroundColor: Theme.of(context).primaryColor,
                                                          shape: RoundedRectangleBorder(
                                                            borderRadius: BorderRadius.circular(20),
                                                          ),
                                                        ),
                                                        child: const Icon(
                                                        Icons.edit,
                                                        color: Colors.white54,
                                                        size: 16,
                                                      ),
                                                      ),
                                                    ),
                                                  ],
                                                ),
                                              ],
                                            ),
                                          ),
                                        ),
                                      ),
                                    ),
                                  ],
                                ),
                              ],
                            ),
                    ),
                  ),
                ],
              ),
            );
          },
        ),
      ),
    );
  }

  void _showSoilTypeModal(BuildContext context) {
    final sensorProvider = Provider.of<SensorProvider>(context, listen: false);
    int selectedSoilType = sensorProvider.sensorData.soilType;

    showModalBottomSheet(
      context: context,
      backgroundColor: Colors.grey[900],
      shape: const RoundedRectangleBorder(
        borderRadius: BorderRadius.vertical(top: Radius.circular(20)),
      ),
      builder: (ctx) => StatefulBuilder(
        builder: (context, setState) => Padding(
          padding: const EdgeInsets.all(20.0),
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              const Text(
                'Change Soil Type',
                style: TextStyle(
                  color: Colors.white,
                  fontWeight: FontWeight.bold,
                  fontSize: 18,
                ),
              ),
              const SizedBox(height: 20),
              ListView.builder(
                shrinkWrap: true,
                itemCount: sensorProvider.soilTypes.length,
                itemBuilder: (context, index) {
                  final soil = sensorProvider.soilTypes[index];
                  return RadioListTile<int>(
                    title: Text(
                      soil['name'],
                      style: const TextStyle(color: Colors.white),
                    ),
                    value: soil['id'],
                    groupValue: selectedSoilType,
                    onChanged: (value) {
                      setState(() {
                        selectedSoilType = value!;
                      });
                    },
                    activeColor: Theme.of(context).primaryColor,
                  );
                },
              ),
              const SizedBox(height: 20),
              Row(
                mainAxisAlignment: MainAxisAlignment.end,
                children: [
                  TextButton(
                    onPressed: () => Navigator.pop(context),
                    child: const Text('Cancel'),
                  ),
                  const SizedBox(width: 8),
                  ElevatedButton(
                    onPressed: () {
                      sensorProvider.updateSoilType(selectedSoilType);
                      Navigator.pop(context);
                    },
                    style: ElevatedButton.styleFrom(
                      backgroundColor: Theme.of(context).primaryColor,
                    ),
                    child: const Text('Save'),
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }

  void _showCropTypeModal(BuildContext context) {
    final sensorProvider = Provider.of<SensorProvider>(context, listen: false);
    int selectedCropType = sensorProvider.sensorData.cropType;

    showModalBottomSheet(
      context: context,
      backgroundColor: Colors.grey[900],
      shape: const RoundedRectangleBorder(
        borderRadius: BorderRadius.vertical(top: Radius.circular(20)),
      ),
      builder: (ctx) => StatefulBuilder(
        builder: (context, setState) => Padding(
          padding: const EdgeInsets.all(20.0),
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              const Text(
                'Change Crop Type',
                style: TextStyle(
                  color: Colors.white,
                  fontWeight: FontWeight.bold,
                  fontSize: 18,
                ),
              ),
              const SizedBox(height: 20),
              SizedBox(
                height: 300,
                child: ListView.builder(
                  shrinkWrap: true,
                  itemCount: sensorProvider.cropTypes.length,
                  itemBuilder: (context, index) {
                    final crop = sensorProvider.cropTypes[index];
                    return RadioListTile<int>(
                      title: Text(
                        crop['name'],
                        style: const TextStyle(color: Colors.white),
                      ),
                      value: crop['id'],
                      groupValue: selectedCropType,
                      onChanged: (value) {
                        setState(() {
                          selectedCropType = value!;
                        });
                      },
                      activeColor: Theme.of(context).primaryColor,
                    );
                  },
                ),
              ),
              const SizedBox(height: 20),
              Row(
                mainAxisAlignment: MainAxisAlignment.end,
                children: [
                  TextButton(
                    onPressed: () => Navigator.pop(context),
                    child: const Text('Cancel'),
                  ),
                  const SizedBox(width: 8),
                  ElevatedButton(
                    onPressed: () {
                      sensorProvider.updateCropType(selectedCropType);
                      Navigator.pop(context);
                    },
                    style: ElevatedButton.styleFrom(
                      backgroundColor: Theme.of(context).primaryColor,
                    ),
                    child: const Text('Save'),
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }
}