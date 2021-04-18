#ifndef SCHEDULE_LIST_THREAD_H
#define SCHEDULE_LIST_THREAD_H

#include "ch.h"

#define  SCHEDULE_LIST_SIZE           10

#define SCHEDULE_STATUS_LIST "UNKNOWN", "RUN", "STOPPED"


typedef struct {
	uint32_t    id;
	thread_t    *tp;
	const char  *name;
	uint16_t    *interval;
	void (*setInterval)(uint16_t interval);
} schedule_t;


extern mailbox_t registerScheduleMail;

void ScheduleList__thread_init(void);
void ScheduleList__thread_start(void);
schedule_t** ScheduleList__getSchedule(void);

#endif