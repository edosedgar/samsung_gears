/*
 * sensors_part.h
 *
 *  Created on: Sep 20, 2018
 *      Author: maxwell
 */

#ifndef SENSORS_PART_H_
#define SENSORS_PART_H_

#include <sensor.h>
#include "biomonitorc.h"
#include <struct.h>

#define HRM_MS 2500
#define ACC_MS 50

// The main structure of HRM sensor
struct hrm_sensor {
	sensor_h sensor;
	sensor_listener_h listener;
	appdata_h ad;
	float hrm;
};
typedef struct hrm_sensor *hrm_sensor_h;

// The main structure of ACC sensor
struct acc_sensor {
	sensor_h sensor;
	sensor_listener_h listener;
	appdata_h ad;
	float acc_x;
	float acc_y;
	float acc_z;
};
typedef struct acc_sensor *acc_sensor_h;
/*
 * Prototypes
 */

/* ================== HRM sensor *************************** */
int hrm_init(hrm_sensor_h hrm_sh, appdata_h ad);

void hrm_create_listener(hrm_sensor_h hrm_sh);

void on_sensor_event(sensor_h sensor, sensor_event_s *event, void *user_data);

/* ================= Accelerometer sensor ****************** */

int acc_init(acc_sensor_h acc_sh, appdata_h ad);

void acc_create_listener(acc_sensor_h acc_sh);

#endif /* SENSORS_PART_H_ */
