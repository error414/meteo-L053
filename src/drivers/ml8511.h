#ifndef ML8511_H
#define ML8511_H

#define ML8511_REFERENCE_VOLTAGE 0.95f

typedef struct {
	ADCDriver *adcDriver;
	const ADCConversionGroup *adcGroup;
	uint32_t enableLine;
	bool isEnabled;
	adcsample_t adcSample[1];
} ml8511_cfg_t;


void ML8511_init(ml8511_cfg_t *_cfg);
float ML8511_getUV(void);
#endif