# System Architecture

```text
+---------------------+
| Detection Node      |
| ESP32                |
+----------+----------+
           | I2C
           v
+---------------------+
| HMC5883L-compatible  |
| Magnetometer         |
+----------+----------+
           | field samples
           v
+---------------------+
| Baseline + Delta     |
| Threshold + Debounce |
+----------+----------+
           | confirmed alert
        +--+-----------+
        |              |
        v              v
     LCD/Buzzer      nRF24L01
                         |
                    2.4 GHz link
                         |
                         v
              +---------------------+
              | Central ESP32       |
              | Multi-node Receiver |
              +---------------------+
```

The current receiver supports six logical radio pipes (`1NODE`–`6NODE`). Each detection node selects its ID in the sketch and uses the corresponding pipe address.
