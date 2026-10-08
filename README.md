# Mobile Phone Detection System

A low-cost embedded mobile phone detection prototype based on an ESP32, HMC5883L-compatible magnetometer, nRF24L01 wireless transceiver, 16×2 LCD, and buzzer.

## Project Overview

This project is intended for restricted environments such as examination halls, secure meeting rooms, and other locations where unauthorized mobile phone use needs to be detected.

Each detection node establishes a baseline magnetic-field vector and continuously measures the magnetic field. A deviation from the calibrated baseline is processed using a threshold and debounce mechanism. A confirmed event activates the local buzzer and LCD alert and is transmitted to a central ESP32 receiver through nRF24L01. The receiver code supports multiple node IDs/pipes for a small multi-zone demonstration.

## Main Hardware

- ESP32 development board
- HMC5883L / compatible 3-axis magnetometer
- nRF24L01 2.4 GHz transceiver
- 16×2 LCD
- Piezoelectric buzzer
- Supporting wiring, resistors and power supply

## Working Principle

At startup, the detection node averages 100 magnetometer samples to create a baseline:

`B0 = (Bx0, By0, Bz0)`

For each monitoring sample, the deviation is calculated as:

`ΔB = sqrt((Bx-Bx0)^2 + (By-By0)^2 + (Bz-Bz0)^2)`

A detection is considered when the deviation exceeds the configured threshold. Five consecutive threshold-exceeding samples are required as a debounce/filter condition.

After confirmation:

1. The buzzer is activated.
2. The LCD shows the alert.
3. A structured alert packet is sent through nRF24L01.
4. The central ESP32 receiver displays the received node alert.

## Repository Structure

```text
Mobile-Phone-Detection-System/
├── README.md
├── LICENSE
├── CHANGELOG.md
├── CITATION.cff
├── CONTRIBUTING.md
├── CODE_OF_CONDUCT.md
├── .github/
├── src/
│   ├── mobile_detection_node/
│   │   └── mobile_detection_node.ino
│   └── central_receiver/
│       └── central_receiver.ino
├── hardware/
│   ├── components-list.md
│   └── pin-connections.md
├── docs/
│   ├── architecture.md
│   ├── repository-guide.md
│   └── working-methodology.md
└── results/
    └── experimental-results.md
```

## Reported Experimental Results

According to the associated project paper:

| Parameter | Reported result |
|---|---:|
| Active phone detection range | Up to 45 cm |
| Idle phone detection range | Approximately 15 cm |
| False-positive rate | 2.4% |
| Wireless latency | 8–12 ms |
| Continuous test duration | 8 hours |

These figures were obtained in a controlled laboratory environment.

## Build and Upload (Arduino IDE)

1. Install the **ESP32 board package** (Espressif) via Boards Manager. Core 2.x and 3.x are both supported.
2. Install these libraries via Library Manager:
   - `RF24` by TMRh20
   - `LiquidCrystal` by Arduino
   - `Wire` and `SPI` come with the ESP32 core.
3. Open `src/mobile_detection_node/mobile_detection_node.ino`, select your ESP32 board and upload it to the **detection node**.
4. Open `src/central_receiver/central_receiver.ino` and upload it to the **second ESP32** (receiver). Open Serial Monitor at **115200 baud**. The receiver listens for node addresses `1NODE` through `6NODE`.
5. Power the detection node with no phone nearby until the LCD shows "Calibration OK".

The magnetometer is read through a built-in direct-I2C HMC5883L driver, so no sensor library is needed. The nRF24L01 radio uses SPI; default ESP32 VSPI pins are listed in `hardware/pin-connections.md`. Many boards sold as "GY-271" actually carry a QMC5883L chip (I2C address 0x0D). If the node shows `SENSOR ERROR`, check wiring first, then check whether your module is a QMC5883L; its driver would need to replace `initMagnetometer()` and `readMagnetometer()`.

> Pin assignments may need adjustment for the exact modules used in your prototype.

## Limitations

- The sensor measures **magnetic-field disturbance**, not radio signals. Any magnetic or ferrous object (magnets, speakers, keys, laptops, motors) can trigger it.
- The baseline is taken once at start-up. Moving the sensor or large metal objects afterwards can cause false alerts; restart to recalibrate.
- The results below come from the authors' controlled laboratory tests and may not hold in other environments.
- This is a prototype, not a certified security or invigilation product.

## Authors

- Vishva Shedge
- Shubham Lashkar
- Swaraj Pawar
- Aaditya Shinde

Guidance: Prof. A. N. Dubey

Department of Electronics and Telecommunication Engineering  
JSPM's Rajarshi Shahu College of Engineering (Polytechnic), Tathawade, Pune

## Reference Paper

**Mobile Phone Detection System Using ESP32, HMC5883L, NRF24L01, LCD Display, and Buzzer**

International Journal of Scientific Research & Engineering Trends, Volume 12, Issue 2, Mar-Apr 2026.

## Future Scope

- Machine-learning-based classification
- Lower false-positive rate
- Distinguishing different electronic devices
- Cloud-based monitoring
- Mobile alert notifications

## Project Status

**Repository status:** prototype / academic project.

The repository contains the detection-node and central-receiver firmware, wiring documentation, methodology notes, and reported test results. The firmware is structured for the stated hardware, but the exact physical module variants should be validated on the bench before deployment.

## Safety and Responsible Use

This project is intended for academic prototyping and authorized security testing. It detects environmental magnetic-field changes and does not decode, intercept, or store mobile communications. Do not deploy it as a production security system without validating the sensor, calibration method, interference tolerance, and applicable rules for the intended environment.

## Attribution

The methodology and reported experimental results are documented in the associated IJSRET paper listed above. The paper reports the use of ESP32, HMC5883L-compatible sensing, nRF24L01, a 16×2 LCD, and a buzzer, with controlled laboratory tests.
