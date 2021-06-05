#include <stdlib.h>
#include "hal.h"
#include "veml7700.h"
#include "math.h"

static bool VEML7700_I2C__sing_reg_write(VEML7700_HandleTypedef *dev, uint8_t regAdd, uint32_t regData);
static bool VEML7700_I2C__sing_reg_read(VEML7700_HandleTypedef *dev, uint8_t regAdd, uint32_t *data);

bool VEML7700_getALS(VEML7700_HandleTypedef* dev, uint32_t *als);
bool VEML7700_scaleLux(VEML7700_HandleTypedef* dev, uint32_t raw_counts, float *lux);


uint8_t VEML7700_getGain(VEML7700_HandleTypedef* dev);
uint8_t VEML7700_getIntegrationTime(VEML7700_HandleTypedef* dev);

/**
 *
 * @param regAdd
 * @param dataMask
 * @param regData
 * @return
 */
static bool VEML7700_I2C__sing_reg_write(VEML7700_HandleTypedef *dev, uint8_t regAdd, uint32_t regData){
    i2cAcquireBus(dev->i2c);
    if(!dev->checkI2cFunc(dev->i2c)){
        i2cReleaseBus(dev->i2c);
        return false;
    }

    static uint8_t dataTx[3]; //static for DMA
    static uint8_t dataRx;

    dataTx[0] = regAdd;
    dataTx[1] = regData & 0xFF;
    dataTx[2] = (regData >> 8) & 0xFF;

    // finally, write the data to the register
    if (i2cMasterTransmitTimeout(dev->i2c, dev->addr, (uint8_t*)&dataTx, 3, (uint8_t*)&dataRx, 0, OSAL_MS2I(800)) == MSG_OK) {
        i2cReleaseBus(dev->i2c);
        return true;
    } else{
        i2cReleaseBus(dev->i2c);
        return false;
    }
}

/**
 *
 * @param regAdd
 * @return
 */
static bool VEML7700_I2C__sing_reg_read(VEML7700_HandleTypedef *dev, uint8_t regAdd, uint32_t *data){
    i2cAcquireBus(dev->i2c);

    if(!dev->checkI2cFunc(dev->i2c)){
        i2cReleaseBus(dev->i2c);
        return true;
    }

    static uint8_t dataTx; //static for DMA
    static uint8_t dataRx[2];

    dataTx = regAdd;

    if (i2cMasterTransmitTimeout(dev->i2c, dev->addr, (uint8_t*)&dataTx, 1, (uint8_t*)&dataRx, 2, OSAL_MS2I(800)) == MSG_OK) {
        i2cReleaseBus(dev->i2c);
        *data = (dataRx[1] << 8) | (dataRx[0] & 0xff);
        return true;
    } else{
        i2cReleaseBus(dev->i2c);
        return false;
    }
}

/**
 *
 * @param dev
 * @param i2c_handle
 * @param addr_grounded
 */
void VEML7700_init_default_params(VEML7700_HandleTypedef* dev, I2CDriver* i2c){
    dev->i2c = i2c;
    dev->addr = VEML7700_I2C_ADDRESS;

    dev->registerCache[0] = (
            (((uint32_t)VEML7700_ALS_GAIN_d8) << VEML7700_ALS_SM_SHIFT) |
            (((uint32_t)VEML7700_ALS_INTEGRATION_25ms) << VEML7700_ALS_IT_SHIFT) |
            (((uint32_t)VEML7700_ALS_PERSISTENCE_1) << VEML7700_ALS_PERS_SHIFT) |
            (((uint32_t)0) << VEML7700_ALS_INT_EN_SHIFT) |
            (((uint32_t)0) << VEML7700_ALS_SD_SHIFT)
   );

    dev->registerCache[1] = 0x0000;
    dev->registerCache[2] = 0xffff;
    dev->registerCache[3] = (
            (((uint32_t)VEML7700_ALS_POWER_MODE_3) << VEML7700_PSM_SHIFT) |
            (((uint32_t)0) << VEML7700_PSM_EN_SHIFT)
    );
}

/**
 *
 * @param dev
 * @param als_gain
 * @return
 */
void VEML7700_init_default_params_gain(VEML7700_HandleTypedef* dev, uint8_t als_gain) {
    dev->registerCache[0] = (
            (((uint32_t)VEML7700_ALS_GAIN_d8) << VEML7700_ALS_SM_SHIFT) |
            (((uint32_t)VEML7700_ALS_INTEGRATION_25ms) << VEML7700_ALS_IT_SHIFT) |
            (((uint32_t)VEML7700_ALS_PERSISTENCE_1) << VEML7700_ALS_PERS_SHIFT) |
            (((uint32_t)0) << VEML7700_ALS_INT_EN_SHIFT) |
            (((uint32_t)0) << VEML7700_ALS_SD_SHIFT)
    );

    dev->registerCache[1] = 0x0000;
    dev->registerCache[2] = 0xffff;
    dev->registerCache[3] = (
            (((uint32_t)VEML7700_ALS_POWER_MODE_3) << VEML7700_PSM_SHIFT) |
            (((uint32_t)0) << VEML7700_PSM_EN_SHIFT)
    );
}

/**
 *
 * @param dev
 * @return
 */
bool VEML7700_init(VEML7700_HandleTypedef* dev) {

    for (uint8_t i = 0; i < 4; i++){
        if(!VEML7700_I2C__sing_reg_write(dev, i, dev->registerCache[i])){
            return false;
        }
    }

    chThdSleepMilliseconds(10);
    return true;
}

/**
 *
 * @param dev
 * @param lux
 * @return
 */
bool VEML7700_getALSLux(VEML7700_HandleTypedef* dev){
    static uint32_t raw_counts;

    if(VEML7700_getALS(dev, &raw_counts) && VEML7700_scaleLux(dev, raw_counts, &dev->lux)){
        return true;
    }

    return false;
}

/**
 *
 * @param dev
 * @param als
 * @return
 */
bool VEML7700_getALS(VEML7700_HandleTypedef* dev, uint32_t *als){
    return VEML7700_I2C__sing_reg_read(dev, VEML7700_COMMAND_ALS, als);
}

/**
 *
 * @param dev
 * @param raw_counts
 * @param lux
 */
bool VEML7700_scaleLux(VEML7700_HandleTypedef* dev, uint32_t rawCounts, float *lux) {

    float factor1, factor2, result;
    static uint8_t x1 = 0, x2 = 1, d8 = 0;

    switch (VEML7700_getGain(dev) & 0x3) {
        case VEML7700_ALS_GAIN_x1:
            factor1 = 1.f;
            break;
        case VEML7700_ALS_GAIN_x2:
            factor1 = 0.5f;
            break;
        case VEML7700_ALS_GAIN_d8:
            factor1 = 8.f;
            break;
        case VEML7700_ALS_GAIN_d4:
            factor1 = 4.f;
            break;
        default:
            factor1 = 1.f;
            break;
    }

    switch (VEML7700_getIntegrationTime(dev)) {
        case VEML7700_ALS_INTEGRATION_25ms:
            factor2 = 0.2304f;
            break;
        case VEML7700_ALS_INTEGRATION_50ms:
            factor2 = 0.1152f;
            break;
        case VEML7700_ALS_INTEGRATION_100ms:
            factor2 = 0.0576f;
            break;
        case VEML7700_ALS_INTEGRATION_200ms:
            factor2 = 0.0288f;
            break;
        case VEML7700_ALS_INTEGRATION_400ms:
            factor2 = 0.0144f;
            break;
        case VEML7700_ALS_INTEGRATION_800ms:
            factor2 = 0.0072f;
            break;
        default:
            factor2 = 0.2304f;
            break;
    }

    result = (float)rawCounts * factor1 * factor2;
    if((result > 1880.00f) && (result < 3771.00f)){
        if(x1 == 1){
            VEML7700_init_default_params_gain(dev, VEML7700_ALS_GAIN_x1);
            if(!VEML7700_init(dev)){
                return false;
            }
            x1 = 0; x2 = 1; d8 = 1;
        }
    }else if(result>3770.00f){
        if(d8 == 1){
            VEML7700_init_default_params_gain(dev, VEML7700_ALS_GAIN_d8);
            if(!VEML7700_init(dev)){
                return false;
            }
            x1 = 1; x2 = 1; d8 = 0;
        }
    }else{
        if(x2 == 1){
            VEML7700_init_default_params_gain(dev, VEML7700_ALS_GAIN_x2);
            if(!VEML7700_init(dev)){
                return false;
            }
            x1 = 1; x2 = 0; d8 = 1;
        }
    }

    *lux = result;
    // apply correction from App. Note for all readings
    //   using Horner's method
   *lux = *lux * (1.0023f + *lux * (8.1488e-5f + *lux * (-9.3924e-9f + *lux * 6.0135e-13f)));

   return true;
}

/**
 *
 * @param dev
 * @return
 */
uint8_t VEML7700_getGain(VEML7700_HandleTypedef* dev) {
    return ((dev->registerCache[VEML7700_COMMAND_ALS_SM] & VEML7700_ALS_SM_MASK) >> VEML7700_ALS_SM_SHIFT );
}

/**
 *
 * @param dev
 * @return
 */
uint8_t VEML7700_getIntegrationTime(VEML7700_HandleTypedef* dev) {
    return ((dev->registerCache[VEML7700_COMMAND_ALS_IT] & VEML7700_ALS_IT_MASK) >> VEML7700_ALS_IT_SHIFT  );
}
