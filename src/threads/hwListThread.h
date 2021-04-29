#ifndef HW_LIST_THREAD_H
#define HW_LIST_THREAD_H

#define  HW_LIST_SIZE           10
#define  HW_LIST_VALUES_SIZE    6

#define HW_STATUS_LIST "UNKNOWN", "OK", "ERROR"
#define HW_TYPE_LIST "SENSOR", "TRANSMITTER"

enum hwType_e {
	VALUE_TYPE_SENSOR       = 0,
	VALUE_TYPE_TRANSMITTER  = 1,
};

enum hwStatus_e {
	HW_STATUS_UNKNOWN       = 0,
	HW_STATUS_OK            = 1,
	HW_STATUS_ERROR         = 2,
};

typedef enum {
	VALUE_FORMATTER_NONE    = 0,
	VALUE_FORMATTER_100     = 1,
	VALUE_FORMATTER_BOOL    = 2,
} valueFormatter_e;

typedef struct {
	uint32_t          value;
	valueFormatter_e  formatter;
	const char        *name;
} sensorValue_t;

typedef struct {
	uint32_t          id;
	const char        *name;
	enum hwStatus_e   status;
	enum hwType_e     type;
	sensorValue_t     values[HW_LIST_VALUES_SIZE];
} hw_t;


extern mailbox_t registerHwMail;

void HwList__thread_init(void);
void HwList__thread_start(void);
hw_t** HwList__getHw(void);

#endif