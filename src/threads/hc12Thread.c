#include <string.h>
#include "ch.h"
#include "main.h"
#include "hwListThread.h"
#include "hc12Thread.h"
#include "chprintf.h"

static hc12State_t hc12State;
static hw_t hc12HW;

void HC12__thread_checkStatus(void);
bool HC12__thread_goToSleep();
bool HC12__thread_wakeUp();

void HC12__thread_progMode();
void HC12__thread_normalMode();

static THD_WORKING_AREA(hc12StreamVA, 128);
static THD_FUNCTION(hc12StreamThread, arg) {
	(void) arg;
	chRegSetThreadName("hc12Stream");

	///////////////////////////////////////////////////////////////
	// REGISTER HW
	///////////////////////////////////////////////////////////////
	hc12HW.id = hc12State.threadCfg->hwId;
	hc12HW.type = VALUE_TYPE_TRANSMITTER;
	hc12HW.name = "HC12";
	hc12HW.status = HW_STATUS_UNKNOWN;

	(void) chMBPostTimeout(&registerHwMail, (msg_t) &hc12HW, TIME_IMMEDIATE);
	///////////////////////////////////////////////////////////////

	HC12__thread_normalMode();

	while(true){
		HC12__thread_checkStatus();
		if(hc12State.status == HC12_STATUS_OK){
			HC12__thread_goToSleep();
			hc12HW.status = HW_STATUS_OK;
			break;
		}else{
			hc12HW.status = HW_STATUS_ERROR;
		}
		chThdSleepMilliseconds(1000);
	}

	poolStreamObject_t *pbuf;
	msg_t msg;

	while (true) {
		msg = chMBFetchTimeout(&streamMail, (msg_t *) &pbuf, OSAL_MS2I(300));
		if (msg == MSG_OK) {
			HC12__thread_wakeUp();
			chBSemWait(&hc12State.uart_bsem);
			chnWriteTimeout(hc12State.threadCfg->sc_channel, (uint8_t *)&pbuf->message, pbuf->size, OSAL_MS2I(5000));
			pbuf->message[0] = 0x0a; //end of line
			pbuf->message[1] = 0x0d; //end of line
			chnWriteTimeout(hc12State.threadCfg->sc_channel, (uint8_t *)&pbuf->message, 2, OSAL_MS2I(5000));
			chThdSleepMilliseconds(50);
			chPoolFree(&streamMemPool, pbuf);
			chBSemSignal(&hc12State.uart_bsem);
		}else{
			HC12__thread_goToSleep();
		}
	}
}

/**
 *
 */
void HC12__thread_init(const hc12ThreadCfg_t *_threadCfg, hc12cfg_t *_hc12Cfg) {
	hc12State.status = HC12_STATUS_NOK;

	palSetLineMode(_threadCfg->lineSet, PAL_STM32_MODE_OUTPUT);
	palSetLineMode(LINE_GPIOC_10, PAL_STM32_MODE_OUTPUT);
	hc12State.threadCfg = _threadCfg;
	hc12State.hc12Cfg   = _hc12Cfg;

	chBSemObjectInit(&hc12State.uart_bsem, false);
}

/**
 *
 */
void HC12__thread_start(void) {
	chThdCreateStatic(hc12StreamVA, sizeof(hc12StreamVA), THREAD_PRIORITY_HC12, hc12StreamThread, NULL);
}

/**
 *
 */
void HC12__thread_checkStatus(void) {
	hc12State.status = HC12_STATUS_NOK;

	msg_t msg = chBSemWaitTimeout(&hc12State.uart_bsem, 500);
	if(msg != MSG_OK){
		return;
	}

	HC12__thread_progMode();
	chprintf((BaseSequentialStream*)hc12State.threadCfg->sc_channel, HC12_AT_PING);

	chnReadTimeout(hc12State.threadCfg->sc_channel, (uint8_t *)&hc12State.rxBuff, 8, OSAL_MS2I(HC12_WAIT_TO_ANSWER_TIMEOUT));
	if((hc12State.rxBuff[0] == 0x4F || hc12State.rxBuff[1] == 0x4F) && (hc12State.rxBuff[1] == 0x4b || hc12State.rxBuff[2] == 0x4b)){
		chSysLock();
		hc12State.status = HC12_STATUS_OK;
		chSysUnlock() ;
	}

	HC12__thread_normalMode();
	chBSemSignal(&hc12State.uart_bsem);
}

/**
 *
 * @param outStream
 * @return
 */
bool HC12__reconfigureDefault(BaseSequentialStream *outStream){
	return HC12__reconfigure(hc12State.hc12Cfg, outStream);
}

/**
 *
 */
bool HC12__reconfigure(hc12cfg_t *cfg, BaseSequentialStream *outStream) {
	if(hc12State.status == HC12_STATUS_SLEEP){
		HC12__thread_wakeUp();
	}

	if(hc12State.status != HC12_STATUS_OK){
		return false;
	}

	if(chBSemWaitTimeout(&hc12State.uart_bsem, 1000) != MSG_OK){
		return false;
	}

	HC12__thread_progMode();
	while(chnReadTimeout(hc12State.threadCfg->sc_channel, (uint8_t *)&hc12State.rxBuff, 1, TIME_IMMEDIATE)){};
	systime_t timeout;
	uint8_t index;


	chprintf((BaseSequentialStream*)hc12State.threadCfg->sc_channel, HC12_AT_CHANNEL, cfg->channel);
	chprintf(outStream, HC12_AT_CHANNEL, cfg->channel);
	chprintf(outStream, NEWLINE_STR);

	timeout = chVTGetSystemTime();
	index = 0;
	while(true){
		if(chVTGetSystemTime() - timeout > 1000){
			HC12__thread_normalMode();
			chBSemSignal(&hc12State.uart_bsem);
			return false;
		}

		if(chnReadTimeout(hc12State.threadCfg->sc_channel, (uint8_t *)&hc12State.rxBuff[index], 1, TIME_IMMEDIATE) != 0){
			chprintf(outStream, "%c", hc12State.rxBuff[index], 1);
			if(index > 1 && hc12State.rxBuff[index - 1] == 0x0d && hc12State.rxBuff[index] == 0x0a){
				break;
			}
			index++;
		}
	}

	chprintf(outStream, NEWLINE_STR);
	chThdSleepMilliseconds(10);

	///////////////////////////////////////////////////////////////////////////////////////////////////
	chprintf((BaseSequentialStream*)hc12State.threadCfg->sc_channel, HC12_AT_BAUD, cfg->baud);
	chprintf(outStream, HC12_AT_BAUD, cfg->baud);
	chprintf(outStream, NEWLINE_STR);


	timeout = chVTGetSystemTime();
	index = 0;
	while(true){
		if(chVTGetSystemTime() - timeout > 1000){
			HC12__thread_normalMode();
			chBSemSignal(&hc12State.uart_bsem);
			return false;
		}

		if(chnReadTimeout(hc12State.threadCfg->sc_channel, (uint8_t *)&hc12State.rxBuff[index], 1, TIME_IMMEDIATE) != 0){
			chprintf(outStream, "%c", hc12State.rxBuff[index], 1);
			if(index > 1 && hc12State.rxBuff[index - 1] == 0x0d && hc12State.rxBuff[index] == 0x0a){
				break;
			}
			index++;
		}
	}

	chprintf(outStream, NEWLINE_STR);
	chThdSleepMilliseconds(10);

	///////////////////////////////////////////////////////////////////////////////////////////////////
	chprintf((BaseSequentialStream*)hc12State.threadCfg->sc_channel, HC12_AT_MODE_FU, cfg->modeFU);
	chprintf(outStream, HC12_AT_MODE_FU, cfg->modeFU);
	chprintf(outStream, NEWLINE_STR);


	timeout = chVTGetSystemTime();
	index = 0;
	while(true){
		if(chVTGetSystemTime() - timeout > 1000){
			HC12__thread_normalMode();
			chBSemSignal(&hc12State.uart_bsem);
			return false;
		}

		if(chnReadTimeout(hc12State.threadCfg->sc_channel, (uint8_t *)&hc12State.rxBuff[index], 1, TIME_IMMEDIATE) != 0){
			chprintf(outStream, "%c", hc12State.rxBuff[index], 1);
			if(index > 1 && hc12State.rxBuff[index - 1] == 0x0d && hc12State.rxBuff[index] == 0x0a){
				break;
			}
			index++;
		}
	}

	chprintf(outStream, NEWLINE_STR);
	chThdSleepMilliseconds(10);


	///////////////////////////////////////////////////////////////////////////////////////////////////
	chprintf((BaseSequentialStream*)hc12State.threadCfg->sc_channel, HC12_AT_POWER, cfg->power);
	chprintf(outStream, HC12_AT_POWER, cfg->power);
	chprintf(outStream, NEWLINE_STR);


	timeout = chVTGetSystemTime();
	index = 0;
	while(true){
		if(chVTGetSystemTime() - timeout > 1000){
			HC12__thread_normalMode();
			chBSemSignal(&hc12State.uart_bsem);
			return false;
		}

		if(chnReadTimeout(hc12State.threadCfg->sc_channel, (uint8_t *)&hc12State.rxBuff[index], 1, TIME_IMMEDIATE) != 0){
			chprintf(outStream, "%c", hc12State.rxBuff[index], 1);
			if(index > 1 && hc12State.rxBuff[index - 1] == 0x0d && hc12State.rxBuff[index] == 0x0a){
				break;
			}
			index++;
		}
	}

	chprintf(outStream, NEWLINE_STR);
	chThdSleepMilliseconds(10);

	hc12State.hc12Cfg = cfg;
	HC12__thread_normalMode();
	chBSemSignal(&hc12State.uart_bsem);

	return true;
}

/**
 *
 * @return
 */
bool HC12__thread_goToSleep(void ){
	if(hc12State.status != HC12_STATUS_OK){
		return false;
	}

	palClearLine(hc12State.threadCfg->enableLine);
	chThdSleepMilliseconds(10);
	hc12State.status = HC12_STATUS_SLEEP;
	return true;
}

/**
 *
 *
 * @return
 */
bool HC12__thread_wakeUp(){
	if(hc12State.status != HC12_STATUS_SLEEP){
		return false;
	}

	palSetLine(hc12State.threadCfg->enableLine);
	chThdSleepMilliseconds(HC12_WAIT_AFTER_SLEEP_TIMEOUT);
	hc12State.status = HC12_STATUS_OK;
	return true;
}

/**
 *
 */
void HC12__thread_progMode(){
	palClearLine(hc12State.threadCfg->lineSet);
	chThdSleepMilliseconds(HC12_WAIT_ENTER_TO_PROG_MODE_TIMEOUT);
}

/**
 *
 */
void HC12__thread_normalMode(){
	palSetLine(hc12State.threadCfg->lineSet);
	chThdSleepMilliseconds(HC12_WAIT_EXIT_FROM_PROG_MODE_TIMEOUT);
}

/**
 *
 */
bool HC12__tunrOffRadio(void){
	if(chBSemWaitTimeout(&hc12State.uart_bsem, OSAL_MS2I(1000)) != MSG_OK){
		return false;
	}

	HC12__thread_goToSleep();
	return true;
}

/**
 *
 */
void HC12__tunrOnRadio(void){
	HC12__thread_wakeUp();
	chBSemSignal(&hc12State.uart_bsem);
}
