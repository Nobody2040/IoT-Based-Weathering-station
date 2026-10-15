# Architecture

```
 DHT11 (GPIO 4)  BMP180 (I2C 21/22)  LM393 (GPIO 32)
        \               |                 /
         v              v                v
      +-----------------------------------------+
      |                 ESP32                   |
      |  millis() scheduler                     |
      |   - sensors every 2 s                   |
      |   - cloud upload every 10 s             |
      |  static pageBuffer[2048] (snprintf)     |
      +----------------+------------------------+
                       |                  |
                       v                  v
            Local web server (:80)   Adafruit IO (MQTT)
```

## Loop

`loop()` does three things and never blocks:

1. `server.handleClient()` - serve any pending HTTP request.
2. `io.run()` - keep the MQTT connection alive.
3. Two `millis()` timers - read sensors (2 s) and publish to Adafruit IO (10 s).

## Why static memory

Building HTML with `String` or a JSON library allocates and frees many small blocks, which fragments
the heap over days of uptime. The page here is rendered with `snprintf()` into a single global buffer,
and `handleRoot()` returns HTTP 500 instead of overflowing if the page ever outgrows it.

## Sensor fallbacks

| Situation | Behaviour |
|---|---|
| BMP180 missing | temperature from DHT11, pressure not published |
| DHT11 read fails | humidity not published; BMP180 temperature still used |
| Both temperature sources fail | temperature not published |
