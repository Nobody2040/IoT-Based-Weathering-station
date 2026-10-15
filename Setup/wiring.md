# Wiring

All sensors run from the ESP32's regulated **3.3 V** rail and share a common ground.

| ESP32 pin | Component pin | Notes |
|---|---|---|
| 3V3 | VCC of DHT11, BMP180, LM393 | 3.3 V only - do not use 5 V with the BMP180 |
| GND | GND of all sensors | common ground |
| GPIO 4 | DHT11 DATA | use a module with a 10 k pull-up, or add one to 3V3 |
| GPIO 21 | BMP180 SDA | I2C data |
| GPIO 22 | BMP180 SCL | I2C clock |
| GPIO 32 | LM393 DO | firmware enables the internal pull-up (`INPUT_PULLUP`) |

```
ESP32
 |-- 3V3 ----------------> VCC  (DHT11, BMP180, LM393)
 |-- GND ----------------> GND  (DHT11, BMP180, LM393)
 |-- GPIO 4  ------------> DHT11 DATA
 |-- GPIO 21 (SDA) ------> BMP180 SDA
 |-- GPIO 22 (SCL) ------> BMP180 SCL
 `-- GPIO 32 ------------> LM393 DO
```

## Notes

- **Rain logic:** DO is HIGH when dry and LOW when water bridges the plate (active LOW).
- **BMP180 address:** `0x77` (fixed).
- If the BMP180 is not detected the Serial Monitor says so and temperature falls back to the DHT11.
