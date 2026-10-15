# Adafruit IO setup

1. Create a free account at <https://io.adafruit.com>.
2. Open **My Key** and copy your username and active key into `secrets.h`.
3. Create four feeds under **Feeds -> New Feed** (the name must match the feed key used in the sketch):

   | Feed | Data | Unit |
   |---|---|---|
   | `temperature` | float | degrees C |
   | `humidity` | number | % RH |
   | `pressure` | number | hPa |
   | `rain` | 0 = dry, 1 = rain | - |

4. Create a dashboard and add blocks: gauges for temperature, humidity and pressure, a line chart, and an indicator for `rain`.
5. Free tier limit: **30 data points per minute**. The default configuration sends 24/min.

> Treat your Adafruit IO key like a password. If it is ever exposed (committed, pasted in an issue, screenshotted), regenerate it from **My Key**.
