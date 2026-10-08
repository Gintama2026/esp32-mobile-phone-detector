# Pin Connections

## ESP32 ↔ HMC5883L-compatible Magnetometer

| HMC5883L | ESP32 |
|---|---|
| VCC | 3.3 V (confirm module requirements) |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

## ESP32 ↔ nRF24L01

Using the ESP32 VSPI pins:

| nRF24L01 | ESP32 |
|---|---|
| VCC | 3.3 V |
| GND | GND |
| CE | GPIO 4 |
| CSN | GPIO 5 |
| SCK | GPIO 18 |
| MISO | GPIO 19 |
| MOSI | GPIO 23 |

## ESP32 ↔ Buzzer

| Buzzer | ESP32 |
|---|---|
| Signal | GPIO 27 |
| GND | GND |

## 16×2 Parallel LCD

The example code uses:

- RS → GPIO 14
- E → GPIO 13
- D4 → GPIO 26
- D5 → GPIO 25
- D6 → GPIO 33
- D7 → GPIO 32

Additional LCD wiring (standard HD44780 16×2 module):

| LCD pin | Connect to |
|---|---|
| VSS | GND |
| VDD | 5 V (VIN) on most modules; check your module |
| V0 | Middle pin of a 10 kΩ contrast potentiometer (ends to 5 V and GND) |
| RW | GND |
| A (backlight +) | 5 V through ~220 Ω resistor |
| K (backlight −) | GND |

Adjust these connections to match your actual prototype.

## Notes

- Place a **10 µF capacitor** across the nRF24L01 VCC and GND pins, close to the module. Without it, the radio is often unreliable.
- The nRF24L01 must run on **3.3 V only**.
- Many buzzer modules work with `tone()`; a plain passive piezo may need a series resistor.

> Always verify the voltage requirements and pinout of the exact modules before powering the circuit.
