#include "ch.h"
#include "main.h"
#include "hwListThread.h"
#include <string.h>

static hw_t *hwList[HW_LIST_SIZE];

////////////////////////////////////////////////////////////////////////////
msg_t registerHwDataLetter[HW_LIST_SIZE];
MAILBOX_DECL(registerHwMail, &registerHwDataLetter, HW_LIST_SIZE);
/////////////////////////////////////////////////////////////////////////

static THD_WORKING_AREA(registerHWVA, 20);
static THD_FUNCTION(registerHwThread, arg) {
	(void) arg;
	chRegSetThreadName("registerHW");

	msg_t msg;
	hw_t *newHw;

	while (true) {
		msg = chMBFetchTimeout(&registerHwMail, (msg_t *) &newHw, TIME_INFINITE);
		if (msg == MSG_OK) {
			hwList[newHw->id] = newHw;
		}
	}
}

/**
 *
 */
void HwList__thread_init(void) {
}

/**
 *
 */
void HwList__thread_start(void) {
	chThdCreateStatic(registerHWVA, sizeof(registerHWVA), THREAD_PRIORITY_REGISTER_HW, registerHwThread, NULL);
}

/**
 *
 * @return
 */
hw_t** HwList__getHw(void) {
	return &hwList[0];
}