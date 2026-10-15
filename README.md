# ESP32 IoT Microclimate Weather Station


A low-cost weather station built on an ESP32. It measures temperature, humidity,
barometric pressure and rain, shows them on a **local web dashboard**, and streams
them to **Adafruit IO** over MQTT for remote viewing.

## Features

- **Dual-path telemetry** - local web page on port 80 (no internet needed) plus a cloud dashboard on Adafruit IO.
- **Non-blocking firmware** - all timing uses `millis()`; no `delay()` in the main loop.
- **No heap fragmentation** - the web page is built with `snprintf()` into one static 2048-byte buffer; no JSON library, no `String` objects.
- **Rain input done right** - the LM393 output is open-collector, so the pin uses `INPUT_PULLUP` to avoid false "rain" readings.
- **Graceful degradation** - if the BMP180 is missing the DHT11 supplies temperature; failed sensors are not published as zeros.
- **Secrets kept out of the code** - credentials live in a git-ignored `secrets.h`.

## Hardware

| Part | Purpose | Approx. cost |
|---|---|---|
| ESP32 dev board (30-pin NodeMCU style) | Microcontroller + Wi-Fi | $6-8 |
| DHT11 | Humidity (and backup temperature) | $1-2 |
| BMP180 | Pressure + temperature (I2C) | $1-2 |
| LM393 rain sensor module | Rain detection (digital out) | $1 |
| Breadboard + jumper wires | | $2 |

Wiring is in [docs/wiring.md](docs/wiring.md). Summary:

| ESP32 | Connects to |
|---|---|
| 3V3 / GND | VCC / GND of all three sensors |
| GPIO 4 | DHT11 data |
| GPIO 21 | BMP180 SDA |
| GPIO 22 | BMP180 SCL |
| GPIO 32 | LM393 DO |

## Software requirements

- [Arduino IDE](https://www.arduino.cc/en/software) 2.x (or `arduino-cli`)
- ESP32 board package (Espressif `esp32`, 2.0.14 or newer)
- Libraries (Library Manager):
  - **Adafruit IO Arduino** (with its dependencies: Adafruit MQTT Library, ArduinoHttpClient)
  - **Adafruit BMP085 Library**
  - **DHT sensor library** (+ Adafruit Unified Sensor)

## Setup

1. **Clone the repo**
   ```bash
   git clone https://github.com/<your-username>/<your-repo>.git
   cd <your-repo>/firmware/weather_station
   ```
2. **Create your credentials file**
   ```bash
   cp secrets.h.example secrets.h
   ```
   Edit `secrets.h` with your Wi-Fi name/password (2.4 GHz) and your Adafruit IO username and key.
   `secrets.h` is git-ignored - do not commit it.
3. **Create the Adafruit IO feeds** named `temperature`, `humidity`, `pressure` and `rain`
   (see [docs/adafruit-io-setup.md](docs/adafruit-io-setup.md)).
4. **Open `weather_station.ino`** in the Arduino IDE, select your ESP32 board and port, and upload.
5. Open the Serial Monitor at **115200 baud**. It prints the local IP address once connected.

## Usage

- **Local dashboard:** browse to `http://<esp32-ip>/` from any device on the same network. It refreshes every 2 seconds.
- **Cloud dashboard:** build a dashboard on Adafruit IO from the four feeds (gauges, line charts, an indicator for rain).

## Configuration

Everything tunable is at the top of `weather_station.ino`: pins, feed names, sensor interval and upload interval.

> **Adafruit IO rate limit:** the free tier allows 30 data points per minute across all feeds.
> Four feeds every 10 s is 24 points/min. If you add feeds or shorten the interval, recompute this.

## Project structure

```
.
├── firmware/weather_station/
│   ├── weather_station.ino     # main sketch
│   └── secrets.h.example       # credentials template (copy to secrets.h)
├── docs/
│   ├── architecture.md
│   ├── wiring.md
│   └── adafruit-io-setup.md
├── .github/                    # CI + issue/PR templates
├── CHANGELOG.md
├── CONTRIBUTING.md
├── SECURITY.md
└── LICENSE
```

## Known limitations

- DHT11 humidity accuracy is about +/-5 % RH; consider a BME280 for better precision.
- Always-on Wi-Fi draws roughly 80-160 mA; battery use needs deep sleep and/or solar.
- Resistive rain plates corrode over time with constant wetting.
- The local page is plain HTTP with no authentication - use it on a trusted network only.

## Roadmap

- BME280 support (temperature, humidity and pressure on one I2C device)
- Deep-sleep mode with rain-interrupt wake-up
- Pressure-trend (dP/dt) storm warning

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). Please read [SECURITY.md](SECURITY.md) before posting logs or screenshots.

## License

[MIT](LICENSE)
