#ifndef HC12_THREAD_H
#define HC12_THREAD_H

#include "hal.h"
#include "pools.h"

#define HC12_AT_PING "AT"

#define HC12_AT_CHANNEL "AT+C00%i"
#define HC12_AT_CHANNEL1 1
#define HC12_AT_CHANNEL2 2
#define HC12_AT_CHANNEL3 3

#define HC12_AT_BAUD "AT+B%i"
#define HC12_AT_BAUD1200 1200
#define HC12_AT_BAUD2400 2400
#define HC12_AT_BAUD9600 9600

#define HC12_AT_MODE_FU "AT+FU%i"
#define HC12_AT_MODE_FU1 1
#define HC12_AT_MODE_FU2 2
#define HC12_AT_MODE_FU3 3

#define HC12_AT_POWER "AT+P%i"
#define HC12_AT_POWER_0_8mw 1
#define HC12_AT_POWER_1_6mw 2
#define HC12_AT_POWER_3_6mw 3
#define HC12_AT_POWER_6_3mw 4
#define HC12_AT_POWER_12_6mw 5

#define HC12_AT_SLEEP "AT+SLEEP"

#define HC12_WAIT_ENTER_TO_PROG_MODE_TIMEOUT 50
#define HC12_WAIT_EXIT_FROM_PROG_MODE_TIMEOUT 50
#define HC12_WAIT_TO_ANSWER_TIMEOUT 200
#define HC12_WAIT_AFTER_SLEEP_TIMEOUT 500

typedef enum {
	HC12_STATUS_NOK     = 0,
	HC12_STATUS_SLEEP   = 1,
	HC12_STATUS_OK      = 2,
} hc12StatusEnum;

typedef struct {
	uint8_t     channel;
	uint16_t    baud;
	uint8_t     modeFU;
	uint8_t     power;
} hc12cfg_t;

typedef struct {
	uint32_t hwId;
	uint32_t lineSet; // pin for go to set mode
	BaseChannel *sc_channel;
} hc12ThreadCfg_t;

typedef struct {
	uint8_t rxBuff[STREAM_BUFFER_SIZE];
	uint8_t txBuff[STREAM_BUFFER_SIZE];
	hc12StatusEnum status;
	hc12cfg_t *hc12Cfg;
	const hc12ThreadCfg_t *threadCfg;
	binary_semaphore_t uart_bsem;
} hc12State_t;

void HC12__thread_init(const hc12ThreadCfg_t *_threadCfg, hc12cfg_t *_hc12Cfg);
void HC12__thread_start(void);
bool HC12__thread_reconfigure(hc12cfg_t *cfg, BaseSequentialStream *outStream);
bool HC12__thread_reconfigureDefault(BaseSequentialStream *outStream);

#endif