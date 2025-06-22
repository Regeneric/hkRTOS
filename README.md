# Pico Weather & Air Quality Station

This is a comprehensive environmental monitoring station built using a Raspberry Pi Pico (RP2040/RP2350) and a wide array of sensors. The system is built on the FreeRTOS real-time operating system to handle multiple tasks concurrently, providing a detailed picture of both outdoor weather conditions and indoor air quality.

## Key Features

* **Multi-Sensor Data Aggregation:** Collects data from over 10 different sensors simultaneously.
* **Weather Station:** Measures outdoor temperature, humidity, barometric pressure, and dew point.
* **Air Quality Monitoring:**
    * **Outdoor (AQI):** Measures PM2.5/PM10 particulate matter and other criteria pollutants (O₃, NO₂, CO, SO₂) to calculate a real-world Air Quality Index.
    * **Indoor (IAQ):** Measures indoor-specific pollutants like Total Volatile Organic Compounds (TVOC), equivalent CO₂ (eCO2), and true CO₂ to assess the health of the indoor environment.
* **Robust RTOS Architecture:** Uses FreeRTOS in Symmetric Multi-Processing (SMP) mode to efficiently manage all tasks across both cores of the RP2040/RP2350.
* **Data Processing:** Implements weighted averaging algorithms to produce a single, more accurate reading from multiple redundant sensors.
* **Multi-Bus Communication:** Interfaces with sensors using I2C, UART, and One-Wire protocols.

## System Architecture

The project is built on a decoupled, multi-tasking architecture designed for stability and scalability.

* **Sensor Tasks:** Each major sensor or group of sensors is managed by its own dedicated FreeRTOS task. This isolates the logic for each piece of hardware and allows them to be read concurrently.
* **Data Collector Task:** A central "collector" task listens for new data from all individual sensor tasks using a FreeRTOS QueueSet.
* **Processing Task:** The collector task periodically sends a complete "snapshot" of all the latest sensor data to a processing task. This task is responsible for calculating weighted averages and other derived metrics (like dew point).
* **Display Task:** The processing task sends the final, clean data to the display task, which is responsible for rendering all information on the screen. This pipeline ensures the display is always updated with a consistent and complete set of data.

## Hardware Used

### Core
* Raspberry Pi Pico (RP2040/RP2350)

### Communication Buses
* I2C (x2)
* UART (for PMS5003)
* OneWire (for DS18B20)

### Key Sensors
* **Particulate Matter (AQI):** PMS5003
* **Gases (AQI/IAQ):** SEN0321, ENS160, SGP30, SGP40
* **True CO₂ (IAQ):** SEN0574
* **Formaldehyde (IAQ):** SEN0441
* **Temperature / Humidity / Pressure:** BME280 (x2), BMP180, DHT11, DHT20 (x2), DHT22
* **Dedicated Temperature:** DS18B20