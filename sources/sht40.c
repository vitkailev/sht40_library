#include "sht40.h"

static const uint8_t crc8_table[] = {
        0x00, 0x31, 0x62, 0x53, 0xC4, 0xF5, 0xA6, 0x97, 0xB9, 0x88, 0xDB, 0xEA, 0x7D, 0x4C, 0x1F, 0x2E,
        0x43, 0x72, 0x21, 0x10, 0x87, 0xB6, 0xE5, 0xD4, 0xFA, 0xCB, 0x98, 0xA9, 0x3E, 0x0F, 0x5C, 0x6D,
        0x86, 0xB7, 0xE4, 0xD5, 0x42, 0x73, 0x20, 0x11, 0x3F, 0x0E, 0x5D, 0x6C, 0xFB, 0xCA, 0x99, 0xA8,
        0xC5, 0xF4, 0xA7, 0x96, 0x01, 0x30, 0x63, 0x52, 0x7C, 0x4D, 0x1E, 0x2F, 0xB8, 0x89, 0xDA, 0xEB,
        0x3D, 0x0C, 0x5F, 0x6E, 0xF9, 0xC8, 0x9B, 0xAA, 0x84, 0xB5, 0xE6, 0xD7, 0x40, 0x71, 0x22, 0x13,
        0x7E, 0x4F, 0x1C, 0x2D, 0xBA, 0x8B, 0xD8, 0xE9, 0xC7, 0xF6, 0xA5, 0x94, 0x03, 0x32, 0x61, 0x50,
        0xBB, 0x8A, 0xD9, 0xE8, 0x7F, 0x4E, 0x1D, 0x2C, 0x02, 0x33, 0x60, 0x51, 0xC6, 0xF7, 0xA4, 0x95,
        0xF8, 0xC9, 0x9A, 0xAB, 0x3C, 0x0D, 0x5E, 0x6F, 0x41, 0x70, 0x23, 0x12, 0x85, 0xB4, 0xE7, 0xD6,
        0x7A, 0x4B, 0x18, 0x29, 0xBE, 0x8F, 0xDC, 0xED, 0xC3, 0xF2, 0xA1, 0x90, 0x07, 0x36, 0x65, 0x54,
        0x39, 0x08, 0x5B, 0x6A, 0xFD, 0xCC, 0x9F, 0xAE, 0x80, 0xB1, 0xE2, 0xD3, 0x44, 0x75, 0x26, 0x17,
        0xFC, 0xCD, 0x9E, 0xAF, 0x38, 0x09, 0x5A, 0x6B, 0x45, 0x74, 0x27, 0x16, 0x81, 0xB0, 0xE3, 0xD2,
        0xBF, 0x8E, 0xDD, 0xEC, 0x7B, 0x4A, 0x19, 0x28, 0x06, 0x37, 0x64, 0x55, 0xC2, 0xF3, 0xA0, 0x91,
        0x47, 0x76, 0x25, 0x14, 0x83, 0xB2, 0xE1, 0xD0, 0xFE, 0xCF, 0x9C, 0xAD, 0x3A, 0x0B, 0x58, 0x69,
        0x04, 0x35, 0x66, 0x57, 0xC0, 0xF1, 0xA2, 0x93, 0xBD, 0x8C, 0xDF, 0xEE, 0x79, 0x48, 0x1B, 0x2A,
        0xC1, 0xF0, 0xA3, 0x92, 0x05, 0x34, 0x67, 0x56, 0x78, 0x49, 0x1A, 0x2B, 0xBC, 0x8D, 0xDE, 0xEF,
        0x82, 0xB3, 0xE0, 0xD1, 0x46, 0x77, 0x24, 0x15, 0x3B, 0x0A, 0x59, 0x68, 0xFF, 0xCE, 0x9D, 0xAC
};

enum SHT40_Commands {
    SERIAL_NUMBER = 0x89,
    SOFT_RESET = 0x94
};

/**
 * @brief Check, that the humidity sensor is initialized
 * @param sht is the SHT40 data structure
 * @return True - sensor has been initialized, otherwise - False
 */
static bool isInit(const SHT40_Def *sht) {
    return sht->isInit;
}

/**
 * @brief Check, that the humidity sensor is reading register values
 * @param sht is the SHT40 data structure
 * @return True - is reading, otherwise - False
 */
static bool isReading(const SHT40_Def *sht) {
    return sht->isReading;
}

/**
 * @brief Send the command value to the sensor
 * @param sht is the SHT40 data structure
 * @param command is the control command value
 * @param isReset is a flag (True - just send the command, False - read a result)
 * @return SHT40_Errors values
 */
static int sendCommand(SHT40_Def *sht, uint8_t command, bool isReset) {
    if (!isInit(sht))
        return SHT40_NOT_INIT;
    if (isReading(sht))
        return SHT40_BUSY;

    sht->command = command;
    int result = I2C_writeData(sht->i2c, sht->devAddr, &sht->command, sizeof(uint8_t), true);
    if (!isReset && result == I2C_SUCCESS) {
        sht->isReading = true;
        sht->delay = 0;
    }

    return result;
}

/**
 * @brief Calculate CRC
 * @param data is the target data
 * @param size is the data size
 * @return CRC-8 value
 */
static uint8_t calculateCRC8(const uint8_t *data, uint16_t size) {
    // www.crccalc.com, CRC-8/NRSC-5

    uint8_t crc = 0xFF;
    for (uint16_t i = 0; i < size; ++i)
        crc = crc8_table[crc ^ data[i]];
    return crc;
}

/**
 * @brief Check, that the received data has valid CRC values
 * @param sht is the SHT40 data structure
 * @param data is the received 6 bytes
 * @return True - both CRC values are valid, otherwise - False
 */
static bool isResponseValid(SHT40_Def *sht, const uint8_t *data) {
    if (calculateCRC8(data, sizeof(uint16_t)) != data[2]) {

    } else if (calculateCRC8(data + 3, sizeof(uint16_t)) != data[5]) {

    } else {
        return true;
    }

    sht->errors++;
    return false;
}

/**
 * @brief The humidity sensor initialization
 * @param sht is the SHT40 data structure
 * @param i2c is the base I2C interface data structure
 * @param addr is the device address (on I2C bus)
 * @return SHT40_Errors values
 */
int SHT40_init(SHT40_Def *sht, void *i2c, uint8_t addr) {
    if (sht == NULL || i2c == NULL || addr == 0)
        return SHT40_WRONG_DATA;

    sht->i2c = i2c;
    sht->devAddr = addr;
    sht->humidity = -1.0f;
    sht->temp = -273.15f;
    sht->isInit = true;
    return SHT40_SUCCESS;
}

/**
 * @brief Read the unique serial number
 * @param sht is the SHT40 data structure
 * @return SHT40_Errors values
 */
int SHT40_readSN(SHT40_Def *sht) {
    return sendCommand(sht, SERIAL_NUMBER, false);
}

/**
 * @brier Perform soft reset of the humidity sensor
 * @param sht is the SHT40 data structure
 * @return SHT40_Errors values
 */
int SHT40_reset(SHT40_Def *sht) {
    return sendCommand(sht, SOFT_RESET, true);
}

/**
 * @brief Read the temperature and humidity registers values
 * @param sht is the SHT40 data structure
 * @param mode is the reading mode (select the accuracy level and turning on the heater)
 * @return SHT40_Errors values
 */
int SHT40_measure(SHT40_Def *sht, uint8_t mode) {
    if (mode == SERIAL_NUMBER || mode == SOFT_RESET)
        return SHT40_WRONG_DATA;

    return sendCommand(sht, mode, false);
}

/**
 * @brief Get the last measured humidity value
 * @param sht is the SHT40 data structure
 * @return humidity value (%)
 */
float SHT40_getHumidity(const SHT40_Def *sht) {
    return sht->humidity;
}

/**
 * @brief Get the last measured temperature value (C)
 * @param sht is the SHT40 data structure
 * @return temperature value (degrees Celsius)
 */
float SHT40_getTemp_C(const SHT40_Def *sht) {
    return sht->temp;
}

/**
 * @brief Get the last measured temperature value (F)
 * @param sht is the SHT40 data structure
 * @return temperature value (degrees Fahrenheit)
 */
float SHT40_getTemp_F(const SHT40_Def *sht) {
    float value = sht->temp;
    value = 32.0f + value * 9.0f / 5.0f;
    return value;
}

/**
 * @brief Get the unique serial number
 * @param sht is the SHT40 data structure
 * @return serial number
 */
uint32_t SHT40_getSerialNumber(const SHT40_Def *sht) {
    return sht->serialNumber;
}

/**
 * @brief Convert sensor register values to relative humidity
 * @param value is the register value
 * @return relative humidity (%)
 */
static float calculateRH(uint16_t value) {
    // Datasheet, Version 6.6, Apr 2024, page 12, equation 1

    float rh = value;
    rh = rh * 125.0f / (((uint32_t) 1 << 16) - 1);
    rh -= 6.0f;
    return rh;
}

/**
 * @brief Convert sensor register values to degrees
 * @param value is the register value
 * @return temperature value (degrees Celsius)
 */
static float calculateTemp(uint16_t value) {
    // Datasheet, Version 6.6, Apr 2024, page 12, equation 2

    float temp = value;
    temp = temp * 175.0f / (((uint32_t) 1 << 16) - 1);
    temp -= 45.0f;
    return temp;
}

/**
 * @brief Update current state of the SHT40 (each 1 msec.)
 * @param sht is the SHT40 data structure
 */
void SHT40_update(SHT40_Def *sht) {
    if (!isInit(sht))
        return;
    if (!isReading(sht))
        return;

    if (I2C_isReading(sht->i2c) || I2C_isWriting(sht->i2c))
        return;

    if (sht->commSent) {
        sht->commSent = false;
        sht->isReading = false;

        if (!I2C_isFailed(sht->i2c)) {
            const uint8_t *data = (const uint8_t *) I2C_getReceivedData(sht->i2c);
            if (isResponseValid(sht, data)) {
                if (sht->command == SERIAL_NUMBER) {
                    sht->serialNumber = ((uint32_t) data[0] << 24 | (uint32_t) data[1] << 16 |
                                         (uint32_t) data[3] << 8 | data[4]);
                } else {
                    sht->temp = calculateTemp(((uint16_t) data[0] << 8) | data[1]);
                    sht->humidity = calculateRH(((uint16_t) data[3] << 8) | data[4]);
                }
            }
        }
    } else {
        uint16_t duration = 0; // msec.
        switch (sht->command) {
            case SHT40_HIGH_PRECISION:
                duration = 8;
                break;
            case SHT40_MED_PRECISION:
                duration = 4;
                break;
            case SHT40_LOW_PRECISION:
                duration = 2;
                break;
            case SHT40_H200_1_HP:
            case SHT40_H110_1_HP:
            case SHT40_H20_1_HP:
                duration = 1100;
                break;
            case SHT40_H200_01_HP:
            case SHT40_H110_01_HP:
            case SHT40_H20_01_HP:
                duration = 110;
                break;
            default:
                duration = 1;
                break;
        }

        if (sht->delay++ == duration) {
            int result = I2C_readData(sht->i2c, sht->devAddr, 6);
            if (result == I2C_SUCCESS)
                sht->commSent = true;
            else
                sht->isReading = false;
        }
    }
}
