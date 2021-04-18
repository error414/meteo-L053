#include "hal.h"
#include "ml8511.h"

static ml8511_cfg_t *cfg;


void ML8511_enable(void);
void ML8511_disable(void);

/**
 *
 * @param _cfg
 */
void ML8511_init(ml8511_cfg_t *_cfg){
	if(_cfg->adcGroup->num_channels != 1){
		chSysHalt(__func__);
	}
	cfg = _cfg;
	cfg->isEnabled = false;
}

/**
 *
 * @return
 */
float ML8511_getUV(){

	ML8511_enable();

	adcAcquireBus(cfg->adcDriver);
	adcStart(cfg->adcDriver, NULL);

	msg_t msg = adcConvert(cfg->adcDriver, cfg->adcGroup, cfg->adcSample, 1);

	adcStop(cfg->adcDriver);
	adcReleaseBus(cfg->adcDriver);

	ML8511_disable();

	if(msg == MSG_OK){
		float voltage = (float)(3.3f / (4096.0f / (double)cfg->adcSample[0]));

		if (voltage <= ML8511_REFERENCE_VOLTAGE || voltage > 2.8f){
			return -1.0f;
		}
		voltage -= ML8511_REFERENCE_VOLTAGE;

		return voltage * (15.0f / 1.8f);


	}else{
		return -1.0f;
	}
}

/**
 *
 */
void ML8511_enable(void){
	if (cfg->enableLine != ML8511_NO_ENABLE_LINE && cfg->isEnabled == false){
		palSetLine(cfg->enableLine);
		cfg->isEnabled = true;
		chThdSleepMilliseconds(1);
	}
}

/**
 *
 */
void ML8511_disable(void){
	if (cfg->enableLine != ML8511_NO_ENABLE_LINE){
		cfg->isEnabled = false;
		palClearLine(cfg->enableLine);
	}
}