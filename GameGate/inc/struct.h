/*
 * struct.h
 *
 *  Created on: Sep 20, 2018
 *      Author: maxwell
 */

#ifndef STRUCT_H_
#define STRUCT_H_

#include <app.h>
#include "sensors_part.h"


struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *box;
	Evas_Object *l_conn;
	Evas_Object *l_hrm;
	void *hrm_sh;
	void *acc_sh;
	void *conn_ih;
};
typedef struct appdata *appdata_h;

#endif /* STRUCT_H_ */
