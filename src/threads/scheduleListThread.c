#include "ch.h"
#include "main.h"
#include "scheduleListThread.h"
#include <string.h>

static schedule_t *scheduleList[SCHEDULE_LIST_SIZE];

////////////////////////////////////////////////////////////////////////////
static msg_t registerScheduleDataLetter[SCHEDULE_LIST_SIZE];
MAILBOX_DECL(registerScheduleMail, &registerScheduleDataLetter, SCHEDULE_LIST_SIZE);
/////////////////////////////////////////////////////////////////////////

static THD_WORKING_AREA(registerScheduleVA, 40);
static THD_FUNCTION(registerScheduleThread, arg) {
	(void) arg;
	chRegSetThreadName("registerSCH");

	msg_t msg;
	schedule_t *newSchedule;

	while (true) {
		msg = chMBFetchTimeout(&registerScheduleMail, (msg_t *) &newSchedule, TIME_INFINITE);
		if (msg == MSG_OK) {
			scheduleList[newSchedule->id] = newSchedule;
		}
	}
}

/**
 *
 */
void ScheduleList__thread_init(void) {
}

/**
 *
 */
void ScheduleList__thread_start(void) {
	chThdCreateStatic(registerScheduleVA, sizeof(registerScheduleVA), THREAD_PRIORITY_REGISTER_SCH, registerScheduleThread, NULL);
}

/**
 *
 * @return
 */
schedule_t** ScheduleList__getSchedule(void) {
	return &scheduleList[0];
}