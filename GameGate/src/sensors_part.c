/*
 * sensors_part.c
 *
 *  Created on: Sep 20, 2018
 *      Author: maxwell
 */
#include "sensors_part.h"
#include "net.h"

/* ============================================================================
 * ============================== HRM =========================================
 * ============================================================================
 */
int hrm_init(hrm_sensor_h hrm_sh, appdata_h ad)
{
	sensor_type_e type = SENSOR_HRM;
	bool supported;

	int error = sensor_is_supported(type, &supported);
	if (error != SENSOR_ERROR_NONE) {
	    dlog_print(DLOG_ERROR, LOG_TAG, "sensor_is_supported error: %d", error);
	    return 0;
	}

	error = sensor_get_default_sensor(type, &(hrm_sh->sensor));
	if (error != SENSOR_ERROR_NONE) {
	    dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_default_sensor error: %d", error);
	    return 0;
	}

	hrm_sh->ad = ad;
	ad->hrm_sh = (void *)hrm_sh;
	return supported;
}

void hrm_create_listener(hrm_sensor_h hrm_sh)
{
	// creating an event listener
	int error = sensor_create_listener(hrm_sh->sensor, &(hrm_sh->listener));
	if (error != SENSOR_ERROR_NONE) {
	    dlog_print(DLOG_ERROR, LOG_TAG, "sensor_create_listener error: %d",error);
	    return;
	}

	error = sensor_listener_set_event_cb(hrm_sh->listener, HRM_MS, on_sensor_event, (void *)hrm_sh);
	if (error != SENSOR_ERROR_NONE) {
	    dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_event_cb error: %d", error);
	    return;
	}

	error = sensor_listener_set_option(hrm_sh->listener, SENSOR_OPTION_ON_IN_SCREEN_OFF);
	if (error != SENSOR_ERROR_NONE) {
	    dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_option error: %d", error);
	    return;
	}

	sensor_listener_start(hrm_sh->listener);
	if (error != SENSOR_ERROR_NONE) {
	    dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_start error: %d", error);
	    return;
	}
}

void on_sensor_event(sensor_h sensor, sensor_event_s *event, void *user_data)
{
    // Select a specific sensor with a sensor handle
    sensor_type_e type;
    sensor_get_type(sensor, &type);
    char a[100] = "";
    static char msg[MSG_LEN] = "";
    memset(&msg, 0x00, MSG_LEN);
    struct timeval ct;

    switch (type) {
    case SENSOR_HRM: {
        hrm_sensor_h hrm_sh = (hrm_sensor_h)user_data;
        appdata_h ad = hrm_sh->ad;

    	dlog_print(DLOG_INFO, LOG_TAG, "%d" , event->values[0]);
    	hrm_sh->hrm = event->values[0];
    	sprintf(a, "<align=center>HRM: %.0f<\align>", hrm_sh->hrm);
    	elm_object_text_set(ad->l_hrm, a);
        check_connection(HOST_ADDR, ad);
        gettimeofday(&ct, NULL);
        sprintf(msg, FRMT_HRM, ct.tv_sec * 1000 + ct.tv_usec / 1000, hrm_sh->hrm);
        conn_sent(ad, msg);
    	break;
    }
    case SENSOR_LINEAR_ACCELERATION: {
        acc_sensor_h acc_sh = (acc_sensor_h)user_data;
        appdata_h ad = acc_sh->ad;

    	acc_sh->acc_x = event->values[0];
    	acc_sh->acc_y = event->values[1];
    	acc_sh->acc_z = event->values[2];
        gettimeofday(&ct, NULL);
    	sprintf(msg, FRMT_ACC, ct.tv_sec * 1000 + ct.tv_usec / 1000, acc_sh->acc_x,
    	             acc_sh->acc_y, acc_sh->acc_z);
    	conn_sent(ad, msg);
    	break;
    }
    default:
        dlog_print(DLOG_ERROR, LOG_TAG, "Not an HRM event");
    }
}

/* ============================================================================
 * =========================== Acceleration ===================================
 * ============================================================================
 */

int acc_init(acc_sensor_h acc_sh, appdata_h ad)
{
	sensor_type_e type = SENSOR_LINEAR_ACCELERATION;
	bool supported;

	int error = sensor_is_supported(type, &supported);
	if (error != SENSOR_ERROR_NONE) {
	    dlog_print(DLOG_ERROR, LOG_TAG, "sensor_is_supported error: %d", error);
	    return 0;
	}

	error = sensor_get_default_sensor(type, &(acc_sh->sensor));
	if (error != SENSOR_ERROR_NONE) {
	    dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_default_sensor error: %d", error);
	    return 0;
	}

	acc_sh->ad = ad;
	ad->acc_sh = (void *)acc_sh;
	return supported;
}

void acc_create_listener(acc_sensor_h acc_sh)
{
	// creating an event listener
	int error = sensor_create_listener(acc_sh->sensor, &(acc_sh->listener));
	if (error != SENSOR_ERROR_NONE) {
	    dlog_print(DLOG_ERROR, LOG_TAG, "sensor_create_listener error: %d",error);
	    return;
	}

	error = sensor_listener_set_event_cb(acc_sh->listener, ACC_MS, on_sensor_event, (void *)acc_sh);
	if (error != SENSOR_ERROR_NONE) {
	    dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_event_cb error: %d", error);
	    return;
	}

	error = sensor_listener_set_option(acc_sh->listener, SENSOR_OPTION_ON_IN_SCREEN_OFF);
	if (error != SENSOR_ERROR_NONE) {
	    dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_option error: %d", error);
	    return;
	}

	sensor_listener_start(acc_sh->listener);
	if (error != SENSOR_ERROR_NONE) {
	    dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_start error: %d", error);
	    return;
	}
}
