#include <string.h>
#include "main.h"
#include "appCfg.h"
#include "msp.h"


/**
 *
 * @return
 */
void MSP__createMspFrame(poolStreamObject_t *streamObject, uint8_t cmd, uint32_t argc, const uint32_t *argv){
	memcpy(streamObject->message, "$M<", 3);
	uint8_t *p;

	streamObject->size =  6 + (argc * 4);
	cmd = HC12_HW_ID_OFFSET + cmd;
	uint8_t crc = cmd ^ (argc * 4);

	p = (uint8_t*)(streamObject->message);              p += 3;
	*p = (uint8_t)(argc * 4);                           p++;
	*p = (uint8_t)cmd;                                  p++;
	for(uint32_t i = 0; i < argc; i++){
		crc ^= *p = (uint8_t)((*argv) >> 24);           p++;
		crc ^= *p = (uint8_t)((*argv) >> 16);           p++;
		crc ^= *p = (uint8_t)((*argv) >> 8);            p++;
		crc ^= *p = (uint8_t)(*argv);                   p++;
		argv++;
	}

	*p = crc;
}

