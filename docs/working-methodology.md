# Working Methodology

1. **Initialization**  
   The ESP32 initializes the magnetometer, LCD, buzzer and nRF24L01 communication.

2. **Baseline calibration**  
   The system averages 100 consecutive magnetic-field samples while no mobile phone is intentionally present.

3. **Continuous monitoring**  
   The system targets a 75 Hz sampling rate.

4. **Deviation calculation**  
   The measured vector is compared with the baseline using the Euclidean deviation:

   `ΔB = sqrt((Bx-Bx0)^2 + (By-By0)^2 + (Bz-Bz0)^2)`

5. **Thresholding**  
   A configurable threshold is applied. The project paper reports an empirical threshold of 25 milligauss.

6. **Debounce / confirmation**  
   Five consecutive threshold-exceeding samples are required to reduce false positives caused by short environmental disturbances.

7. **Local alert**  
   A confirmed event activates the buzzer and updates the LCD.

8. **Wireless alert**  
   An alert packet containing node ID, detection status, timestamp and the measured deviation (ΔB) is sent to the central ESP32 receiver using nRF24L01.

9. **Central monitoring**  
   The receiver listens continuously and logs received detection information to the serial monitor.

## Implementation Note

The repository separates the sensor-driver function from the main detection algorithm. This allows the exact HMC5883L-compatible sensor/module library used in the physical prototype to be integrated without rewriting the detection logic.
