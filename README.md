# SHT40 I2C library

This is a library that covers all capabilities of the SHT40 relative humidity sensor via the I2C interface  
[Official Sensirion data source](https://sensirion.com/products/catalog/SHT40)

## Sensor description

- Accuracies: RH = &pm;1.0 %RH, T = &pm;0.1&deg;C
- I2C FM+, CRC checksum, multip. I2C addr.
- Supply voltage: 1.08 to 3.6 V
- Avg. current: 0.4uA, idle current: 80nA
- Operating range: 0 ... 100 %RH, -40 ... 125 &deg;C
- Power heater, true NIST-traceability
- JEDEC JESD47 qualification
- Sensor-specific calibration certificate acc. to ISO 17025: 2017, 3-point temp. calibration

## Library features

- Reading the unique serial number;
- Reading the values of temperature and humidity registers;

## Algorithm

1. Initialization: SHT40_init;
2. Read serial number (optional): SHT40_readSN;
3. Read values: SHT40_measure

- you can measure relative humidity in three modes
-
    - high precision
-
    - medium precision
-
    - low precision
- you can turn ON the heater and specify its power (1 sec. or 0.1 sec.)
-
    - 200mW
-
    - 110mw
-
    - 20mW
