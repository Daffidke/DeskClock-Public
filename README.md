# Smart Desk Clock

An embedded desk clock and ambient environmental monitor engineered for the ESP32-C6 SuperMini microcontroller. The device combines local sensing, networked time synchronization, automated meteorological forecasting, and e-ink display output designed for ultra-low power consumption.

---

## Hardware Specifications

* Microcontroller: ESP32-C6 SuperMini
* Display: 2.9-inch SPI E-Paper / E-Ink panel
* Sensors: 
  * SHT30: Precision digital temperature and relative humidity sensor
  * SGP30: Multi-pixel gas sensor for Indoor Air Quality (eCO2 and TVOC)
* User Input: 3-button physical navigation interface (Up, Select, Down)
* Audio Output: Passive buzzer
* Charging: Integrated Li-Po battery management via TP4056 charging module

---

## Architecture and System Features

### Display and User Interface
* Partial and full updates for a relatively fast interface.
* Main Dashboard: Displays RTC time, calendar date, active alarm/timer flags, and live indoor climate metrics (temperature, humidity, air quality).
* 5-Hour Weather View: Hourly projections featuring temperature, wind speed, condition codes, and day/night indicators.
* System Menu: Complete on-device configuration tree for runtime adjustments without requiring a reflash:
  * Manual Date & Time Adjustment
  * Configurable Alarm Scheduling
  * Precision Countdown Timer
  * Network / Wi-Fi Access Point Provisioning
  * Factory Reset and NVS Partition Scrubbing

### Networking and Localization
* SoftAP Captive Portal: Automatically serves a responsive configuration interface for scanning local SSIDs and submitting WPA credentials without terminal access.
* Automated Geolocation: Automatically resolves IP-based latitude, longitude, and city names upon establishing a Wi-Fi connection.
* Meteorological Engine: Polls hourly forecast endpoints from Open-Meteo using non-blocking state progression.
* Network Time Protocol (NTP): Periodically aligns the ESP32 hardware real-time clock to atomic UTC pools with automatic POSIX timezone and daylight saving rules.

---

## To Be Added

### WS2812B Dynamic LED Strip
* Dynamic lighting based on weather and daytime.
* Turns on only when charging to keep the power draw as low as possible.

### Deep Sleep & Battery Management
* The device returns from sleep whenever something on the screen needs to be refreshed or an alarm/timer is triggered.
* Battery percentage will be displayed on all the pages.

---

The project is being developed in PlatformIO IDE using the Arduino Framework.
