#ifndef SHT40_H
#define SHT40_H

#ifdef __cplusplus
extern "C" {
#endif

enum SHT40_Errors {
    SHT40_SUCCESS = 0,

    SHT40_NOT_INIT = -I2C_NUMBER_ERRORS - 1,
    SHT40_WRONG_DATA = -I2C_NUMBER_ERRORS - 2,
    SHT40_BUSY = -I2C_NUMBER_ERRORS - 3,
};

enum SHT40_Modes {
    // each mode allows reading T and RH

    SHT40_HIGH_PRECISION = 0xFD, // high precision (high repeatability)
    SHT40_MED_PRECISION = 0xF6, // medium precision (medium repeatability)
    SHT40_LOW_PRECISION = 0xE0, // lowest precision (low repeatability)

    // activate heater, including a high precision measurement just before deactivation
    SHT40_H200_1_HP = 0x39, // 200mW for 1sec.
    SHT40_H200_01_HP = 0x32, // 200mW for 0.1sec.
    SHT40_H110_1_HP = 0x2F, // 110mW for 1sec.
    SHT40_H110_01_HP = 0x24, // 110mW for 0.1sec.
    SHT40_H20_1_HP = 0x1E, // 20mW for 1sec.
    SHT40_H20_01_HP = 0x15 // 20mW for 0.1sec.
};

typedef struct {
    bool isInit;

    bool isReading;
    bool commSent;
    uint8_t command;
    uint32_t errors;
    uint32_t delay; // msec

    uint32_t serialNumber;
    float humidity; // %
    float temp; // C

    uint8_t devAddr;
    void *i2c;
} SHT40_Def;

int SHT40_init(SHT40_Def *sht, void *i2c, uint8_t addr);

int SHT40_readSN(SHT40_Def *sht);

int SHT40_reset(SHT40_Def *sht);

int SHT40_measure(SHT40_Def *sht, uint8_t mode);

float SHT40_getHumidity(const SHT40_Def *sht);

float SHT40_getTemp_C(const SHT40_Def *sht);

float SHT40_getTemp_F(const SHT40_Def *sht);

uint32_t SHT40_getSerialNumber(const SHT40_Def *sht);

void SHT40_update(SHT40_Def *sht);

#ifdef __cplusplus
}
#endif

#endif // SHT40_H
